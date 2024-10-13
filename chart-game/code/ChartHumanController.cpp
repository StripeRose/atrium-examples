// Filter "Chart/Playback"
#include "ChartHumanController.hpp"

void ChartHumanController::HandleInput(const Atrium::Core::InputEvent& anInputEvent)
{
	using namespace Atrium::Core;

	switch (anInputEvent.Source)
	{
		case InputSourceId::Keyboard::Alpha1:
		case InputSourceId::Keyboard::A:
			SetLane(0, anInputEvent.Value > 0.5f);
			break;
		case InputSourceId::Keyboard::Alpha2:
		case InputSourceId::Keyboard::S:
			SetLane(1, anInputEvent.Value > 0.5f);
			break;
		case InputSourceId::Keyboard::Alpha3:
		case InputSourceId::Keyboard::D:
		case InputSourceId::Keyboard::J:
			SetLane(2, anInputEvent.Value > 0.5f);
			break;
		case InputSourceId::Keyboard::Alpha4:
		case InputSourceId::Keyboard::K:
			SetLane(3, anInputEvent.Value > 0.5f);
			break;
		case InputSourceId::Keyboard::Alpha5:
		case InputSourceId::Keyboard::L:
			SetLane(4, anInputEvent.Value > 0.5f);
			break;

		case InputSourceId::Keyboard::Spacebar:
			Strum();
			break;
	}
}