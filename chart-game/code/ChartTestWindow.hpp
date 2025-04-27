// Filter "Chart"
#pragma once

#include "ChartData.hpp"

#include "Atrium_Math.hpp"

#include <filesystem>
#include <map>
#include <span>

struct ImGui_ChartDrawParameters
{
	Atrium::Vector2 Point;
	Atrium::Vector2 Size;
	std::function<float(std::chrono::microseconds aTime)> TimeToPoint;
};

class ChartController;
class ChartPlayer;
class ChartRenderer;
class ChartTestWindow
{
public:
	ChartTestWindow(ChartPlayer& aPlayer, ChartRenderer& aRenderer);

	void ImGui();

	void ImGui_GuitarControlState(ChartController& aController);

	#if IS_IMGUI_ENABLED
	void ImGui_DrawChart(Atrium::Vector2 aSize, std::function<void(const ImGui_ChartDrawParameters&)> aDrawFunction);
	void ImGui_DrawChart_Lanes(const ImGui_ChartDrawParameters& someParameters, ChartTrackType aTrackType);
	void ImGui_DrawChart_Note(const ImGui_ChartDrawParameters& someParameters, const ChartNoteRange& aNote, ChartTrackType aTrackType, bool aShowOpens = true);
	#endif

private:
#if IS_IMGUI_ENABLED
	void ImGui_ChartList();
	void ImGui_ChartList_Path();

	void ImGui_Controllers();

	void ImGui_Player_PlayControls();
	void ImGui_Player_LookAheadControl();

	void ImGui_Tracks();
	void ImGui_Track(ChartTrack& aTrack);
	void ImGui_Track(const ImGui_ChartDrawParameters& someParameters, ChartGuitarTrack& aTrack);
	void ImGui_DrawChart_HitWindow(const ImGui_ChartDrawParameters& someParameters);
	void ImGui_DrawChart_Beats(const ImGui_ChartDrawParameters& someParameters);
	void ImGui_Track_TimeSignatures();

	void ImGui_GuitarNote(const ChartNoteRange& aNote, float aStartX, float anEndX, float aLaneY);
	void ImGui_GuitarNote_Open(bool isHOPO, float aNoteStart, float aNoteEnd, float aTopLane, float aBottomLane);
#endif

	void RefreshSongList();
	void LoadSong(const std::filesystem::path& aSong);

	struct TrackSettings
	{
		ChartTrackDifficulty Difficulty = ChartTrackDifficulty::Hard;
		bool ShowOpen = true;
	};

	std::filesystem::path mySongsDirectory;
	std::array<char, 512> mySongsDirectoryBuffer;
	std::map<std::filesystem::path, std::unique_ptr<ChartInfo>> myChartInfos;

	std::string myCurrentSong;

	ChartData myChartData;
	ChartPlayer& myChartPlayer;
	ChartRenderer& myChartRenderer;
	std::map<ChartTrackType, TrackSettings> myTrackSettings;
	std::chrono::microseconds myLookAhead;
};
