// Filter "Chart/Playback"
#include "ChartPlayer.hpp"

#include "ChartData.hpp"
#include "Atrium_Diagnostics.hpp"

ChartPlayer::State ChartPlayer::GetState() const
{
	switch (myState)
	{
	case InternalState::Playing:
		return State::Playing;
	case InternalState::Paused:
		return State::Paused;
	case InternalState::SeekingPaused:
	case InternalState::SeekingPlaying:
		return State::Seeking;
	case InternalState::Stopped:
		return State::Stopped;
	}

	return State::Stopped;
}

void ChartPlayer::LoadChart(const std::filesystem::path& aSong)
{
	myActiveChart = ActiveChart();
	ActiveChart& activeChart = myActiveChart.value();

	activeChart.Info.Load(aSong);
	activeChart.Data.LoadMidi(aSong.parent_path() / "notes.mid");

	// Todo: Set up all audio clips.

	for (const std::unique_ptr<ChartController>& controller : myControllers)
		controller->HandleChartChange(myActiveChart.value().Data);
}

void ChartPlayer::Pause()
{
	Atrium::Debug::Log("Chart pause.");
	myState = InternalState::Paused;
}

void ChartPlayer::Play()
{
	if (!myActiveChart)
		return;

	Atrium::Debug::Log("Chart play.");
	switch (myState)
	{
	case InternalState::Playing:
		return;
	case InternalState::Paused:
		myState = InternalState::Playing;
		return;
	case InternalState::SeekingPaused:
	case InternalState::SeekingPlaying:
		return;
	case InternalState::Stopped:
		myStartTime = std::chrono::high_resolution_clock::now();
		myLastUpdateTime = myStartTime;
		myState = InternalState::Playing;
		return;
	}
}

void ChartPlayer::RemoveController(ChartController& aController)
{
	const auto it = std::find_if(
		myControllers.begin(),
		myControllers.end(),
		[&aController](const std::unique_ptr<ChartController>& listController)
		{ return listController.get() == &aController; }
	);

	myControllers.erase(it);
}

void ChartPlayer::Seek(std::chrono::microseconds aPlayTime)
{
	switch (myState)
	{
	case InternalState::Playing:
		myState = InternalState::SeekingPlaying;
		break;
	case InternalState::Paused:
		myState = InternalState::SeekingPaused;
		break;
	default:
		return;
	}

	for (const std::unique_ptr<ChartController>& controller : myControllers)
		controller->HandlePlayheadStep(myPlayhead, aPlayTime);

	myPlayhead = aPlayTime;
}

void ChartPlayer::Stop()
{
	Atrium::Debug::Log("Chart stop.");
	myState = InternalState::Stopped;
	myPlayhead = std::chrono::microseconds(0);
}

void ChartPlayer::Update()
{
	const auto newUpdateTime = std::chrono::high_resolution_clock::now();
	const auto lastUpdatePoint = std::chrono::duration_cast<std::chrono::microseconds>(myLastUpdateTime - myStartTime);
	const auto thisUpdatePoint = std::chrono::duration_cast<std::chrono::microseconds>(newUpdateTime - myStartTime);

	myLastUpdateTime = newUpdateTime;

	switch (myState)
	{
	case InternalState::Playing:
		for (const std::unique_ptr<ChartController>& controller : myControllers)
			controller->HandlePlayheadStep(lastUpdatePoint, thisUpdatePoint);
		myPlayhead = thisUpdatePoint;
		break;
	case InternalState::Paused:
		myStartTime += (thisUpdatePoint - lastUpdatePoint);
		break;
	case InternalState::SeekingPlaying:
	case InternalState::SeekingPaused:
		myStartTime = (newUpdateTime - myPlayhead);
		myState = (myState == InternalState::SeekingPlaying ? InternalState::Playing : InternalState::Paused);
		break;
	case InternalState::Stopped:
		break;
	}
}
