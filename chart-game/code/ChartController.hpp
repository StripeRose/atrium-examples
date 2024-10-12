// Filter "Chart/Playback"
#pragma once

#include "ChartCommonStructures.hpp"

#include <Core_InputEvent.hpp>

#include <array>
#include <chrono>

class ChartData;
class ChartTestWindow;

class ChartController
{
public:
	ChartController();
	virtual ~ChartController() = default;

	virtual const char* GetName() const = 0;

	ChartTrackType GetTrackType() const { return myTrackType; }
	ChartTrackDifficulty GetTrackDifficulty() const { return myTrackDifficulty; }

#if IS_IMGUI_ENABLED
	virtual void ImGui(ChartTestWindow& aTestWindow);
#endif

	void SetTrackType(ChartTrackType aType);
	void SetTrackDifficulty(ChartTrackDifficulty aDifficulty);

	virtual void OnChartChange([[maybe_unused]] const ChartData& aData) { }
	virtual void OnPlayheadStep([[maybe_unused]] const std::chrono::microseconds& aPrevious, [[maybe_unused]] const std::chrono::microseconds& aNew) { }

	virtual void HandleInput([[maybe_unused]] const Atrium::Core::InputEvent& anInputEvent) { }

protected:
	std::array<bool, 10> myLaneStates;

private:
	ChartTrackType myTrackType;
	ChartTrackDifficulty myTrackDifficulty;
};
