// Filter "Chart/Rendering"
#pragma once

#include "ChartMeshes.hpp"
#include "Mesh.hpp"

#include "Common_Math.hpp"

#include "Graphics_FrameContext.hpp"

class ChartController;
class ChartGuitarTrack;
class ChartPlayer;
struct ChartNoteRange;
class ChartRenderer
{
public:
	ChartRenderer(ChartPlayer& aPlayer);

	void SetupResources(RoseGold::Core::GraphicsAPI& aGraphicsAPI, RoseGold::Core::GraphicsFormat aColorTargetFormat);

	void Render(RoseGold::Core::FrameContext& aContext, const std::shared_ptr<RoseGold::Core::RenderTexture>& aTarget);

private:
	void SetupQuadResources(RoseGold::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<RoseGold::Core::RootSignature>& aRootSignature, RoseGold::Core::GraphicsFormat aColorTargetFormat);
	void SetupFretboardResources(RoseGold::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<RoseGold::Core::RootSignature>& aRootSignature, RoseGold::Core::GraphicsFormat aColorTargetFormat);

	std::pair<int, int> GetControllerRectanglesGrid(RoseGold::Math::Rectangle aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const;
	std::vector<RoseGold::Math::Rectangle> GetControllerRectangles(RoseGold::Math::Rectangle aTotalRectangle, std::size_t aControllerCount) const;

	void RenderController(ChartController& aController);
	void RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack);

	void QueueFretboardQuads();

	void QueueQuad(RoseGold::Math::Matrix aTransform, std::optional<RoseGold::Color32> aColor, std::optional<RoseGold::Math::Rectangle> aUVRectangle);
	void FlushQuads(RoseGold::Core::FrameContext& aContext);

	ChartPlayer& myPlayer;

	std::unique_ptr<Mesh> myQuadMesh;
	std::shared_ptr<RoseGold::Core::PipelineState> myQuadPipelineState;

	std::shared_ptr<RoseGold::Core::Texture> myAtlas;
	std::shared_ptr<RoseGold::Core::Texture> myFretboardTexture;

	std::vector<ChartQuadInstance> myQuadInstanceData;
	std::shared_ptr<RoseGold::Core::GraphicsBuffer> myQuadInstanceBuffer;
	std::size_t myLastQuadFlush = 0;

	std::shared_ptr<RoseGold::Core::GraphicsBuffer> myCameraMatrices;

	std::unique_ptr<Mesh> myFretboardMesh;
	std::shared_ptr<RoseGold::Core::PipelineState> myFretboardPipelineState;
	std::shared_ptr<RoseGold::Core::GraphicsBuffer> myFretboardModelViewProjection;
};
