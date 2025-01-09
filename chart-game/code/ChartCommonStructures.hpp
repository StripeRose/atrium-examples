// Filter "Chart"
#pragma once

#include <stdint.h>
#include <chrono>

#if IS_IMGUI_ENABLED
static constexpr const char* ChartTrackTypeCombo = "Lead guitar\0Rhythm guitar\0Bass guitar\0Drums\0Vocals\0Harmony vocals\0\0";
static constexpr const char* ChartTrackDifficultyCombo = "Easy\0Medium\0Hard\0Expert\0\0";
#endif

enum class ChartTrackType
{
	LeadGuitar,
	RhythmGuitar,
	BassGuitar,
	Drums,
	Vocal_Main,
	Vocal_Harmony
};
static constexpr unsigned int ChartTrackTypeCount = static_cast<unsigned int>(ChartTrackType::Vocal_Harmony) + 1;

static constexpr std::uint8_t ChartTrackTypeLaneCount[] = {
	5u,
	5u,
	5u,
	0u,
	0u,
	0u
};

enum class ChartTrackDifficulty
{
	Easy,
	Medium,
	Hard,
	Expert
};
static constexpr unsigned int ChartTrackDifficultyCount = static_cast<unsigned int>(ChartTrackDifficulty::Expert) + 1;

enum class ChartNoteType
{
	Strum,
	HOPO,
	Tap
};

struct ChartNoteRange
{
	std::uint8_t Lane = 0;
	ChartNoteType Type = ChartNoteType::Strum;
	bool CanBeOpen = false;

	std::chrono::microseconds Start = std::chrono::microseconds(0);
	std::chrono::microseconds End = std::chrono::microseconds(0);

	bool IsSustain() const
	{
		return (End - Start) >= std::chrono::microseconds(10'000);
	}
};
