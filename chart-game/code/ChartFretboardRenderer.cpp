// Filter "Chart/Rendering"

#include "ChartFretboardRenderer.hpp"

#include "FretAtlas.hpp"

struct ModelViewProjection
{
	Atrium::Matrix Model;
	Atrium::Matrix View;
	Atrium::Matrix Projection;
};

void ChartFretboardRenderer::Render(Atrium::Core::FrameContext& aContext)
{
	aContext.SetPipelineState(myFretboardPipelineState);
	aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerObject, 0, myFretboardModelViewProjection);
	aContext.SetPipelineResource(Atrium::Core::ResourceUpdateFrequency::PerMaterial, 0, myFretboardTexture);
	myFretboardMesh->DrawToFrame(aContext);
}

void ChartFretboardRenderer::Setup(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	myFretboardMesh = CreateFretboardMesh(aGraphicsAPI);
	myFretboardMesh->SetName(L"Fretboard vertices");

	Atrium::Core::PipelineStateDescription fretboardPipelineDescription;
	fretboardPipelineDescription.RootSignature = aRootSignature;
	fretboardPipelineDescription.InputLayout = ChartFretboardVertex::GetInputLayout();
	const std::filesystem::path shaderPath = "ChartFretboard.hlsl";
	fretboardPipelineDescription.VertexShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Vertex, "vertexShader");
	fretboardPipelineDescription.PixelShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Core::Shader::Type::Pixel, "pixelShader");
	fretboardPipelineDescription.OutputFormats = { aColorTargetFormat };
	myFretboardPipelineState = aGraphicsAPI.GetResourceManager().CreatePipelineState(fretboardPipelineDescription);

	ModelViewProjection fretboardMatrices;
	fretboardMatrices.Model = Atrium::Matrix::Identity();
	fretboardMatrices.View = FretboardMatrices::CameraViewMatrix;
	fretboardMatrices.Projection = FretboardMatrices::CameraProjectionMatrix;

	myFretboardModelViewProjection = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::Core::GraphicsBuffer::Target::Constant, 1, sizeof(ModelViewProjection));
	myFretboardModelViewProjection->SetData(&fretboardMatrices, sizeof(ModelViewProjection));
}

void ChartFretboardRenderer::SetTexture(std::shared_ptr<Atrium::Core::Texture> aTexture)
{
	myFretboardTexture = aTexture;
}
