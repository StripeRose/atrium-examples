// Filter "Chart/Scoring"

#include "ChartScoring.hpp"

#include "ChartData.hpp"

#include <Core_Math.hpp>

ChartScoring::ChartScoring()
{
	Reset();
}

float ChartScoring::GetAccuracy() const
{
	if (myTotalNotes == 0)
		return 1.0f;

	return static_cast<float>(myNotesHit) / static_cast<float>(myTotalNotes);
}

void ChartScoring::HitInvalidNotes()
{
	myStreak = 0;
	myMultiplier = 1;

	// Does not count to total.
}

void ChartScoring::HitValidNotes(unsigned int aCount)
{
	for (unsigned int i = 0; i < aCount; ++i)
	{
		if ((myStreak % StreakMultiplierInterval) == 0)
			myMultiplier = Atrium::Math::Min(myMultiplier + 1, MaximumMultiplier);

		myScore += myMultiplier * BaseScore;
	}

	myNotesHit += aCount;
	myTotalNotes += aCount;

	myStreak += aCount;
	myMaximumStreak = Atrium::Math::Max(myStreak, myMaximumStreak);
}

void ChartScoring::MissedValidNotes(unsigned int aCount)
{
	myStreak = 0;
	myMultiplier = 1;

	myTotalNotes += aCount;
}

void ChartScoring::SustainProgress(const ChartData& aChartData, std::chrono::microseconds aStart, std::chrono::microseconds anEnd)
{
	aChartData;

	// Todo: Implement proper scoring based on the beats.
	// Currently implemented to just give points based on seconds of sustain progress.

	const std::chrono::microseconds sustainProgress = anEnd - aStart;
	static constexpr std::chrono::microseconds microsecondsPerSecond = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1));

	myScoreFraction += (static_cast<float>(sustainProgress.count()) / static_cast<float>(microsecondsPerSecond.count())) * ChartScoring::SustainScorePerBeat;
	const unsigned int wholeScore = Atrium::Math::TruncateTo<unsigned int>(myScoreFraction);

	myScoreFraction -= wholeScore;
	myScore += wholeScore;
}

void ChartScoring::Reset()
{
	myScore = 0;
	myStreak = 0;
	myMultiplier = 1;
	myMaximumStreak = 0;

	myNotesHit = 0;
	myTotalNotes = 0;

	myScoreFraction = 0.f;
}
