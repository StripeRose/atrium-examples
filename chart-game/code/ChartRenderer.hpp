// Filter "Chart/Rendering"
#pragma once

#include "ChartMeshes.hpp"
#include "Mesh.hpp"

#include "Core_Math.hpp"

#include "Core_FrameContext.hpp"

class ChartController;
class ChartGuitarTrack;
class ChartPlayer;
struct ChartNoteRange;
class ChartRenderer
{
public:
	ChartRenderer(ChartPlayer& aPlayer);

	void SetupResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, Atrium::Core::GraphicsFormat aColorTargetFormat);

	void Render(Atrium::Core::FrameContext& aContext, const std::shared_ptr<Atrium::Core::RenderTexture>& aTarget);

private:
	void SetupQuadResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat);
	void SetupFretboardResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat);

	std::pair<int, int> GetControllerRectanglesGrid(const Atrium::RectangleF& aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const;
	std::vector<Atrium::RectangleF> GetControllerRectangles(const Atrium::RectangleF& aTotalRectangle, std::size_t aControllerCount) const;

	void RenderController(ChartController& aController);
	void RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack);

	void QueueFretboardQuads();

	void QueueQuad(const Atrium::Matrix& aTransform, std::optional<Atrium::Color32> aColor, std::optional<Atrium::RectangleF> aUVRectangle);
	void FlushQuads(Atrium::Core::FrameContext& aContext);

	ChartPlayer& myPlayer;

	std::unique_ptr<Mesh> myQuadMesh;
	std::shared_ptr<Atrium::Core::PipelineState> myQuadPipelineState;

	std::shared_ptr<Atrium::Core::Texture> myAtlas;
	std::shared_ptr<Atrium::Core::Texture> myFretboardTexture;

	std::vector<ChartQuadInstance> myQuadInstanceData;
	std::shared_ptr<Atrium::Core::GraphicsBuffer> myQuadInstanceBuffer;
	std::size_t myLastQuadFlush = 0;

	std::shared_ptr<Atrium::Core::GraphicsBuffer> myCameraMatrices;

	std::unique_ptr<Mesh> myFretboardMesh;
	std::shared_ptr<Atrium::Core::PipelineState> myFretboardPipelineState;
	std::shared_ptr<Atrium::Core::GraphicsBuffer> myFretboardModelViewProjection;
};
