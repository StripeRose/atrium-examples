// Filter "Chart/Rendering"

#include "ChartFretboardRenderer.hpp"

#include "FretAtlas.hpp"

struct ModelViewProjection
{
	Atrium::Matrix Model;
	Atrium::Matrix View;
	Atrium::Matrix Projection;
};

void ChartFretboardRenderer::Render(Atrium::FrameGraphicsContext& aContext)
{
	aContext.SetPipelineState(myFretboardPipelineState);
	aContext.SetPipelineResource(Atrium::ResourceUpdateFrequency::PerObject, 0, myFretboardModelViewProjection);
	aContext.SetPipelineResource(Atrium::ResourceUpdateFrequency::PerMaterial, 0, myFretboardTexture);
	myFretboardMesh->DrawToFrame(aContext);
}

void ChartFretboardRenderer::Setup(Atrium::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::RootSignature>& aRootSignature, Atrium::GraphicsFormat aColorTargetFormat)
{
	ZoneScoped;

	myFretboardMesh = CreateFretboardMesh(aGraphicsAPI);
	myFretboardMesh->SetName(L"Fretboard vertices");

	Atrium::PipelineStateDescription fretboardPipelineDescription;
	fretboardPipelineDescription.RootSignature = aRootSignature;
	fretboardPipelineDescription.InputLayout = ChartFretboardVertex::GetInputLayout();
	const std::filesystem::path shaderPath = "ChartFretboard.hlsl";
	fretboardPipelineDescription.VertexShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Shader::Type::Vertex, "vertexShader");
	fretboardPipelineDescription.PixelShader = aGraphicsAPI.GetResourceManager().CreateShader(shaderPath, Atrium::Shader::Type::Pixel, "pixelShader");
	fretboardPipelineDescription.OutputFormats = { aColorTargetFormat };
	myFretboardPipelineState = aGraphicsAPI.GetResourceManager().CreatePipelineState(fretboardPipelineDescription);

	ModelViewProjection fretboardMatrices;
	fretboardMatrices.Model = Atrium::Matrix::Identity();
	fretboardMatrices.View = FretboardMatrices::CameraViewMatrix;
	fretboardMatrices.Projection = FretboardMatrices::CameraProjectionMatrix;

	myFretboardModelViewProjection = aGraphicsAPI.GetResourceManager().CreateGraphicsBuffer(Atrium::GraphicsBuffer::Target::Constant, 1, sizeof(ModelViewProjection));
	myFretboardModelViewProjection->SetData(&fretboardMatrices, sizeof(ModelViewProjection));
}

void ChartFretboardRenderer::SetTexture(std::shared_ptr<Atrium::Texture> aTexture)
{
	myFretboardTexture = aTexture;
}
