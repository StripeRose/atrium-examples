// Filter "Chart/Playback"
#include "ChartController.hpp"

#include "ChartTestWindow.hpp"

#include "Editor_GUI.hpp"

ChartController::ChartController()
	: myTrackDifficulty(ChartTrackDifficulty::Hard)
	, myTrackType(ChartTrackType::LeadGuitar)
{
	myLaneStates.fill(false);
}

void ChartController::HandleChartChange(const ChartData& /*aData*/)
{
	myLastStrum.reset();
}

void ChartController::HandlePlayheadStep(const std::chrono::microseconds& /*aPrevious*/, const std::chrono::microseconds& aNew)
{
	if (myLastStrum && myLastStrum.value() > aNew)
		myLastStrum.reset();

	myLastPlayhead = aNew;
}

#if IS_IMGUI_ENABLED
void ChartController::ImGui(ChartTestWindow& aTestWindow)
{
	aTestWindow.ImGui_Lanes(*this);
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
	ImGui::Text("Accuracy: %.0f%%", myScoring.GetAccuracy() * 100.f);

	if (ImGui::SmallButton("Reset stats"))
		myScoring.Reset();
}
#endif

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
	myLaneStates = {};
}

void ChartController::SetLane(std::uint8_t aLane, bool aState)
{
	myLaneStates.at(aLane) = aState;
}

void ChartController::Strum()
{
	myLastStrum = myLastPlayhead;
}
