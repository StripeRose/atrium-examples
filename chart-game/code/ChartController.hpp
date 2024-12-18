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
class ChartTrack;

class ChartController
{
public:
	ChartController();
	virtual ~ChartController() = default;

	virtual bool AllowOpenNotes() const { return false; }

	virtual const char* GetName() const = 0;

	std::span<const bool> GetLaneStates() const { return myLaneStates; }
	const std::optional<std::chrono::microseconds>& GetLastStrum() const { return myLastStrum; }

	const ChartScoring& GetScoring() const { return myScoring; }

	ChartTrackType GetTrackType() const { return myTrackType; }
	ChartTrackDifficulty GetTrackDifficulty() const { return myTrackDifficulty; }

	virtual void HandleChartChange(const ChartData& aData);
	virtual void HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew);
	virtual void HandleInput([[maybe_unused]] const Atrium::Core::InputEvent& anInputEvent) { }

	#if IS_IMGUI_ENABLED
	virtual void ImGui(ChartTestWindow& aTestWindow);
	#endif

	virtual void SetTrackType(ChartTrackType aType);
	virtual void SetTrackDifficulty(ChartTrackDifficulty aDifficulty);

protected:
	void ClearLanes();

	const ChartTrack* GetTrack() const;

	void SetLane(std::uint8_t aLane, bool aState);

	void Strum();

private:
	void ImGui_Scoring();

	void CheckTapHit(std::uint8_t aLane);
	void CheckStrumHits();
	void CheckUnhitNotes();

	std::optional<float> CalculateNoteAccuracy(std::chrono::microseconds aPerfectTimepoint, std::chrono::microseconds aHitTimepoint) const;

	ChartScoring myScoring;
	ChartTrackType myTrackType;
	ChartTrackDifficulty myTrackDifficulty;
	const ChartData* myCurrentChart = nullptr;

	std::array<bool, 10> myLaneStates;
	std::chrono::microseconds myLastPlayhead;
	std::optional<std::chrono::microseconds> myLastStrum;

	// Per lane, the timepoint we've checked hits and misses up until.
	std::map<std::uint8_t, std::chrono::microseconds> myLastLaneHitCheck;
};
