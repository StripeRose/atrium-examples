// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

#include <map>

class ChartAIController : public ChartController
{
public:
	virtual const char* GetName() const override { return "AI player"; }

	void HandleChartChange(const ChartData& aData) override;
	void HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew) override;

	#if IS_IMGUI_ENABLED
	void ImGui(ChartTestWindow& aTestWindow) override;
	#endif

private:
	const ChartData* myCurrentChart = nullptr;
	std::map<std::uint8_t, std::pair<const ChartNoteRange*, std::chrono::microseconds>> myClosestNotes;
};