#pragma once

#include <chrono>
#include <filesystem>
#include <span>
#include <vector>

#include <rose-common/EventSlot.hpp>

class MidiDecoder
{
public:
	enum class FormatType : std::uint16_t
	{
		SingleTrack = 0,
		MultipleSimultaneous = 1,
		Sequential = 2
	};

public:
	static void DecomposeNoteNumber(const std::uint8_t aNoteIndex, std::uint8_t& outOctave, std::uint8_t& outNote);
	static std::string NoteNumberToString(const std::uint8_t aNoteIndex);

	void ProcessFile(const std::filesystem::path& aPath, FormatType& outFormatType, std::uint16_t& outTicksPerQuarterNote);

public:
	RoseCommon::EventSlot<> OnNewTrack;
	RoseCommon::EventSlot<std::uint32_t> OnTrackEnd;

	// Note event.
	// Time delta, Channel, Note, Velocity(0 for off)
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t, std::uint8_t> OnNote;
	// Note pressure event
	// Time delta, Channel, Note, Pressure
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t, std::uint8_t> OnNotePressure;

	// Sound off event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnSoundOff;
	// Reset all controllers event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnResetAllControllers;
	// Local control event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnLocalControl;
	// All notes off event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnAllNotesOff;
	// Omni off event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnOmniOff;
	// Omni on event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnOmniOn;
	// Mono on, Poly off event
	// Time delta, Channel, Value
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnMonoOn;
	// Poly on, Mono off event
	// Time delta, Channel
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t> OnPolyOn;
	// Program change event
	// Time delta, Channel, Program num
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnProgramChange;
	// Channel pressure event
	// Time delta, Channel, Pressure
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnChannelPressure;
	// Pitch bend event
	// Time delta, Channel, Pressure
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnPitchBend;
	// System Exclusive event
	// Time delta, Data
	RoseCommon::EventSlot<std::uint32_t, const std::span<std::uint8_t>&> OnSysEx;

	RoseCommon::EventSlot<std::uint32_t, std::uint16_t> OnSequenceNumber;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnText;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnCopyrightNotice;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnTrackName;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnInstrumentName;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnLyric;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnMarker;
	RoseCommon::EventSlot<std::uint32_t, const std::string&> OnCuePoint;

	RoseCommon::EventSlot<std::uint32_t, std::uint32_t> OnSetTempo;
	// Time delta, Numerator, Denominator, Metronome Clock, Number of notated 32nd notes per MIDI quarter note.
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t> OnTimeSignature;

	// Time delta, Sharp/Flat, Major/Minor
	RoseCommon::EventSlot<std::uint32_t, std::uint8_t, std::uint8_t> OnKeySignature;

private:
	void ReadHeaderChunk(std::istream& aStream, FormatType& outFormatType, std::uint16_t& outTicksPerQuarterNote);

	void ProcessTrackChunk(std::istream& aStream);
	
	std::uint32_t ProcessEvent(std::istream& aStream, std::uint32_t& aTicksProgress, std::uint8_t& aRunningEventStatus);
	std::uint32_t ProcessEventData(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t anEventStatus);

	std::uint32_t ReadNoteEvent(std::istream& aStream, std::uint32_t aTickCount, bool anIsOn, std::uint8_t aChannelIndex);
	std::uint32_t ReadPolyphonicKeyPressureEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex);
	std::uint32_t ReadControlChangeEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex);
	std::uint32_t ReadProgramChangeEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex);
	std::uint32_t ReadChannelPressureEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex);
	std::uint32_t ReadPitchBendEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex);
	std::uint32_t ReadSysExEvent(std::istream& aStream, std::uint32_t aTickCount, bool aHasEnd);
	std::uint32_t ReadMetaEvent(std::istream& aStream, std::uint32_t aTickCount);
};
