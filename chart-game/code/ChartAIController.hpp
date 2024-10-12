// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

#include <map>

class ChartAIController : public ChartController
{
public:
	virtual const char* GetName() const override { return "AI player"; }

	#if IS_IMGUI_ENABLED
	void ImGui(ChartTestWindow& aTestWindow) override;
	#endif

	void OnChartChange(const ChartData& aData) override;
	void OnPlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew) override;

private:
	const ChartData* myCurrentChart = nullptr;
	std::map<std::uint8_t, std::pair<const ChartNoteRange*, std::chrono::microseconds>> myClosestNotes;
};