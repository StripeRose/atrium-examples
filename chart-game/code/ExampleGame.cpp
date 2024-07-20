#include "stdafx.hpp"

#include "ExampleGame.hpp"

#include <Common_Diagnostics.hpp>

ExampleGame::ExampleGame(RoseGold::EngineInstance& anEngineInstance)
	: myEngineInstance(anEngineInstance)
	, myChartTestWindow(myChartPlayer)
	, myChartRenderer(myChartPlayer)
{
	myEngineInstance.OnStart.Connect(this, [&]() { HandleStart(); });
	myEngineInstance.OnLoop.Connect(this, [&]() { HandleLoop(); });
	myEngineInstance.OnImGui.Connect(this, [&]() { HandleImGui(); });
	myEngineInstance.OnExit.Connect(this, [&]() { HandleExit(); });
}

ExampleGame::~ExampleGame()
{
	myEngineInstance.OnStart.Disconnect(this);
	myEngineInstance.OnLoop.Disconnect(this);
	myEngineInstance.OnImGui.Disconnect(this);
	myEngineInstance.OnExit.Disconnect(this);
}

void ExampleGame::HandleStart()
{
	ZoneScoped;

	OnStart_SetupWindows();

	myChartRenderer.SetupResources(myEngineInstance.GetGraphicsAPI(), myWindow1->GetDescriptor().ColorGraphicsFormat);
}

void ExampleGame::HandleLoop()
{
	if (myEngineInstance.GetWindowManager().GetWindows().empty())
		myEngineInstance.Stop();

	ZoneScoped;

	using namespace RoseGold::Core;
	FrameContext& frameContext = myEngineInstance.GetGraphicsAPI().GetCurrentFrameContext();

	if (myWindow1)
	{
		{
			CONTEXT_ZONE(frameContext, "Clear");
			frameContext.ClearColor(myWindow1, RoseGold::Color::Predefined::Black);
			frameContext.ClearDepth(myWindow1, 1.f, 0);
		}

		myChartRenderer.Render(frameContext, myWindow1);
	}
}

void ExampleGame::HandleImGui()
{
	myChartTestWindow.ImGui();
}

void ExampleGame::HandleExit()
{
	ZoneScoped;
}

void ExampleGame::OnStart_SetupWindows()
{
	ZoneScoped;
	RoseGold::Core::WindowManager::CreationParameters windowParams;
	windowParams.Title = "Window 1";
	windowParams.Size = { 640, 480 };
	auto window1 = myEngineInstance.GetWindowManager().NewWindow(windowParams);
	myWindow1 = myEngineInstance.GetGraphicsAPI().GetResourceManager().CreateRenderTextureForWindow(*window1);
	window1->Closed.Connect(nullptr, [&](RoseGold::Core::Window&) {
		myWindow1.reset();
		});

	myEngineInstance.InitializeImGui(*window1, myWindow1);
}
