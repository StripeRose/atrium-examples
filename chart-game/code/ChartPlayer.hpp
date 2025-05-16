// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"
#include "ChartData.hpp"

#include <array>
#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

class ChartPlayer
{
public:
	enum class State { Playing, Paused, Seeking, Stopped };

public:
	template <typename T>
	T* AddController();

	const ChartData* GetChartData() { return myActiveChart.transform([](ActiveChart& chart) { return &chart.Data; }).value_or(nullptr); }

	const std::vector<std::unique_ptr<ChartController>>& GetControllers() const { return myControllers; }

	std::chrono::microseconds GetPlayhead() const { return myPlayhead; }

	State GetState() const;

	void LoadChart(const std::filesystem::path& aSong);

	void Pause();

	void Play();

	void RemoveController(ChartController& aController);

	void Seek(std::chrono::microseconds aPlayTime);

	void Stop();

	void Update();

private:
	enum class InternalState
	{
		Playing, Paused, SeekingPlaying, SeekingPaused, Stopped
	};

	struct ActiveChart
	{
		ChartInfo Info;
		ChartData Data;
	};

	std::optional<ActiveChart> myActiveChart;

	InternalState myState = InternalState::Stopped;
	std::chrono::high_resolution_clock::time_point myStartTime;
	std::chrono::high_resolution_clock::time_point myLastUpdateTime;

	std::chrono::microseconds myPlayhead{ 0 };

	std::vector<std::unique_ptr<ChartController>> myControllers;
};

template<typename T>
inline T* ChartPlayer::AddController()
{
	std::unique_ptr<ChartController>& newController = myControllers.emplace_back(std::make_unique<T>());
	if (myActiveChart)
		newController->HandleChartChange(myActiveChart.value().Data);
	return static_cast<T*>(newController.get());
}
