// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

#include <array>
#include <map>
#include <set>

struct ImGui_ChartDrawParameters;
class ChartAIController : public ChartController
{
public:

	virtual const char* GetName() const override { return "AI player"; }

	void HandleChartChange(const ChartData& aData) override;
	void HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew) override;

	#if IS_IMGUI_ENABLED
	void ImGui(ChartTestWindow& aTestWindow) override;
	#endif

	void SetTrackType(ChartTrackType aType) override;
	void SetTrackDifficulty(ChartTrackDifficulty aDifficulty) override;

private:
	#if IS_IMGUI_ENABLED
	void ImGui_DrawGrips(ChartTestWindow& aTestWindow, const ImGui_ChartDrawParameters& someParameters);
	#endif

	enum class StrumType { Always, IfNoCombo, Never };

	struct ChordGrip
	{
		std::chrono::microseconds Start;
		std::chrono::microseconds End;
		std::set<std::uint8_t> Lanes;
		StrumType Type = StrumType::Never;
	};

	void RefreshGrips();

	void RefreshGrips_AddNote(const ChartNoteRange& aNote);

	std::pair<std::vector<ChordGrip>::iterator, std::vector<ChordGrip>::iterator> RefreshGrips_CreateOrGetChordBlocksAt(const std::chrono::microseconds& aStart, const std::chrono::microseconds& anEnd);

	void RefreshGrips_SplitChordAt(const std::chrono::microseconds& aTimepoint);

	std::vector<ChordGrip> myGrips;
};