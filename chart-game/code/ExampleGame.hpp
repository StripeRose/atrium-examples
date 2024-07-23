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
	ExampleGame(Atrium::EngineInstance& anEngineInstance);
	~ExampleGame();

private:
	void HandleStart();
	void HandleLoop();
	void HandleImGui();
	void HandleExit();

	void OnStart_SetupWindows();

	Atrium::EngineInstance& myEngineInstance;

	std::shared_ptr<Atrium::Core::RenderTexture> myWindow1;

	ChartTestWindow myChartTestWindow;
	ChartRenderer myChartRenderer;
	ChartPlayer myChartPlayer;
};
