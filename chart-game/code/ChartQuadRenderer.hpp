// Filter "Chart/Rendering"

#pragma once

#include "ChartMeshes.hpp"

#include <Core_GraphicsAPI.hpp>

class ChartQuadRenderer
{
public:
	void Flush(std::size_t aGroupID);

	void Setup(
		Atrium::Core::GraphicsAPI& aGraphicsAPI,
		const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature,
		Atrium::Core::GraphicsFormat aColorTargetFormat
	);

	void SetTexture(std::shared_ptr<Atrium::Core::Texture> aTexture);

	void Render(
		Atrium::Core::FrameContext& aContext,
		std::function<void(std::size_t)> aGroupPreparation
	);

	void Queue(
		const Atrium::Matrix& aTransform,
		std::optional<Atrium::Color32> aColor,
		std::optional<Atrium::RectangleF> aUVRectangle
	);

private:
	std::unique_ptr<Mesh> myQuadMesh;
	std::shared_ptr<Atrium::Core::PipelineState> myQuadPipelineState;

	std::vector<ChartQuadInstance> myQuadInstanceData;
	std::shared_ptr<Atrium::Core::GraphicsBuffer> myQuadInstanceBuffer;
	std::size_t myLastQuadFlush = 0;

	struct QuadInstanceGroup
	{
		std::size_t Start = 0;
		std::size_t Count = 0;
		std::size_t GroupID = 0;
	};
	std::vector<QuadInstanceGroup> myQuadGroups;

	std::shared_ptr<Atrium::Core::GraphicsBuffer> myCameraMatrices;
	std::shared_ptr<Atrium::Core::Texture> myTexture;
};
