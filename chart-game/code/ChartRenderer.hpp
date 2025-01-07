// Filter "Chart/Rendering"

#pragma once

#include "ChartMeshes.hpp"
#include "ChartFretboardRenderer.hpp"
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

	void Render(Atrium::Core::FrameGraphicsContext& aContext, const std::shared_ptr<Atrium::Core::RenderTexture>& aTarget);

private:
	std::pair<int, int> GetControllerRectanglesGrid(const Atrium::RectangleF& aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const;
	std::vector<Atrium::RectangleF> GetControllerRectangles(const Atrium::RectangleF& aTotalRectangle, std::size_t aControllerCount) const;

	void RenderController(ChartController& aController);
	void RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack);

	void RenderNote_Guitar(const ChartNoteRange& aNote);
	void RenderNote_GuitarOpen(const ChartNoteRange& aNote);
	void RenderNote_GuitarSustain(const ChartNoteRange& aNote);
	void RenderNote_GuitarOpenSustain(const ChartNoteRange& aNote);

	void QueueTargets();

	float TimeToPositionOffset(std::chrono::microseconds aTime) const;

	ChartPlayer& myPlayer;

	ChartQuadRenderer myQuadRenderer;
	ChartFretboardRenderer myFretboardRenderer;
};
