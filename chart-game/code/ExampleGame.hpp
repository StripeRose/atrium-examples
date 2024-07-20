#pragma once

#include <Engine_Instance.hpp>

#include "ChartRenderer.hpp"
#include "ChartTestWindow.hpp"
#include "Mesh.hpp"

#include <memory>
#include <vector>

class ExampleGame
{
public:
	ExampleGame(RoseGold::EngineInstance& anEngineInstance);
	~ExampleGame();

private:
	void HandleStart();
	void HandleLoop();
	void HandleImGui();
	void HandleExit();

	void OnStart_SetupWindows();

	RoseGold::EngineInstance& myEngineInstance;

	std::shared_ptr<RoseGold::Core::RenderTexture> myWindow1;

	ChartTestWindow myChartTestWindow;
	ChartRenderer myChartRenderer;
	ChartPlayer myChartPlayer;
};
