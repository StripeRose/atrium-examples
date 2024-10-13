// Filter "Chart"
#pragma once

#include "ChartCommonStructures.hpp"

#include <array>
#include <bitset>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>

struct ChartTrackLoadData
{
	using PerDifficultyFlag = std::bitset<ChartTrackDifficultyCount>;
	void AddNote(std::chrono::microseconds aTime, std::uint8_t aNote, std::uint8_t aVelocity);
	void AddSysEx(std::chrono::microseconds aTime, const std::span<const std::uint8_t>& someData);
	void AddLyric(std::chrono::microseconds aTime, const std::string& aText);

	std::map<std::uint8_t, std::chrono::microseconds> myPartialNotes;

	std::vector<std::pair<std::uint8_t, std::pair<std::chrono::microseconds, std::chrono::microseconds>>> NoteRanges;
	std::vector<std::pair<std::chrono::microseconds, std::vector<std::uint8_t>>> SysExEvents;
	std::map<std::chrono::microseconds, std::string> Lyrics;

	bool EnhancedOpens = false;
};

class ChartTrack
{
public:
	static std::unique_ptr<ChartTrack> CreateTrackByName(const std::string& aName);

public:
	virtual ~ChartTrack() = default;

	virtual const ChartNoteRange* GetClosestNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const = 0;

	virtual const ChartNoteRange* GetNextNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const = 0;

	virtual std::vector<ChartNoteRange> GetNotesInRange(ChartTrackDifficulty aDifficulty, std::chrono::microseconds aStart, std::chrono::microseconds anEnd) const = 0;

	virtual bool Load(const ChartTrackLoadData& someData) = 0;

	ChartTrackType GetType() const { return myType; }

private:
	ChartTrackType myType = ChartTrackType::LeadGuitar;
};

class ChartGuitarTrack : public ChartTrack
{
public:
	enum class Marker
	{
		Solo,
		TapNote,
		P1VersusPhase,
		P2VersusPhase,

		StarPower,

		BigRockEnding1,
		BigRockEnding2,
		BigRockEnding3,
		BigRockEnding4,
		BigRockEnding5,

		TremoloLane,
		TrillLane
	};

	struct MarkerRange
	{
		Marker Marker = Marker::Solo;
		std::chrono::microseconds Start = std::chrono::microseconds(0);
		std::chrono::microseconds End = std::chrono::microseconds(0);
	};

public:
	const std::map<ChartTrackDifficulty, std::vector<ChartNoteRange>>& GetNoteRanges() const { return myNoteRanges; }
	const ChartNoteRange* GetClosestNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const override;

	const ChartNoteRange* GetNextNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const override;

	std::vector<ChartNoteRange> GetNotesInRange(ChartTrackDifficulty aDifficulty, std::chrono::microseconds aStart, std::chrono::microseconds anEnd) const override;

	bool Load(const ChartTrackLoadData& someData) override;

private:
	bool Load_AddNotes(const ChartTrackLoadData& someData);
	bool Load_UpdateDefaultNoteTypes();
	bool Load_ProcessSysEx(const ChartTrackLoadData& someData);
	bool Load_ProcessMarkers(const ChartTrackLoadData& someData);
	void Load_ForEachNoteInRange(std::function<void(ChartNoteRange&)> aCallback, const ChartTrackLoadData::PerDifficultyFlag& someDifficulties, std::optional<std::chrono::microseconds> aMinimumRange = {}, std::optional<std::chrono::microseconds> aMaximumRange = {});

	std::map<ChartTrackDifficulty, std::vector<ChartNoteRange>> myNoteRanges;
	std::vector<MarkerRange> myMarkers;
};
