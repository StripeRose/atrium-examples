// Filter "Chart/Playback"
#include "ChartHumanController.hpp"

void ChartHumanController::HandleInput(const Atrium::Core::InputEvent& anInputEvent)
{
	using namespace Atrium::Core;

	std::uint8_t lane = 0xFF;

	switch (anInputEvent.Source)
	{
		case InputSourceId::Keyboard::Alpha1:
			lane = 0;
			break;
		case InputSourceId::Keyboard::Alpha2:
			lane = 1;
			break;
		case InputSourceId::Keyboard::Alpha3:
			lane = 2;
			break;
		case InputSourceId::Keyboard::Alpha4:
			lane = 3;
			break;
		case InputSourceId::Keyboard::Alpha5:
			lane = 4;
			break;
	}

	if (lane != 0xFF)
		SetLane(lane, anInputEvent.Value > 0.5f);
}