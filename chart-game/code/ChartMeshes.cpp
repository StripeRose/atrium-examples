// Filter "Chart/Rendering"
#include "ChartMeshes.hpp"

std::unique_ptr<Mesh> CreateFretboardMesh(Atrium::GraphicsAPI& aGraphicsAPI)
{
	std::unique_ptr<ChartFretboardMesh> mesh(new ChartFretboardMesh(aGraphicsAPI));
	std::vector<ChartFretboardVertex> vertices;

	vertices.emplace_back(ChartFretboardVertex{ { -FretboardHalfWidth, 0, 0 },					{ 0, 0 } });
	vertices.emplace_back(ChartFretboardVertex{ { -FretboardHalfWidth, 0, FretboardLength },	{ 0, 1 } });
	vertices.emplace_back(ChartFretboardVertex{ { FretboardHalfWidth, 0, 0 },					{ 1, 0 } });

	vertices.emplace_back(ChartFretboardVertex{ { FretboardHalfWidth, 0, 0 },					{ 1, 0 } });
	vertices.emplace_back(ChartFretboardVertex{ { -FretboardHalfWidth, 0, FretboardLength },	{ 0, 1 } });
	vertices.emplace_back(ChartFretboardVertex{ { FretboardHalfWidth, 0, FretboardLength },		{ 1, 1 } });

	mesh->SetVertices(vertices);
	return mesh;
}

std::unique_ptr<Mesh> CreateQuadMesh(Atrium::GraphicsAPI& aGraphicsAPI)
{
	std::unique_ptr<ChartQuadMesh> mesh(new ChartQuadMesh(aGraphicsAPI));
	std::vector<ChartQuadVertex> vertices;

	vertices.emplace_back(ChartQuadVertex{ 0, 0, 0 });
	vertices.emplace_back(ChartQuadVertex{ 1, 0, 0 });
	vertices.emplace_back(ChartQuadVertex{ 0, 1, 0 });

	vertices.emplace_back(ChartQuadVertex{ 1, 0, 0 });
	vertices.emplace_back(ChartQuadVertex{ 1, 1, 0 });
	vertices.emplace_back(ChartQuadVertex{ 0, 1, 0 });

	mesh->SetVertices(vertices);
	return mesh;
}

std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> ChartFretboardVertex::GetInputLayout()
{
	std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> layout;
	layout.emplace_back("POSITION", Atrium::GraphicsFormat::R32G32B32_SFloat);
	layout.emplace_back("TEXCOORD", Atrium::GraphicsFormat::R32G32_SFloat);
	return layout;
}

std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> ChartQuadVertex::GetInputLayout()
{
	std::vector<Atrium::PipelineStateDescription::InputLayoutEntry> layout;
	layout.emplace_back("POSITION", Atrium::GraphicsFormat::R32G32B32_SFloat);

	layout.emplace_back("TRANSFORM", 0, Atrium::GraphicsFormat::R32G32B32A32_SFloat, 1, 1);
	layout.emplace_back("TRANSFORM", 1, Atrium::GraphicsFormat::R32G32B32A32_SFloat, 1, 1);
	layout.emplace_back("TRANSFORM", 2, Atrium::GraphicsFormat::R32G32B32A32_SFloat, 1, 1);
	layout.emplace_back("TRANSFORM", 3, Atrium::GraphicsFormat::R32G32B32A32_SFloat, 1, 1);

	layout.emplace_back("TEXCOORD", 0, Atrium::GraphicsFormat::R32G32_SFloat, 1, 1);
	layout.emplace_back("TEXCOORD", 1, Atrium::GraphicsFormat::R32G32_SFloat, 1, 1);
	layout.emplace_back("COLOR", Atrium::GraphicsFormat::R32G32B32A32_SFloat, 1, 1);
	return layout;
}
