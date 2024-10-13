// Filter "Chart/Playback"
#pragma once

#include "ChartCommonStructures.hpp"
#include "ChartScoring.hpp"

#include <Core_InputEvent.hpp>

#include <array>
#include <chrono>
#include <span>

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

	virtual void HandleChartChange([[maybe_unused]] const ChartData& aData) { }
	virtual void HandlePlayheadStep([[maybe_unused]] const std::chrono::microseconds& aPrevious, [[maybe_unused]] const std::chrono::microseconds& aNew) { }
	virtual void HandleInput([[maybe_unused]] const Atrium::Core::InputEvent& anInputEvent) { }

	#if IS_IMGUI_ENABLED
	virtual void ImGui(ChartTestWindow& aTestWindow);
	#endif

	void SetTrackType(ChartTrackType aType);
	void SetTrackDifficulty(ChartTrackDifficulty aDifficulty);

protected:
	std::span<const bool> GetLaneStates() const { return myLaneStates; }

	void SetLane(std::uint8_t aLane, bool aState);
	void Strum();

private:
	void ImGui_Scoring();

	ChartScoring myScoring;
	ChartTrackType myTrackType;
	ChartTrackDifficulty myTrackDifficulty;

	std::array<bool, 10> myLaneStates;
};
