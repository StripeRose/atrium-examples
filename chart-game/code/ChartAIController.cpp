// Filter "Chart/Playback"
#include "ChartAIController.hpp"

#include "ChartData.hpp"

#include "Core_Diagnostics.hpp"

#include "Editor_GUI.hpp"

void ChartAIController::HandleChartChange(const ChartData& aData)
{
	ZoneScoped;

	ChartController::HandleChartChange(aData);

	myCurrentChart = &aData;

	RefreshGrips();
}

void ChartAIController::HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew)
{
	ChartController::HandlePlayheadStep(aPrevious, aNew);

	const auto currentChord = std::find_if(
		myGrips.cbegin(), myGrips.cend(),
		[&](const ChordGrip& aChord)
		{
			return aChord.Start <= aNew && aNew < aChord.End;
		}
	);

	ClearLanes();

	if (currentChord != myGrips.cend())
	{
		for (std::uint8_t lane : currentChord->Lanes)
			SetLane(lane, true);

		// Todo: Add "NoCombo" strumming, only if the combo has been lost.
		if (currentChord->Type == StrumType::Always && aPrevious < currentChord->Start && currentChord->Start < aNew)
			Strum();
	}
}

void ChartAIController::SetTrackType(ChartTrackType aType)
{
	ChartController::SetTrackType(aType);

	RefreshGrips();
}

void ChartAIController::SetTrackDifficulty(ChartTrackDifficulty aDifficulty)
{
	ChartController::SetTrackDifficulty(aDifficulty);

	RefreshGrips();
}

void ChartAIController::RefreshGrips()
{
	ZoneScoped;

	myGrips.clear();

	if (!myCurrentChart)
		return;

	const auto trackIterator = myCurrentChart->GetTracks().find(GetTrackType());
	if (trackIterator == myCurrentChart->GetTracks().end())
		return;

	const ChartTrack& track = *trackIterator->second;

	const auto difficultyIterator = track.GetNoteRanges().find(GetTrackDifficulty());
	if (difficultyIterator == track.GetNoteRanges().end())
		return;

	const std::vector<ChartNoteRange>& trackNotes = difficultyIterator->second;

	for (const ChartNoteRange& note : trackNotes)
		RefreshGrips_AddNote(note);

	// Todo: Assert that the chord list is in order and has no overlap.
}

void ChartAIController::RefreshGrips_AddNote(const ChartNoteRange& aNote)
{
	ZoneScoped;

	std::chrono::microseconds duration = aNote.End - aNote.Start;
	duration = Atrium::Math::Max(duration, std::chrono::microseconds(150'000));

	const std::chrono::microseconds adjustedEnd = aNote.Start + duration;

	const auto chords = RefreshGrips_CreateOrGetChordBlocksAt(aNote.Start, adjustedEnd);

	for (auto chord = chords.first; chord != chords.second; chord++)
	{
		// Only the start of the note needs to be strummed.
		if (chord == chords.first)
		{
		switch (aNote.Type)
		{
			case ChartNoteType::Strum:
					chord->Type = StrumType::Always;
				break;
			case ChartNoteType::HOPO:
					if (chord->Type == StrumType::Never)
						chord->Type = StrumType::IfNoCombo;
				break;
		}
		}

		chord->Lanes.insert(aNote.Lane);
	}
}

std::pair<std::vector<ChartAIController::ChordGrip>::iterator, std::vector<ChartAIController::ChordGrip>::iterator> ChartAIController::RefreshGrips_CreateOrGetChordBlocksAt(const std::chrono::microseconds& aStart, const std::chrono::microseconds& anEnd)
{
	ZoneScoped;

	RefreshGrips_SplitChordAt(aStart);
	RefreshGrips_SplitChordAt(anEnd);

	auto startChord = std::find_if(
		myGrips.begin(), myGrips.end(),
		[&aStart](const ChordGrip& aChord)
		{
			return (aChord.Start <= aStart && aStart < aChord.End) || aChord.Start > aStart;
		}
	);

	// No chords after aStart, just create one.
	if (startChord == myGrips.end())
	{
		ChordGrip& newChord = myGrips.emplace_back();
		newChord.Start = aStart;
		newChord.End = anEnd;
		return { myGrips.end() - 1, myGrips.end() };
	}

	// First chord started after aStart, need to fill the gap.
	if (startChord->Start != aStart)
	{
		std::chrono::microseconds emptyBlockEnd = startChord->Start;
		startChord = myGrips.emplace(startChord);
		startChord->Start = aStart;
		startChord->End = emptyBlockEnd;
	}

	// We got the first chord block in the range.
	const std::size_t startChordIndex = static_cast<std::size_t>(startChord - myGrips.begin());

	// Time to find the last.
	for (std::size_t i = startChordIndex; i < myGrips.size(); ++i)
	{
		const ChordGrip& currentChord = myGrips.at(i);

		if (currentChord.End == anEnd)
		{
			return { myGrips.begin() + startChordIndex, myGrips.begin() + i + 1 };
		}

		if (i < (myGrips.size() - 1))
		{
			const ChordGrip& nextChord = myGrips.at(i + 1);
			if (currentChord.End != nextChord.Start)
			{
				const std::chrono::microseconds inbetweenEnd = nextChord.Start;
				ChordGrip& inbetween = *myGrips.emplace(myGrips.begin() + (i + 1));
				inbetween.Start = currentChord.End;
				inbetween.End = inbetweenEnd;
			}
		}
		else
		{
			ChordGrip& endChord = myGrips.emplace_back();
			endChord.Start = currentChord.End;
			endChord.End = anEnd;
			return { myGrips.begin() + startChordIndex, myGrips.end() };
		}
	}

	throw std::out_of_range("Couldn't find or create the last chord.");
}

void ChartAIController::RefreshGrips_SplitChordAt(const std::chrono::microseconds& aTimepoint)
{
	ZoneScoped;

	auto firstHalf = std::find_if(
		myGrips.begin(), myGrips.end(),
		[&aTimepoint](const ChordGrip& aChord)
		{
			return aChord.Start < aTimepoint && aTimepoint < aChord.End;
		}
	);

	if (firstHalf == myGrips.end())
		return;

	if (firstHalf->Start == aTimepoint || firstHalf->End == aTimepoint)
		return;

	auto secondHalf = myGrips.emplace(firstHalf + 1, *firstHalf);
	secondHalf->Start = aTimepoint;
	(secondHalf - 1)->End = aTimepoint;

	// After the split, the second chord will contain the same notes continued, and so doesn't need to be re-strummed.
	secondHalf->Type = StrumType::Never;
}
