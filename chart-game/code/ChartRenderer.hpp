// Filter "Chart/Rendering"

#pragma once

#include "ChartMeshes.hpp"
#include "ChartFretboardRenderer.hpp"
#include "ChartQuadRenderer.hpp"
#include "Mesh.hpp"

#include "Atrium_Math.hpp"

#include "Atrium_FrameContext.hpp"

class ChartController;
class ChartGuitarTrack;
class ChartPlayer;

struct ChartNoteRange;

class ChartRenderer
{
public:
	ChartRenderer(ChartPlayer& aPlayer);

	#if IS_IMGUI_ENABLED
	void ImGui();
	#endif

	void SetupResources(Atrium::Core::GraphicsAPI& aGraphicsAPI, Atrium::Core::GraphicsFormat aColorTargetFormat);

	void Render(Atrium::Core::FrameGraphicsContext& aContext, const std::shared_ptr<Atrium::Core::RenderTexture>& aTarget);

private:
	enum class SustainState { Missed, Neutral, Active };

	std::pair<int, int> GetControllerRectanglesGrid(const Atrium::RectangleF& aTotalRectangle, float aGridCellAspectRatio, std::size_t aControllerCount) const;
	std::vector<Atrium::RectangleF> GetControllerRectangles(const Atrium::RectangleF& aTotalRectangle, std::size_t aControllerCount) const;

	void RenderController(ChartController& aController);
	void RenderNotes(ChartController& aController, const ChartGuitarTrack& aTrack);

	void RenderNote_Guitar(const ChartNoteRange& aNote);
	void RenderNote_GuitarOpen(const ChartNoteRange& aNote);
	void RenderNote_GuitarSustain(const ChartNoteRange& aNote, SustainState aState, std::optional<std::chrono::microseconds> anOverrideStart = {});
	void RenderNote_GuitarOpenSustain(const ChartNoteRange& aNote, std::optional<std::chrono::microseconds> anOverrideStart = {});

	void QueueTargets(ChartController& aController);

	float TimeToPositionOffset(std::chrono::microseconds aTime) const;

	ChartPlayer& myPlayer;

	ChartQuadRenderer myQuadRenderer;
	ChartFretboardRenderer myFretboardRenderer;
};
