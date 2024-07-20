// Filter "Chart/Playback"
#include "stdafx.hpp"
#include "ChartController.hpp"

#include "ChartData.hpp"

#include "Common_Diagnostics.hpp"

#include "Editor_GUI.hpp"

ChartController::ChartController()
	: myTrackDifficulty(ChartTrackDifficulty::Hard)
	, myTrackType(ChartTrackType::LeadGuitar)
{

}

#if IS_IMGUI_ENABLED
void ChartController::ImGui()
{
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
#endif

void ChartController::SetTrackType(ChartTrackType aType)
{
	myTrackType = aType;
}

void ChartController::SetTrackDifficulty(ChartTrackDifficulty aDifficulty)
{
	myTrackDifficulty = aDifficulty;
}

#if IS_IMGUI_ENABLED
void ChartAIController::ImGui()
{
	ImGui::TextUnformatted("AI player");
	ChartController::ImGui();

	ImGui::TextUnformatted("Next note in:");

	auto nextNote =
		[](const char* aLabel, std::chrono::microseconds aNext)
		{
			ImGui::Text("%s: %f", aLabel, static_cast<float>(aNext.count()) / 1000000.f);

			bool v = (aNext < std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(30)));
			ImGui::SameLine();
			ImGui::BeginDisabled();
			ImGui::Checkbox("", &v);
			ImGui::EndDisabled();
		};

	nextNote("GRN", myNextNotes[0].second);
	nextNote("RED", myNextNotes[1].second);
	nextNote("YLW", myNextNotes[2].second);
	nextNote("BLU", myNextNotes[3].second);
	nextNote("ORG", myNextNotes[4].second);
}

void ChartAIController::OnChartChange(const ChartData& aData)
{
	myCurrentChart = &aData;
}

void ChartAIController::OnPlayheadStep(const std::chrono::microseconds&, const std::chrono::microseconds& aNew)
{
	myNextNotes.clear();
	for (std::uint8_t i = 0; i < 5; ++i)
	{
		const ChartNoteRange* next = myCurrentChart->GetTracks().at(GetTrackType())->GetNextNote(GetTrackDifficulty(), i, aNew);
		if (next)
			myNextNotes[i] = { next, next->Start - aNew };
		else
			myNextNotes[i] = { nullptr, std::chrono::microseconds(-1) };
	}
}
#endif
