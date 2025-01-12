// Filter "Chart/Playback"
#include "ChartController.hpp"

#include "ChartTestWindow.hpp"

#include "Core_Diagnostics.hpp"

#include "Editor_GUI.hpp"

#define NOTE_LOWEST_ACCURACY std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(50))

ChartController::ChartController()
	: myTrackDifficulty(ChartTrackDifficulty::Hard)
	, myTrackType(ChartTrackType::LeadGuitar)
{
	myLaneStates.fill(false);
}

std::optional<std::chrono::microseconds> ChartController::GetNoteHitEnd(const ChartNoteRange& aNoteRange) const
{
	for (const auto it : myHitNoteRanges)
	{
		const bool isSame
			= (it.first == &aNoteRange)
			|| (it.first->Lane == aNoteRange.Lane
			&& it.first->Start == aNoteRange.Start
			&& it.first->End == aNoteRange.End)
			;

		if (isSame)
			return it.second;
	}

	return { };
}

void ChartController::HandleChartChange(const ChartData& aData)
{
	myScoring.Reset();
	myCurrentChart = &aData;

	myLaneStates.fill(false);
	myLaneLastStrum.fill(std::chrono::microseconds(0));

	myLastPlayhead = std::chrono::microseconds(0);
	myLastStrum.reset();

	myLastLaneHitCheck.clear();
	myActiveSustains.clear();
	myHitNoteRanges.clear();
}

void ChartController::HandlePlayheadStep(const std::chrono::microseconds& aPrevious, const std::chrono::microseconds& aNew)
{
	if (aNew >= aPrevious)
	{
		UpdateActiveSustains(aPrevious, aNew);
		CheckUnhitNotes(aNew);
	}
	else
	{
		// If we seek backwards we don't want to keep going with the same state.
		// Reset most things; similar to restarting and seeking forward.

		myScoring.Reset();

		myLaneStates.fill(false);
		myLaneLastStrum.fill(std::chrono::microseconds(0));
		myLastStrum.reset();

		myLastLaneHitCheck.clear();
		myActiveSustains.clear();
		myHitNoteRanges.clear();
	}

	myLastPlayhead = aNew;
}

#if IS_IMGUI_ENABLED
void ChartController::ImGui(ChartTestWindow& aTestWindow)
{
	aTestWindow.ImGui_GuitarControlState(*this);
	ImGui_Scoring();

	int currentTrackType = static_cast<int>(GetTrackType());
	if (ImGui::Combo("Track", &currentTrackType, ChartTrackTypeCombo))
	{
		SetTrackType(ChartTrackType(currentTrackType));
	}

	ImGui::TableNextColumn();

	int currentDifficulty = static_cast<int>(GetTrackDifficulty());
	if (ImGui::Combo("Difficulty", &currentDifficulty, ChartTrackDifficultyCombo))
	{
		SetTrackDifficulty(ChartTrackDifficulty(currentDifficulty));
	}
}

void ChartController::ImGui_Scoring()
{
	ImGui::Text("Score: %i (x%i)", myScoring.GetScore(), myScoring.GetMultiplier());
	ImGui::Text("Streak: %i (Max: %i)", myScoring.GetStreak(), myScoring.GetMaximumStreak());
	ImGui::Text("Accuracy: %.0f%% (%i / %i)", myScoring.GetAccuracy() * 100.f, myScoring.GetHitCount(), myScoring.GetNoteCount());

	if (ImGui::SmallButton("Reset stats"))
		myScoring.Reset();
}
#endif

void ChartController::CheckTapHit(std::uint8_t aLane)
{
	const ChartTrack* track = GetTrack();
	if (track == nullptr)
		return;

	const std::chrono::microseconds lastHitCheck = myLastLaneHitCheck.contains(aLane) ? myLastLaneHitCheck.at(aLane) : std::chrono::microseconds(0);

	const ChartNoteRange* nextNote = track->GetNextNote(GetTrackDifficulty(), aLane, lastHitCheck);
	if (!nextNote)
		return;

	if (nextNote->Type == ChartNoteType::Strum)
		return;
	
	if (nextNote->Type == ChartNoteType::HOPO && GetScoring().GetStreak() == 0)
		return;

	const std::optional<float> accuracy = CalculateNoteAccuracy(nextNote->Start, myLastPlayhead);

	if (!accuracy.has_value())
		return;

	if (nextNote->IsSustain())
		myActiveSustains.insert(nextNote);

	myHitNoteRanges[nextNote] = nextNote->IsSustain() ? nextNote->Start : nextNote->End;
	myScoring.HitValidNotes(1);
	myLastLaneHitCheck[aLane] = myLastPlayhead;
}

void ChartController::CheckStrumHits()
{
	Atrium::Debug::Assert(myLastStrum.has_value(), "Need a strum time to check for hits.");

	const ChartTrack* track = GetTrack();
	if (track == nullptr)
		return;

	const std::uint8_t laneCount = ChartTrackTypeLaneCount[static_cast<int>(GetTrackType())];

	bool overStrummed = false;

	for (std::uint8_t lane = 0; lane < laneCount; ++lane)
	{
		if (myLaneStates[lane] == false)
			continue;

		const std::chrono::microseconds lastHitCheck = myLastLaneHitCheck.contains(lane) ? myLastLaneHitCheck.at(lane) : std::chrono::microseconds(0);

		const ChartNoteRange* nextNote = track->GetNextNote(GetTrackDifficulty(), lane, lastHitCheck);
		if (!nextNote)
			continue;

		// Assumes strumming a note is never incorrect.

		const std::optional<float> accuracy = CalculateNoteAccuracy(nextNote->Start, myLastStrum.value());

		if (accuracy)
		{
			if (nextNote->IsSustain())
				myActiveSustains.insert(nextNote);

			myHitNoteRanges[nextNote] = nextNote->IsSustain() ? nextNote->Start : nextNote->End;
			myScoring.HitValidNotes(1);
		}
		else
		{
			overStrummed = true;
		}

		myLastLaneHitCheck[lane] = myLastPlayhead;
	}

	if (overStrummed)
		myScoring.HitInvalidNotes();
}

void ChartController::CheckUnhitNotes(std::chrono::microseconds aNewPlayhead)
{
	const ChartTrack* track = GetTrack();
	if (track == nullptr)
		return;

	const std::uint8_t laneCount = ChartTrackTypeLaneCount[static_cast<int>(GetTrackType())];
	
	for (std::uint8_t lane = 0; lane < laneCount; ++lane)
	{
		const std::chrono::microseconds lastHitCheck = myLastLaneHitCheck.contains(lane) ? myLastLaneHitCheck.at(lane) : std::chrono::microseconds(0);

		const ChartNoteRange* nextNote = track->GetNextNote(GetTrackDifficulty(), lane, lastHitCheck);
		if (!nextNote)
			continue;

		if ((nextNote->Start + NOTE_LOWEST_ACCURACY) < aNewPlayhead)
		{
			myScoring.MissedValidNotes(1);
			myLastLaneHitCheck[lane] = aNewPlayhead;
		}
	}
}

std::optional<float> ChartController::CalculateNoteAccuracy(std::chrono::microseconds aPerfectTimepoint, std::chrono::microseconds aHitTimepoint) const
{
	const std::chrono::microseconds accuracyMilliseconds = aHitTimepoint - aPerfectTimepoint;

	const float accuracy = 1 - (static_cast<float>(accuracyMilliseconds.count()) / static_cast<float>(NOTE_LOWEST_ACCURACY.count()));

	if (accuracy < 0.f || accuracy > 1.f)
		return { };

	return accuracy;
}

void ChartController::UpdateActiveSustains(const std::chrono::microseconds& aPreviousPlayhead, const std::chrono::microseconds& aNewPlayhead)
{	
	for (auto sustainIt : myActiveSustains)
		myHitNoteRanges[sustainIt] = aNewPlayhead;

	while (true)
	{
		const auto activeSustainInLane = std::find_if(
			myActiveSustains.begin(),
			myActiveSustains.end(),
			[&aNewPlayhead](const ChartNoteRange* aNote) { return aNote->End < aNewPlayhead; }
		);

		if (activeSustainInLane == myActiveSustains.end())
			break;

		myScoring.SustainProgress(*myCurrentChart, aPreviousPlayhead, (*activeSustainInLane)->End);
		myActiveSustains.erase(activeSustainInLane);
	}

	for (auto sustainIt : myActiveSustains)
		myLaneLastStrum[sustainIt->Lane] = aNewPlayhead;

	myScoring.SustainProgress(*myCurrentChart, aPreviousPlayhead, aNewPlayhead, myActiveSustains.size());
}

bool ChartController::IsSustainActive(const ChartNoteRange& aNoteRange) const
{
	const auto activeSustain = std::find_if(
		myActiveSustains.begin(),
		myActiveSustains.end(),
		[&aNoteRange](const ChartNoteRange* aNote)
		{
			return
				aNoteRange.Lane == aNote->Lane
				&& aNoteRange.Start == aNote->Start
				&& aNoteRange.End == aNote->End
				;
		}
	);

	return activeSustain != myActiveSustains.end();
}

bool ChartController::IsNoteMissed(const ChartNoteRange& aNoteRange) const
{
	if (myLastPlayhead < aNoteRange.Start)
		return false;

	return !CalculateNoteAccuracy(aNoteRange.Start, myLastPlayhead).has_value();
}

void ChartController::SetTrackType(ChartTrackType aType)
{
	myTrackType = aType;
}

void ChartController::SetTrackDifficulty(ChartTrackDifficulty aDifficulty)
{
	myTrackDifficulty = aDifficulty;
}

void ChartController::ClearLanes()
{
	myLaneStates.fill(false);
}

const ChartTrack* ChartController::GetTrack() const
{
	if (!myCurrentChart)
		return nullptr;

	const auto trackIterator = myCurrentChart->GetTracks().find(GetTrackType());
	if (trackIterator == myCurrentChart->GetTracks().end())
		return nullptr;

	return trackIterator->second.get();
}

void ChartController::SetLane(std::uint8_t aLane, bool aState)
{
	if (aState && !myLaneStates.at(aLane))
		CheckTapHit(aLane);

	myLaneStates.at(aLane) = aState;

	if (!aState)
	{
		const auto activeSustainInLane = std::find_if(
			myActiveSustains.begin(),
			myActiveSustains.end(),
			[&aLane](const ChartNoteRange* aNote) { return aNote->Lane == aLane; }
		);

		if (activeSustainInLane != myActiveSustains.end())
			myActiveSustains.erase(activeSustainInLane);
	}
}

void ChartController::Strum()
{
	myLastStrum = myLastPlayhead;

	for (std::size_t i = 0; i < myLaneStates.size(); ++i)
	{
		if (!myLaneStates[i])
			continue;

		myLaneLastStrum[i] = myLastPlayhead;
	}

	CheckStrumHits();
}
