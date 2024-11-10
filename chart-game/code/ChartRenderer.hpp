// Filter "Chart/Rendering"
#pragma once

#include "ChartMeshes.hpp"
#include "ChartQuadRenderer.hpp"
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
	void SetupFretboardResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, const std::shared_ptr<Atrium::Core::RootSignature>& aRootSignature, Atrium::Core::GraphicsFormat aColorTargetFormat);

	std::pair<int, int> GetControllerRectanglesGrid(const Atrium::RectangleF& aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const;
	std::vector<Atrium::RectangleF> GetControllerRectangles(const Atrium::RectangleF& aTotalRectangle, std::size_t aControllerCount) const;

	void RenderController(ChartController& aController);
	void RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack);

	void RenderNote_Guitar(const ChartNoteRange& aNote);
	void RenderNote_GuitarOpen(const ChartNoteRange& aNote);
	void RenderNote_GuitarSustain(const ChartNoteRange& aNote);
	void RenderNote_GuitarOpenSustain(const ChartNoteRange& aNote);

	void QueueFretboardQuads();

	float TimeToPositionOffset(std::chrono::microseconds aTime) const;

	ChartPlayer& myPlayer;

	ChartQuadRenderer myQuadRenderer;

	std::shared_ptr<Atrium::Core::Texture> myFretboardTexture;

	std::unique_ptr<Mesh> myFretboardMesh;
	std::shared_ptr<Atrium::Core::PipelineState> myFretboardPipelineState;
	std::shared_ptr<Atrium::Core::GraphicsBuffer> myFretboardModelViewProjection;
};
