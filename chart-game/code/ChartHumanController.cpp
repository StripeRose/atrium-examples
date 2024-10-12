// Filter "Chart/Playback"
#include "ChartHumanController.hpp"

void ChartHumanController::HandleInput(const Atrium::Core::InputEvent& anInputEvent)
{
	using namespace Atrium::Core;

	std::optional<int> lane;

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

	if (lane.has_value())
		myLaneStates[lane.value()] = anInputEvent.Value > 0.5f;
}