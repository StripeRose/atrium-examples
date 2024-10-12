// Filter "Chart/Playback"
#pragma once

#include "ChartController.hpp"

class ChartHumanController : public ChartController
{
public:
	virtual const char* GetName() const override { return "Human player"; }

	void HandleInput(const Atrium::Core::InputEvent& anInputEvent);
};
