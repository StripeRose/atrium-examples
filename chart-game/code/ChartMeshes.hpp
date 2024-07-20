// Filter "Chart/Rendering"
#pragma once

#include "Mesh.hpp"

// Todo: Generalize to a Chart Quad, which is just a textured, colored quad for displaying graphics onto.
// It should be instantiable if all graphics are on a single texture, with a color.

static constexpr float FretboardHalfWidth = 0.4803f;
static constexpr float FretboardLength = 1.9212f;

struct ChartFretboardVertex
{
	static std::vector<RoseGold::Core::PipelineStateDescription::InputLayoutEntry> GetInputLayout();

	float Position[3];
	float UV[2];
};
using ChartFretboardMesh = MeshT<ChartFretboardVertex>;

std::unique_ptr<Mesh> CreateFretboardMesh(RoseGold::Core::GraphicsAPI& aGraphicsAPI);

struct ChartQuadVertex
{
	static std::vector<RoseGold::Core::PipelineStateDescription::InputLayoutEntry> GetInputLayout();

	float Position[3];
};

struct ChartQuadInstance
{
	RoseGold::Math::Matrix Transform;
	RoseGold::Math::Vector2 UVMin;
	RoseGold::Math::Vector2 UVMax;
	float Color[4];
};

using ChartQuadMesh = MeshT<ChartQuadVertex>;

std::unique_ptr<Mesh> CreateQuadMesh(RoseGold::Core::GraphicsAPI& aGraphicsAPI);