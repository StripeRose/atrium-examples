// Filter "Chart/Playback"
#include "ChartAIController.hpp"

#include "ChartData.hpp"
#include "ChartTestWindow.hpp"

#include "Atrium_Diagnostics.hpp"

#include "Atrium_GUI.hpp"

void ChartAIController::HandleChartChange(const ChartData& aData)
{
	ChartController::HandleChartChange(aData);

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

		switch (currentChord->Type)
		{
			case StrumType::IfNoCombo:
				if (GetScoring().GetStreak() > 0)
					break;

				[[fallthrough]];

			case StrumType::Always:
				if (aPrevious < currentChord->Start && currentChord->Start < aNew)
					Strum();
				break;
			default:
				break;
		}
	}
}

#if IS_IMGUI_ENABLED
void ChartAIController::ImGui(ChartTestWindow& aTestWindow)
{
	ChartController::ImGui(aTestWindow);

	if (ImGui::TreeNode("Grips"))
	{
		aTestWindow.ImGui_DrawChart({ 0, 150.f },
			[&](const ImGui_ChartDrawParameters& someParameters)
			{
				aTestWindow.ImGui_DrawChart_Lanes(someParameters, GetTrackType());

				ImGui_DrawGrips(aTestWindow, someParameters);
			});

		ImGui::TreePop();
	}
}
#endif

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

#if IS_IMGUI_ENABLED
void ChartAIController::ImGui_DrawGrips(ChartTestWindow&, const ImGui_ChartDrawParameters& someParameters)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	for (const ChordGrip& grip : myGrips)
	{
		const float gripXStart = someParameters.Point.X + someParameters.TimeToPoint(grip.Start);
		const float gripXEnd = someParameters.Point.X + someParameters.TimeToPoint(grip.End);

		if (gripXEnd < (someParameters.Point.X) || (someParameters.Point.X + someParameters.Size.X) < gripXStart)
			continue;

		const std::uint8_t laneCount = ChartTrackTypeLaneCount[static_cast<int>(GetTrackType())];

		const float laneHeight = (someParameters.Size.Y / laneCount);

		for (std::uint8_t lane : grip.Lanes)
		{
			const float top = someParameters.Point.Y + laneHeight * lane;
			const float bottom = top + laneHeight;

			drawList->AddRectFilled(
				{ gripXStart, top },
				{ gripXEnd, bottom },
				0x80FFFFFF
			);
		}

		ImU32 strumColor = 0xFFFFFFFF;
		float width = 5.f;
		switch (grip.Type)
		{
			case StrumType::Always:
				strumColor = IM_COL32(0xAA, 0xDD, 0xFF, 0xFF);
				width = 7.f;
				break;
			case StrumType::IfNoCombo:
				strumColor = IM_COL32(0xFF, 0xCC, 0xAA, 0xFF);
				width = 5.f;
				break;
			case StrumType::Never:
				strumColor = IM_COL32(0xFF, 0xFF, 0xFF, 0x80);
				width = 2.f;
				break;
		}

		drawList->AddRectFilled(
			{ gripXStart, someParameters.Point.Y },
			{ gripXStart + width, someParameters.Point.Y + someParameters.Size.Y },
			strumColor
		);
	}
}
#endif

void ChartAIController::RefreshGrips()
{
	ZoneScoped;

	myGrips.clear();

	const ChartTrack* track = GetTrack();

	if (!track)
		return;

	const auto difficultyIterator = track->GetNoteRanges().find(GetTrackDifficulty());
	if (difficultyIterator == track->GetNoteRanges().end())
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
