// Filter "Chart/Scoring"

#pragma once

#include <chrono>

/*
	Source
	https://gaming.stackexchange.com/questions/7197/what-is-the-scoring-algorithm-for-rock-band-2
*/

class ChartData;
class ChartScoring
{
public:

	//--------------------------------------------------
	// * Static constants
	//--------------------------------------------------
	#pragma region Static constants

	static constexpr unsigned int BaseScore = 25;
	static constexpr unsigned int StreakMultiplierInterval = 10;
	static constexpr unsigned int MaximumMultiplier = 4;
	static constexpr unsigned int SustainScorePerBeat = 12;

	#pragma endregion

	//--------------------------------------------------
	// * Construction
	//--------------------------------------------------
	#pragma region Construction

	ChartScoring();

	#pragma endregion

	//--------------------------------------------------
	// * Properties
	//--------------------------------------------------
	#pragma region Properties

	float GetAccuracy() const;
	unsigned int GetHitCount() const { return myNotesHit; }
	unsigned int GetNoteCount() const { return myTotalNotes; }

	unsigned int GetMaximumStreak() const { return myMaximumStreak; }
	unsigned int GetMultiplier() const { return myMultiplier; }
	unsigned int GetScore() const { return myScore; }
	unsigned int GetStreak() const { return myStreak; }

	#pragma endregion

	//--------------------------------------------------
	// * Methods
	//--------------------------------------------------
	#pragma region Methods

	void HitInvalidNotes();

	void HitValidNotes(unsigned int aCount);

	void MissedValidNotes(unsigned int aCount);

	void SustainProgress(const ChartData& aChartData, std::chrono::microseconds aStart, std::chrono::microseconds anEnd, std::size_t aSustainCount = 1);

	void Reset();

	#pragma endregion

private:
	unsigned int myMaximumStreak;
	unsigned int myMultiplier;
	unsigned int myScore;
	unsigned int myStreak;

	unsigned int myNotesHit;
	unsigned int myTotalNotes;

	float myScoreFraction;
};
