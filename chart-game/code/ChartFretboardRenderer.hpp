// Filter "Chart/Rendering"

#pragma once

#include "ChartMeshes.hpp"

#include <Core_GraphicsAPI.hpp>

class ChartFretboardRenderer
{
public:
	void Render(Atrium::Core::FrameContext& aContext);

	void Setup(
		Atrium::Core::GraphicsAPI& aGraphicsAPI,
		const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature,
		Atrium::Core::GraphicsFormat aColorTargetFormat
	);

	void SetTexture(std::shared_ptr<Atrium::Core::Texture> aTexture);

private:
	std::shared_ptr<Atrium::Core::Texture> myFretboardTexture;

	std::unique_ptr<Mesh> myFretboardMesh;
	std::shared_ptr<Atrium::Core::PipelineState> myFretboardPipelineState;
	std::shared_ptr<Atrium::Core::GraphicsBuffer> myFretboardModelViewProjection;
};
