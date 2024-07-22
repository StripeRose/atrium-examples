// Filter "Chart"
#include "ChartTrack.hpp"

#include "MidiDecoder.hpp"

#include "Common_Diagnostics.hpp"
#include "Common_Math.hpp"

void ChartTrackLoadData::AddNote(std::chrono::microseconds aTime, std::uint8_t aNote, std::uint8_t aVelocity)
{
	// Run this even when velocity = 1, to make it "restart" the note.
	if (myPartialNotes.contains(aNote))
	{
		NoteRanges.push_back({ aNote, { myPartialNotes.at(aNote), aTime } });
		myPartialNotes.erase(aNote);
	}

	if (aVelocity > 0)
		myPartialNotes[aNote] = aTime;
}

void ChartTrackLoadData::AddSysEx(std::chrono::microseconds aTime, const std::span<std::uint8_t>& someData)
{
	SysExEvents.emplace_back(aTime, std::vector<std::uint8_t>(someData.begin(), someData.end()));
}

void ChartTrackLoadData::AddLyric(std::chrono::microseconds aTime, const std::string& aText)
{
	Lyrics.emplace(aTime, aText);
}

std::unique_ptr<ChartTrack> ChartTrack::CreateTrackByName(const std::string& aName)
{
	std::unique_ptr<ChartTrack> track;
	if (aName == "GUITAR")
	{
		track.reset(new ChartGuitarTrack());
		track->myType = ChartTrackType::LeadGuitar;
	}
	else if (aName == "RHYTHM")
	{
		track.reset(new ChartGuitarTrack());
		track->myType = ChartTrackType::RhythmGuitar;
	}
	else if (aName == "BASS")
	{
		track.reset(new ChartGuitarTrack());
		track->myType = ChartTrackType::BassGuitar;
	}
	else
	{
		RoseGold::Debug::LogError("Unknown track type: %s", aName.c_str());
	}

	return track;
}

const ChartNoteRange* ChartGuitarTrack::GetClosestNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const
{
	const std::vector<ChartNoteRange>& difficultyNotes = myNoteRanges.at(aDifficulty);

	const ChartNoteRange* closestNote = nullptr;
	std::chrono::microseconds distance = std::chrono::microseconds::max();

	for (const ChartNoteRange& noteRange : difficultyNotes)
	{
		if (noteRange.Lane != aLane)
			continue;

		const std::chrono::microseconds distanceToStart = RoseGold::Math::Abs(noteRange.Start - aTimepoint);
		const std::chrono::microseconds distanceToEnd = RoseGold::Math::Abs(noteRange.End - aTimepoint);

		if (distanceToStart < distance)
		{
			distance = distanceToStart;
			closestNote = &noteRange;
		}
		
		if (distanceToEnd < distance)
		{
			distance = distanceToEnd;
			closestNote = &noteRange;
		}
	}

	return closestNote;
}

const ChartNoteRange* ChartGuitarTrack::GetNextNote(ChartTrackDifficulty aDifficulty, std::uint8_t aLane, std::chrono::microseconds aTimepoint) const
{
	const std::vector<ChartNoteRange>& difficultyNotes = myNoteRanges.at(aDifficulty);

	const ChartNoteRange* closestNote = nullptr;
	std::chrono::microseconds distance = std::chrono::microseconds::max();

	for (const ChartNoteRange& noteRange : difficultyNotes)
	{
		if (noteRange.Lane != aLane)
			continue;

		const std::chrono::microseconds distanceToStart = (noteRange.Start - aTimepoint);
		if (distanceToStart.count() < 0)
			continue;

		if (distanceToStart < distance)
		{
			distance = distanceToStart;
			closestNote = &noteRange;
		}
	}

	return closestNote;
}

std::vector<ChartNoteRange> ChartGuitarTrack::GetNotesInRange(ChartTrackDifficulty aDifficulty, std::chrono::microseconds aStart, std::chrono::microseconds anEnd) const
{
	const std::vector<ChartNoteRange>& difficultyNotes = myNoteRanges.at(aDifficulty);

	std::vector<ChartNoteRange> notesInRange;
	notesInRange.reserve(difficultyNotes.size());

	for (const ChartNoteRange& noteRange : difficultyNotes)
	{
		if (noteRange.End < aStart || noteRange.Start >= anEnd)
			continue;

		notesInRange.push_back(noteRange);
	}

	return notesInRange;
}

bool ChartGuitarTrack::Load(const ChartTrackLoadData& someData)
{
	myNoteRanges.clear();
	myMarkers.clear();

	return Load_AddNotes(someData)
		&& Load_UpdateDefaultNoteTypes()
		&& Load_ProcessMarkers(someData)
		&& Load_ProcessSysEx(someData)
		;
}

bool ChartGuitarTrack::Load_AddNotes(const ChartTrackLoadData& someData)
{
	for (const auto& midiNoteRange : someData.NoteRanges)
	{
		const std::uint8_t midiNote = midiNoteRange.first;
		const std::pair<std::chrono::microseconds, std::chrono::microseconds>& range = midiNoteRange.second;

		std::uint8_t lane = 0;
		std::uint8_t octave = 0;
		MidiDecoder::DecomposeNoteNumber(midiNote, octave, lane);
		if (octave < 5 && octave >= 9)
			continue;

		if (lane < 5)
		{
			ChartNoteRange& newNoteRange = myNoteRanges[ChartTrackDifficulty(octave - 5)].emplace_back();
			newNoteRange.Lane = lane;
			newNoteRange.Start = range.first;
			newNoteRange.End = range.second;
		}
	}

#if IS_DEBUG_BUILD
	// Debug-only sorting of notes to make them make a bit more sense when inspecting.
	for (auto& difficulty : myNoteRanges)
	{
		std::sort(difficulty.second.begin(), difficulty.second.end(), [](const ChartNoteRange& a, const ChartNoteRange& b)
			{
				return a.Start < b.Start;
			}
		);
	}
#endif

	return true;
}

bool ChartGuitarTrack::Load_UpdateDefaultNoteTypes()
{
	// Todo: Update default note types based on surrounding notes.
	// https://github.com/TheNathannator/GuitarGame_ChartFormats/blob/main/doc/FileFormats/.mid/Standard/5-Fret%20Guitar.md#note-mechanics

	return true;
}

bool ChartGuitarTrack::Load_ProcessSysEx(const ChartTrackLoadData& someData)
{
	struct SysExPerDifficulty
	{
		std::optional<std::chrono::microseconds> OpenFlagStart;
		std::optional<std::chrono::microseconds> TapFlagStart;
	};

	std::map<ChartTrackDifficulty, SysExPerDifficulty> perDifficulty;

	auto handleSysExEvent = [&](const std::chrono::microseconds& aTime, const std::vector<std::uint8_t>& someData) -> bool
		{
			if (someData.size() != 7)
				return false;

			if (someData[0] != 'P' || someData[1] != 'S')
				return false;

			static constexpr std::uint8_t Easy = 0;
			static constexpr std::uint8_t Medium = 1;
			static constexpr std::uint8_t Hard = 2;
			static constexpr std::uint8_t Expert = 3;
			static constexpr std::uint8_t All = 0xFF;

			static constexpr std::uint8_t OpenNote = 0x10;
			static constexpr std::uint8_t TapNote = 0x40;

			const std::uint8_t difficulty = someData[4];
			const std::uint8_t type = someData[5];
			const std::uint8_t value = someData[6];

			for (std::uint8_t i = 0; i < ChartTrackDifficultyCount; ++i)
			{
				if (difficulty != All && i != difficulty)
					continue;

				SysExPerDifficulty& perDiff = perDifficulty[ChartTrackDifficulty(i)];
				std::optional<std::chrono::microseconds>* flag = nullptr;
				if (type == 0x01)
					flag = &perDiff.OpenFlagStart;
				else if (type == 0x04)
					flag = &perDiff.TapFlagStart;

				if (flag->has_value() == (value != 0))
					throw std::runtime_error("Overlapping sysex events. Please check.");

				if (value != 0)
				{
					*flag = aTime;
				}
				else
				{
					Load_ForEachNoteInRange([type](ChartNoteRange& aRange) {
						if (type == 0x01)
							aRange.CanBeOpen = true;
						else if (type == 0x04)
							aRange.Type = ChartNoteType::Tap;
						},
						ChartTrackLoadData::PerDifficultyFlag().set(i),
						flag->value(),
						aTime);
					(*flag).reset();
				}
			}

			return true;
		};

	for (const auto& sysEx : someData.SysExEvents)
	{
		if (handleSysExEvent(sysEx.first, sysEx.second))
			continue;

		std::string dataInHex;
		dataInHex.reserve(sysEx.second.size() * 3);
		static const char* digits = "0123456789ABCDEF";
		for (std::uint8_t d : sysEx.second)
		{
			dataInHex += digits[(d >> 4) & 0xF];
			dataInHex += digits[d & 0xF];
			dataInHex += ' ';
		}

		RoseGold::Debug::Log("Unknown SysEx event - % s", dataInHex.c_str());
	}

	return true;
}

bool ChartGuitarTrack::Load_ProcessMarkers(const ChartTrackLoadData& someData)
{
	for (const auto& midiNoteRange : someData.NoteRanges)
	{
		const std::uint8_t midiNote = midiNoteRange.first;
		const std::pair<std::chrono::microseconds, std::chrono::microseconds>& range = midiNoteRange.second;
		if (midiNote < 103) // Per difficulty midi notes.
		{
			std::uint8_t lane = 0;
			std::uint8_t octave = 0;
			MidiDecoder::DecomposeNoteNumber(midiNote, octave, lane);

			switch (lane)
			{
			case 5: // Force HOPO
				Load_ForEachNoteInRange(
					[](ChartNoteRange& noteRange) { noteRange.Type = ChartNoteType::HOPO; },
					ChartTrackLoadData::PerDifficultyFlag().set(octave - 5),
					range.first,
					range.second
				);
				break;
			case 6: // Force strum
				Load_ForEachNoteInRange(
					[](ChartNoteRange& noteRange) { noteRange.Type = ChartNoteType::Strum; },
					ChartTrackLoadData::PerDifficultyFlag().set(octave - 5),
					range.first,
					range.second
				);
				break;
			case 11: // Per-difficulty open markers
				if (someData.EnhancedOpens)
				{
					Load_ForEachNoteInRange(
						[](ChartNoteRange& noteRange) { noteRange.CanBeOpen = true; },
						ChartTrackLoadData::PerDifficultyFlag().set(octave - 4), // Enhanced opens is one octave below the difficulty octave they belong to.
						range.first,
						range.second
					);
				}
				break;
			}
		}
		else // All difficulty markers
		{
			MarkerRange& marker = myMarkers.emplace_back();
			marker.Start = range.first;
			marker.End = range.second;

			switch (midiNote)
			{
			case 103: marker.Marker = Marker::Solo; break;
			case 104: marker.Marker = Marker::TapNote; break;
			case 105: marker.Marker = Marker::P1VersusPhase; break;
			case 106: marker.Marker = Marker::P2VersusPhase; break;

			case 116: marker.Marker = Marker::StarPower; break;

			case 120: marker.Marker = Marker::BigRockEnding5; break;
			case 121: marker.Marker = Marker::BigRockEnding4; break;
			case 122: marker.Marker = Marker::BigRockEnding3; break;
			case 123: marker.Marker = Marker::BigRockEnding2; break;
			case 124: marker.Marker = Marker::BigRockEnding1; break;

			case 126: marker.Marker = Marker::TremoloLane; break;
			case 127: marker.Marker = Marker::TrillLane; break;

			default:
				RoseGold::Debug::LogWarning("Unknown marker note %s (%u)", MidiDecoder::NoteNumberToString(midiNote).c_str(), midiNote);
				break;
			}
		}
	}

	return true;
}

void ChartGuitarTrack::Load_ForEachNoteInRange(std::function<void(ChartNoteRange&)> aCallback, const ChartTrackLoadData::PerDifficultyFlag& someDifficulties, std::optional<std::chrono::microseconds> aMinimumRange, std::optional<std::chrono::microseconds> aMaximumRange)
{
	for (auto& noteRanges : myNoteRanges)
	{
		if (!someDifficulties.test(static_cast<std::size_t>(noteRanges.first)))
			continue;

		for (auto& noteRange : noteRanges.second)
		{
			if (aMinimumRange.has_value() && noteRange.Start < aMinimumRange.value())
				continue;

			if (aMaximumRange.has_value() && noteRange.End > aMaximumRange.value())
				continue;

			aCallback(noteRange);
		}
	}
}
