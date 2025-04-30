// Filter "Chart/Rendering"

#include "ChartQuadRenderer.hpp"

#include "FretAtlas.hpp"

void ChartQuadRenderer::Flush(std::size_t aGroupID)
{
	ZoneScoped;

	const std::size_t queuedSinceLastFlush = myQuadInstanceData.size() - myLastQuadFlush;
	if (queuedSinceLastFlush == 0)
		return;

	QuadInstanceGroup& instanceGroup = myQuadGroups.emplace_back();
	instanceGroup.Start = myLastQuadFlush;
	instanceGroup.Count = queuedSinceLastFlush;
	instanceGroup.GroupID = aGroupID;

	myLastQuadFlush = myQuadInstanceData.size();
}

void ChartQuadRenderer::Setup(
	Atrium::GraphicsAPI& aGraphicsAPI,
	const std::shared_ptr<Atrium::RootSignature>& aRootSignature,
	Atrium::GraphicsFormat aColorTargetFormat
	)
{
	ZoneScoped;

	myQuadMesh = CreateQuadMesh(aGraphicsAPI);
	myQuadMesh->SetName(L"Quad");

	Atrium::PipelineStateDescription pipelineDescription;
	pipelineDescription.RootSignature = aRootSignature;
	pipelineDescription.InputLayout = ChartQuadVertex::GetInputLayout();
	const std::filesystem::path shaderPath = "ChartQuad.hlsl";
	pipelineDescription.VertexShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Shader::Type::Vertex, "vertexShader");
	pipelineDescription.PixelShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Shader::Type::Pixel, "pixelShader");
	pipelineDescription.OutputFormats = { aColorTargetFormat };
	pipelineDescription.BlendMode.BlendFactors[0].Enabled = true;

	myQuadPipelineState = aGraphicsAPI.GetResourceManager().CreatePipelineState(pipelineDescription);

	struct CameraMatrices
	{
		Atrium::Matrix View;
		Atrium::Matrix Projection;
	} cameraMatrices;
	cameraMatrices.View = FretboardMatrices::CameraViewMatrix;
	cameraMatrices.Projection = FretboardMatrices::CameraProjectionMatrix;

	myCameraMatrices = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::GraphicsBuffer::Target::Constant, 1, sizeof(CameraMatrices));
	myCameraMatrices->SetData(&cameraMatrices, sizeof(CameraMatrices));

	myQuadInstanceBuffer = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::GraphicsBuffer::Target::Vertex, 512, sizeof(ChartQuadInstance));
}

void ChartQuadRenderer::SetTexture(std::shared_ptr<Atrium::Texture> aTexture)
{
	myTexture = aTexture;
}

void ChartQuadRenderer::Render(Atrium::FrameGraphicsContext& aContext, std::function<void(std::size_t)> aGroupPreparation)
{
	ZoneScoped;
	CONTEXT_ZONE(aContext, "Render quads");

	myQuadInstanceBuffer->SetData<ChartQuadInstance>(myQuadInstanceData);

	aContext.SetPipelineState(myQuadPipelineState);
	aContext.SetPipelineResource(Atrium::ResourceUpdateFrequency::PerFrame, 0, myCameraMatrices);
	aContext.SetPipelineResource(Atrium::ResourceUpdateFrequency::PerMaterial, 0, myTexture);
	aContext.SetVertexBuffer(myQuadInstanceBuffer, 1);

	for (const QuadInstanceGroup& group : myQuadGroups)
	{
		aGroupPreparation(group.GroupID);

		myQuadMesh->DrawInstancedToFrame(
			aContext,
			static_cast<unsigned int>(group.Count),
			static_cast<unsigned int>(group.Start)
		);
	}

	myLastQuadFlush = 0;
	myQuadInstanceData.clear();
	myQuadGroups.clear();
}

void ChartQuadRenderer::Queue(const Atrium::Matrix& aTransform, std::optional<Atrium::Color32> aColor, std::optional<Atrium::RectangleF> aUVRectangle)
{
	ZoneScoped;

	ChartQuadInstance& instance = myQuadInstanceData.emplace_back();
	instance.Transform = aTransform;

	const Atrium::Color32 color = aColor.value_or(Atrium::Color32::Predefined::White);
	instance.Color[0] = static_cast<float>(color.R) / 255.f;
	instance.Color[1] = static_cast<float>(color.G) / 255.f;
	instance.Color[2] = static_cast<float>(color.B) / 255.f;
	instance.Color[3] = static_cast<float>(color.A) / 255.f;

	const auto uvRectangle = aUVRectangle.value_or(Atrium::RectangleF(Atrium::PointF(0, 0), Atrium::SizeF(1, 1)));
	instance.UVMin = Atrium::Vector2(uvRectangle.TopLeft());
	instance.UVMax = Atrium::Vector2(uvRectangle.BottomRight());
}
