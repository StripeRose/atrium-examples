// Filter "Chart"
#pragma once

#include "ChartData.hpp"
#include "ChartPlayer.hpp"

#include "Core_Math.hpp"

#include <filesystem>
#include <map>

class ChartTestWindow
{
public:
	ChartTestWindow() = delete;
	ChartTestWindow(ChartPlayer& aPlayer);

	void ImGui();

private:
#if IS_IMGUI_ENABLED
	void ImGui_ChartList();

	void ImGui_Controllers();

	void ImGui_Player();
	void ImGui_Player_PlayControls();

	void ImGui_Track(ChartTrack& aTrack);
	void ImGui_Track(ChartGuitarTrack& aTrack, Atrium::Vector2 aPoint, Atrium::Vector2 aSize);
	void ImGui_Track_HitWindow(Atrium::Vector2 aPoint, Atrium::Vector2 aSize);
	void ImGui_Track_Beats(Atrium::Vector2 aPoint, Atrium::Vector2 aSize);
	void ImGui_Track_TimeSignatures();

	void ImGui_Track_Note(const ChartNoteRange& aNote, float aStartX, float anEndX, float aLaneY);
	void ImGui_Track_OpenNote(bool isHOPO, float aNoteStart, float aNoteEnd, float aTopLane, float aBottomLane);

	float ImGui_TimeToTrackPosition(float aTrackWidth, std::chrono::microseconds aTime) const;
#endif

	void RefreshSongList();
	void LoadSong(const std::filesystem::path& aSong);

	struct TrackSettings
	{
		ChartTrackDifficulty Difficulty = ChartTrackDifficulty::Hard;
		bool ShowOpen = true;
	};

	std::filesystem::path mySongsDirectory;
	std::map<std::filesystem::path, std::unique_ptr<ChartInfo>> myChartInfos;

	std::string myCurrentSong;

	ChartData myChartData;
	ChartPlayer& myChartPlayer;
	std::map<ChartTrackType, TrackSettings> myTrackSettings;
	std::chrono::microseconds myLookAhead;
};
