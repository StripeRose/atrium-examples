// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

#include <array>
#include <chrono>
#include <memory>
#include <vector>

class ChartData;

class ChartPlayer
{
public:
	enum class State { Playing, Paused, Seeking, Stopped };

public:
	template <typename T>
	T* AddController();

	const ChartData* GetChartData() { return myChartData; }

	const std::vector<std::unique_ptr<ChartController>>& GetControllers() const { return myControllers; }

	std::chrono::microseconds GetPlayhead() const { return myPlayhead; }

	State GetState() const;

	void Pause();

	void Play();

	void RemoveController(ChartController& aController);

	void Seek(std::chrono::microseconds aPlayTime);

	void SetChartData(const ChartData& aData);

	void Stop();

	void Update();

private:
	enum class InternalState
	{
		Playing, Paused, SeekingPlaying, SeekingPaused, Stopped
	};

	const ChartData* myChartData = nullptr;

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
	if (myChartData)
		newController->HandleChartChange(*myChartData);
	return static_cast<T*>(newController.get());
}
