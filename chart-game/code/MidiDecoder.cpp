#include "MidiDecoder.hpp"

#include <Core_Diagnostics.hpp>

#include <fstream>

using namespace Atrium;

template <typename T>
std::uint8_t ReadFixed(std::istream& aStream, T& anOut, unsigned int anOffset = 0)
{
	aStream.read(reinterpret_cast<char*>(&anOut), sizeof(T) - anOffset);
	if constexpr (sizeof(T) != 1)
		std::reverse(reinterpret_cast<char*>(&anOut), reinterpret_cast<char*>(&anOut) + sizeof(T) - anOffset);
	return sizeof(T);
}

[[nodiscard]]
std::uint8_t ReadVariable(std::istream& aStream, std::uint32_t& anOut)
{
	anOut = 0;
	std::uint8_t readBytes = 0;

	std::uint8_t c = 0;
	readBytes += ReadFixed(aStream, c);
	anOut |= c;

	if ((anOut & 0x80) != 0)
	{
		anOut &= 0x7F;
		do
		{
			readBytes += ReadFixed(aStream, c);
			anOut = (anOut << 7) + (c & 0x7F);
		} while ((c & 0x80) != 0);
	}

	return readBytes;
}

std::string ReadText(std::istream& aStream, std::uint32_t aLength)
{
	std::unique_ptr<char> textData(new char[aLength + 1]);
	aStream.read(textData.get(), aLength);
	textData.get()[aLength] = '\0';
	return textData.get();
}

void MidiDecoder::DecomposeNoteNumber(const std::uint8_t aNoteIndex, std::uint8_t& outOctave, std::uint8_t& outNote)
{
	outNote = (aNoteIndex % 12);
	outOctave = (aNoteIndex - outNote) / 12;
}

std::string MidiDecoder::NoteNumberToString(const std::uint8_t aNoteIndex)
{
	const char* notes[] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B", "D" };
	const std::uint8_t note = (aNoteIndex % 12);
	const std::uint8_t octave = (aNoteIndex - note) / 12;
	return std::string(notes[note]) + " " + (octave == 0 ? "-" : std::to_string(octave - 1));
}

void MidiDecoder::ProcessFile(const std::filesystem::path& aPath, FormatType& outFormatType, std::uint16_t& outTicksPerQuarterNote)
{
	std::ifstream fileStream;
	fileStream.open(aPath, std::ios::in | std::ios::binary);
	while (!fileStream.eof())
	{
		char chunkMarker[5];
		fileStream >> chunkMarker;
		chunkMarker[4] = '\0';

		if (std::strcmp(chunkMarker, "MThd") == 0)
			ReadHeaderChunk(fileStream, outFormatType, outTicksPerQuarterNote);
		else if (std::strcmp(chunkMarker, "MTrk") == 0)
			ProcessTrackChunk(fileStream);
	}

	fileStream.close();
}

void MidiDecoder::ReadHeaderChunk(std::istream& aStream, FormatType& outFormatType, std::uint16_t& outTicksPerQuarterNote)
{
	std::uint32_t chunkSize = 0;
	ReadFixed(aStream, chunkSize);

	ReadFixed(aStream, outFormatType);

	// We're reading in all the tracks, so throw out the metadata track count here.
	std::uint16_t trackCount = 0;
	ReadFixed(aStream, trackCount);

	{
		std::uint16_t packedTimeCode = 0;
		ReadFixed(aStream, packedTimeCode);

		if ((packedTimeCode & 0x8000) == 0)
		{
			// Ticks per quarter note
			outTicksPerQuarterNote = (packedTimeCode & 0x7FFF);
		}
		else
		{
			// SMPTE frame
			/*const std::uint8_t resolution = (packedTimeCode & 0x0F);
			const std::int8_t smpteFormat = static_cast<std::int8_t>((packedTimeCode >> 8) & 0x0F);*/
			throw std::runtime_error("SMPTE conversion needs implementing.");
		}
	}

	// We read 6 fixed bytes, skip any remaining ones in this chunk.
	aStream.seekg(chunkSize - 0x06, std::ios::cur);
}

void MidiDecoder::ProcessTrackChunk(std::istream& aStream)
{
	OnNewTrack.Invoke();

	std::uint32_t chunkSize = 0;
	ReadFixed(aStream, chunkSize);

	std::uint32_t readBytes = 0;
	std::uint8_t runningStatus = 0;
	std::uint32_t tickCount = 0;
	while (readBytes < chunkSize)
		readBytes += ProcessEvent(aStream, tickCount, runningStatus);

	aStream.seekg(chunkSize - readBytes, std::ios::cur);
}

std::uint32_t MidiDecoder::ProcessEvent(std::istream& aStream, std::uint32_t& aTickCount, std::uint8_t& aRunningEventStatus)
{
	std::uint32_t readBytes = 0;
	std::uint32_t deltaTicks = 0;
	readBytes += ReadVariable(aStream, deltaTicks);
	aTickCount += deltaTicks;

	if (aStream.peek() >= 0x80)
		readBytes += ReadFixed(aStream, aRunningEventStatus);

	return ProcessEventData(aStream, aTickCount, aRunningEventStatus) + readBytes;
}

std::uint32_t MidiDecoder::ProcessEventData(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t anEventStatus)
{
	constexpr std::uint8_t NoteOff = 0x80;
	constexpr std::uint8_t NoteOn = 0x90;
	constexpr std::uint8_t PolyKeyPressure = 0xA0;
	constexpr std::uint8_t ControlChange = 0xB0;
	constexpr std::uint8_t ProgramChange = 0xC0;
	constexpr std::uint8_t ChannelPressure = 0xD0;
	constexpr std::uint8_t PitchBend = 0xE0;

	constexpr std::uint8_t SysEx_Standard = 0xF0;
	constexpr std::uint8_t SysEx_NoEnd = 0xF7;
	constexpr std::uint8_t Meta = 0xFF;

	std::uint8_t type = 0;
	std::uint8_t channelIndex = 0;
	if (anEventStatus < SysEx_Standard)
	{
		type = (anEventStatus & 0xF0);
		channelIndex = anEventStatus & 0x0F;
	}
	else
	{
		type = anEventStatus;
		channelIndex = 0xFF;
	}

	switch (type)
	{
	case NoteOff:
	case NoteOn:
		return ReadNoteEvent(aStream, aTickCount, type == NoteOn, channelIndex);
	case PolyKeyPressure:
		return ReadPolyphonicKeyPressureEvent(aStream, aTickCount, channelIndex);
	case ControlChange:
		return ReadControlChangeEvent(aStream, aTickCount, channelIndex);
	case ProgramChange:
		return ReadProgramChangeEvent(aStream, aTickCount, channelIndex);
	case ChannelPressure:
		return ReadChannelPressureEvent(aStream, aTickCount, channelIndex);
	case PitchBend:
		return ReadPitchBendEvent(aStream, aTickCount, channelIndex);
	case SysEx_Standard:
	case SysEx_NoEnd:
		return ReadSysExEvent(aStream, aTickCount, type == SysEx_Standard);
	case Meta:
		return ReadMetaEvent(aStream, aTickCount);
	}

	throw std::runtime_error("Invalid event type.");
}

std::uint32_t MidiDecoder::ReadNoteEvent(std::istream& aStream, std::uint32_t aTickCount, bool anIsOn, std::uint8_t aChannelIndex)
{
	std::uint8_t noteNum = 0;
	ReadFixed(aStream, noteNum);

	std::uint8_t noteVelocity = 0;
	ReadFixed(aStream, noteVelocity);

	if (!anIsOn)
		noteVelocity = 0;

	OnNote.Invoke(aTickCount, aChannelIndex, noteNum, noteVelocity);

	return 2;
}

std::uint32_t MidiDecoder::ReadPolyphonicKeyPressureEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex)
{
	std::uint8_t noteNum = 0;
	ReadFixed(aStream, noteNum);

	std::uint8_t notePressure = 0;
	ReadFixed(aStream, notePressure);

	OnNotePressure.Invoke(aTickCount, aChannelIndex, noteNum, notePressure);

	return 2;
}

std::uint32_t MidiDecoder::ReadControlChangeEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex)
{
	std::uint8_t controllerNum = 0;
	ReadFixed(aStream, controllerNum);

	std::uint8_t controlValue = 0;
	ReadFixed(aStream, controlValue);

	switch (controllerNum)
	{
	case 120:
		OnSoundOff.Invoke(aTickCount, aChannelIndex);
		break;
	case 121:
		OnResetAllControllers.Invoke(aTickCount, aChannelIndex, controlValue);
		break;
	case 122:
		OnLocalControl.Invoke(aTickCount, aChannelIndex);
		break;
	case 123:
		OnAllNotesOff.Invoke(aTickCount, aChannelIndex);
		break;
	case 124:
		OnOmniOff.Invoke(aTickCount, aChannelIndex);
		break;
	case 125:
		OnOmniOn.Invoke(aTickCount, aChannelIndex);
		break;
	case 126:
		OnMonoOn.Invoke(aTickCount, aChannelIndex, controlValue);
		break;
	case 127:
		OnPolyOn.Invoke(aTickCount, aChannelIndex);
		break;
	default:
		Debug::LogWarning("Unknown Event: %i Control change - N: %i V: %i", aChannelIndex, controllerNum, controlValue);
		break;
	}

	return 2;
}

std::uint32_t MidiDecoder::ReadProgramChangeEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex)
{
	std::uint8_t programNum = 0;
	ReadFixed(aStream, programNum);
	OnProgramChange.Invoke(aTickCount, aChannelIndex, programNum);
	return 1;
}

std::uint32_t MidiDecoder::ReadChannelPressureEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex)
{
	std::uint8_t pressure = 0;
	ReadFixed(aStream, pressure);
	OnChannelPressure.Invoke(aTickCount, aChannelIndex, pressure);
	return 1;
}

std::uint32_t MidiDecoder::ReadPitchBendEvent(std::istream& aStream, std::uint32_t aTickCount, std::uint8_t aChannelIndex)
{
	std::uint8_t noteNum = 0;
	ReadFixed(aStream, noteNum);

	std::uint8_t leastSignificant = 0;
	std::uint8_t mostSignificant = 0;
	ReadFixed(aStream, leastSignificant);
	ReadFixed(aStream, mostSignificant);

	aTickCount;

	Debug::LogWarning("Unfinished Event: %i Pitch bend - lsb: %i msb: %i", aChannelIndex, leastSignificant, mostSignificant);

	return 3;
}

std::uint32_t MidiDecoder::ReadSysExEvent(std::istream& aStream, std::uint32_t aTickCount, bool aHasEnd)
{
	std::uint32_t readBytes = 0;

	std::uint32_t length = 0;
	readBytes += ReadVariable(aStream, length);

	std::unique_ptr<std::uint8_t> data(new std::uint8_t[length]);
	aStream.read(reinterpret_cast<char*>(data.get()), length);

	OnSysEx.Invoke(aTickCount, std::span<const std::uint8_t>(data.get(), aHasEnd ? length - 1 : length));

	return readBytes + length;
}

std::uint32_t MidiDecoder::ReadMetaEvent(std::istream& aStream, std::uint32_t aTickCount)
{
	std::uint32_t readBytes = 0;

	unsigned char type = 0;
	readBytes += ReadFixed(aStream, type);

	std::uint32_t length = 0;
	readBytes += ReadVariable(aStream, length);
	readBytes += length;

	// We have the length so we add it onto the read bytes, just read the data in the switch.
	switch (type)
	{
	case 0x00:
	{
		std::uint16_t sequenceNumber = 0;
		ReadFixed(aStream, sequenceNumber);
		OnSequenceNumber.Invoke(aTickCount, sequenceNumber);
		return readBytes;
	}
	case 0x01:
	{
		std::string textData = ReadText(aStream, length);
		OnText.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x02:
	{
		std::string textData = ReadText(aStream, length);
		OnCopyrightNotice.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x03:
	{
		std::string textData = ReadText(aStream, length);
		OnTrackName.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x04:
	{
		std::string textData = ReadText(aStream, length);
		OnInstrumentName.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x05:
	{
		std::string textData = ReadText(aStream, length);
		OnLyric.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x06:
	{
		std::string textData = ReadText(aStream, length);
		OnMarker.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x07:
	{
		std::string textData = ReadText(aStream, length);
		OnCuePoint.Invoke(aTickCount, textData);
		return readBytes;
	}
	case 0x20:
	{
		std::uint8_t channelPrefix = 0;
		ReadFixed(aStream, channelPrefix);
		Debug::LogWarning("Unknown Event: Meta - MIDI channel prefix: %i", channelPrefix);
		return readBytes;
	}
	case 0x2F:
	{
		OnTrackEnd.Invoke(aTickCount);
		return readBytes;
	}
	case 0x51:
	{
		std::uint32_t tempo = 0;
		ReadFixed(aStream, tempo, 1);
		OnSetTempo.Invoke(aTickCount, tempo);
		return readBytes;
	}
	case 0x54:
	{
		Debug::Log("Event: Meta - SMPTE offset");
		break;
	}
	case 0x58:
	{
		std::uint8_t num = 0;
		std::uint8_t den = 0;
		std::uint8_t clk = 0;
		std::uint8_t base = 0;
		ReadFixed(aStream, num);
		ReadFixed(aStream, den);
		den = 1 << den;
		ReadFixed(aStream, clk);
		ReadFixed(aStream, base);
		OnTimeSignature.Invoke(aTickCount, num, den, clk, base);
		return readBytes;
	}
	case 0x59:
	{
		std::uint8_t shpFlt = 0;
		std::uint8_t majMin = 0;
		ReadFixed(aStream, shpFlt);
		ReadFixed(aStream, majMin);
		OnKeySignature.Invoke(aTickCount, shpFlt, majMin);
		return readBytes;
	}
	case 0x7F:
	{
		Debug::Log("Event: Meta - Vendor-defined");
		break;
	}
	}

	// Event data not processed, skip over.
	aStream.seekg(length, std::ios::cur);

	return readBytes;
}
