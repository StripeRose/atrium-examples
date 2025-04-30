// Filter "Chart/Rendering"

#pragma once

#include "ChartMeshes.hpp"

#include <Atrium_GraphicsAPI.hpp>

class ChartFretboardRenderer
{
public:
	void Render(Atrium::FrameGraphicsContext& aContext);

	void Setup(
		Atrium::GraphicsAPI& aGraphicsAPI,
		const std::shared_ptr<Atrium::RootSignature>& aRootSignature,
		Atrium::GraphicsFormat aColorTargetFormat
	);

	void SetTexture(std::shared_ptr<Atrium::Texture> aTexture);

private:
	std::shared_ptr<Atrium::Texture> myFretboardTexture;

	std::unique_ptr<Mesh> myFretboardMesh;
	std::shared_ptr<Atrium::PipelineState> myFretboardPipelineState;
	std::shared_ptr<Atrium::GraphicsBuffer> myFretboardModelViewProjection;
};
