// Filter "Chart"
#pragma once

#include "ChartTrack.hpp"
#include "ChartCommonStructures.hpp"

#include <rose-common/fileformat/Ini.hpp>

#include <filesystem>
#include <map>

namespace FileName_Audio
{
	// Preview audio to play on the song select menu
	static constexpr const char* Preview = "preview";
	// Background audio
	static constexpr const char* Song = "song";

	static constexpr const char* LeadGuitar = "guitar";
	static constexpr const char* RhythmGuitar = "rhythm";

	static constexpr const char* BassGuitar = "bass";

	static constexpr const char* KeysAudio = "keys";

	// Single drums stem, should be ignored if individual stems exist.
	static constexpr const char* Drums = "drums";
	static constexpr const char* Drums_Kick = "drums_1";
	static constexpr const char* Drums_Snare = "drums_2";
	// Includes cymbals if individual stem doesn't exist for it.
	static constexpr const char* Drums_Toms = "drums_3";
	static constexpr const char* Drums_Cymbal = "drums_4";

	// Vocals as single stem. Should be ignored if individual stems exist.
	static constexpr const char* Vocals = "vocals";
	static constexpr const char* Vocals_Main = "vocals_1";
	static constexpr const char* Vocals_Secondary = "vocals_2";

	// Explicit vocals as single stem. Should be ignored if individual stems exist.
	static constexpr const char* Vocals_Explicit = "vocals_explicit";
	static constexpr const char* Vocals_Explicit_Main = "vocals_explicit_1";
	static constexpr const char* Vocals_Explicit_Secondary = "vocals_explicit_2";

	static constexpr const char* Crowd = "crowd";
}

class ChartInfo
{
public:
	static constexpr int MaxDifficulty = 6;

public:
	struct SongInfo
	{
		std::string Title;
		std::string Artist;
		std::string Album;
		std::string Genre;
		unsigned int Year = 0;
		std::chrono::milliseconds SongLength{ 0 };
	};

public:
	void Load(const std::filesystem::path& aSongIni);

	const SongInfo& GetSongInfo() const { return mySongInfo; }

	const int GetDifficulty(ChartTrackType anInstrument) const { return myDifficulties.at(anInstrument); }

private:
	RoseCommon::Ini myIni;

	SongInfo mySongInfo;
	std::map<ChartTrackType, int> myDifficulties;
};

class ChartTrack;

class ChartData
{
public:
	struct TempoSection
	{
		std::uint32_t TickStart;
		std::chrono::microseconds TimeStart;
		std::chrono::microseconds TimePerBeat;
	};

	struct TimeSignature
	{
		std::uint8_t Numerator = 0;
		std::uint8_t Denominator = 0;
		std::uint8_t Clock = 0;
		std::uint8_t Base = 0;
	};

public:
	std::chrono::microseconds GetBeatLengthAt(std::chrono::microseconds aTime) const;

	float GetBPMAt(std::chrono::microseconds aTime) const;

	std::chrono::microseconds GetDuration() const;

	const std::string GetSectionNameAt(std::chrono::microseconds aTime) const;

	const std::vector<std::pair<std::chrono::microseconds, std::string>>& GetSectionNames() const { return mySections; }

	const std::vector<TempoSection>& GetTempoSections() const { return myTempos; }

	const TimeSignature GetTimeSignatureAt(std::chrono::microseconds aTime) const;

	const std::vector<std::pair<std::chrono::microseconds, TimeSignature>>& GetTimeSignatures() const { return myTimeSignatures; }

	const std::map<ChartTrackType, std::unique_ptr<ChartTrack>>& GetTracks() const { return myTracks; }

	void LoadMidi(const std::filesystem::path& aMidi);

private:
	std::map<ChartTrackType, std::unique_ptr<ChartTrack>> myTracks;
	std::vector<TempoSection> myTempos;
	std::vector<std::pair<std::chrono::microseconds, TimeSignature>> myTimeSignatures;
	std::vector<std::pair<std::chrono::microseconds, std::string>> mySections;
};
