// Filter "Chart/Playback"
#include "ChartAIController.hpp"

#include "ChartData.hpp"

#include "Editor_GUI.hpp"

void ChartAIController::HandleChartChange(const ChartData& aData)
{
	myCurrentChart = &aData;
}

void ChartAIController::HandlePlayheadStep(const std::chrono::microseconds&, const std::chrono::microseconds& aNew)
{
	myClosestNotes.clear();
	for (std::uint8_t i = 0; i < 5; ++i)
	{
		const ChartNoteRange* next = myCurrentChart->GetTracks().at(GetTrackType())->GetClosestNote(GetTrackDifficulty(), i, aNew);
		if (next)
			myClosestNotes[i] = { next, next->Start - aNew };
		else
			myClosestNotes[i] = { nullptr, std::chrono::microseconds(-1) };

		const bool activateNote = (Atrium::Math::Abs(myClosestNotes[i].second) < std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(30)));
		SetLane(i, activateNote);
	}
}

#if IS_IMGUI_ENABLED
void ChartAIController::ImGui(ChartTestWindow& aTestWindow)
{
	ImGui::TextUnformatted("AI player");
	ChartController::ImGui(aTestWindow);

	ImGui::TextUnformatted("Next note in:");

	auto nextNote =
		[](const char* aLabel, std::chrono::microseconds aNext)
		{
			ImGui::Text("%s: %f", aLabel, static_cast<float>(aNext.count()) / 1000000.f);
		};

	nextNote("GRN", myClosestNotes[0].second);
	nextNote("RED", myClosestNotes[1].second);
	nextNote("YLW", myClosestNotes[2].second);
	nextNote("BLU", myClosestNotes[3].second);
	nextNote("ORG", myClosestNotes[4].second);
}
#endif
