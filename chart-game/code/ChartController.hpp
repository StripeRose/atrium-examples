// Filter "Chart/Playback"
#pragma once

#include "ChartCommonStructures.hpp"

#include <chrono>
#include <map>
#include <vector>

class ChartData;

class ChartController
{
public:
	ChartController();
	virtual ~ChartController() = default;

	virtual const char* GetName() const = 0;

	ChartTrackType GetTrackType() const { return myTrackType; }
	ChartTrackDifficulty GetTrackDifficulty() const { return myTrackDifficulty; }

#if IS_IMGUI_ENABLED
	virtual void ImGui();
#endif

	void SetTrackType(ChartTrackType aType);
	void SetTrackDifficulty(ChartTrackDifficulty aDifficulty);

	virtual void OnChartChange([[maybe_unused]] const ChartData& aData) { };
	virtual void OnPlayheadStep([[maybe_unused]] const std::chrono::microseconds& aPrevious, [[maybe_unused]] const std::chrono::microseconds& aNew) { }

private:
	ChartTrackType myTrackType;
	ChartTrackDifficulty myTrackDifficulty;
};

class ChartAIController : public ChartController
{
public:
	virtual const char* GetName() const override { return "AI player"; }

#if IS_IMGUI_ENABLED
	void ImGui() override;
#endif

	void OnChartChange(const ChartData& aData) override;
	void OnPlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew) override;

private:
	const ChartData* myCurrentChart = nullptr;
	std::map<std::uint8_t, std::pair<const ChartNoteRange*, std::chrono::microseconds>> myNextNotes;
};
