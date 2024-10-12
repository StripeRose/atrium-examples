// Filter "Chart/Playback"
#include "ChartController.hpp"

#include "ChartData.hpp"
#include "ChartTestWindow.hpp"

#include "Core_Diagnostics.hpp"
#include "Core_InputSource.hpp"

#include "Editor_GUI.hpp"

ChartController::ChartController()
	: myTrackDifficulty(ChartTrackDifficulty::Hard)
	, myTrackType(ChartTrackType::LeadGuitar)
{
	myLaneStates.fill(false);
}

#if IS_IMGUI_ENABLED
void ChartController::ImGui(ChartTestWindow& aTestWindow)
{
	aTestWindow.ImGui_Lanes(myTrackType, myLaneStates);

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

		const bool activateNote = (myNextNotes[i].second < std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(30)));
		myLaneStates[i] = activateNote;
	}
}
#endif

void ChartHumanController::HandleInput(const Atrium::Core::InputEvent& anInputEvent)
{
	using namespace Atrium::Core;

	std::optional<int> lane;

	switch (anInputEvent.Source)
	{
		case InputSourceId::Keyboard::Alpha1:
			lane = 0;
			break;
		case InputSourceId::Keyboard::Alpha2:
			lane = 1;
			break;
		case InputSourceId::Keyboard::Alpha3:
			lane = 2;
			break;
		case InputSourceId::Keyboard::Alpha4:
			lane = 3;
			break;
		case InputSourceId::Keyboard::Alpha5:
			lane = 4;
			break;
	}

	if (lane.has_value())
		myLaneStates[lane.value()] = anInputEvent.Value > 0.5f;
}
