// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

#include <array>
#include <map>
#include <set>

class ChartAIController : public ChartController
{
public:

	virtual const char* GetName() const override { return "AI player"; }

	void HandleChartChange(const ChartData& aData) override;
	void HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew) override;

	void SetTrackType(ChartTrackType aType) override;
	void SetTrackDifficulty(ChartTrackDifficulty aDifficulty) override;

private:

	struct ChordGrip
	{
		std::chrono::microseconds Start;
		std::chrono::microseconds End;
		std::set<std::uint8_t> Lanes;
		ChartNoteType Type = ChartNoteType::Tap;
	};

	void RefreshGrips();

	void RefreshGrips_AddNote(const ChartNoteRange& aNote);

	std::pair<std::vector<ChordGrip>::iterator, std::vector<ChordGrip>::iterator> RefreshGrips_CreateOrGetChordBlocksAt(const std::chrono::microseconds& aStart, const std::chrono::microseconds& anEnd);

	void RefreshGrips_SplitChordAt(const std::chrono::microseconds& aTimepoint);

	const ChartData* myCurrentChart = nullptr;

	std::vector<ChordGrip> myGrips;
};