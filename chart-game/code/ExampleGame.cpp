#include "ExampleGame.hpp"

#include <Atrium_Diagnostics.hpp>

ExampleGame::ExampleGame(Atrium::EngineInstance& anEngineInstance)
	: myEngineInstance(anEngineInstance)
	, myChartTestWindow(myChartPlayer, myChartRenderer)
	, myChartRenderer(myChartPlayer)
{
	myEngineInstance.OnStart.Connect(this, [&]() { HandleStart(); });
	myEngineInstance.OnLoop.Connect(this, [&]() { HandleLoop(); });
	myEngineInstance.OnImGui.Connect(this, [&]() { HandleImGui(); });
	myEngineInstance.OnExit.Connect(this, [&]() { HandleExit(); });

	myEngineInstance.GetInputAPI().OnInput.Connect(
		this,
		[&](const auto& anInputEvent)
		{
			HandleInput(anInputEvent);
		}
	);
}

ExampleGame::~ExampleGame()
{
	myEngineInstance.GetInputAPI().OnInput.Disconnect(this);

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

	myChartPlayer.Update();

	using namespace Atrium;
	FrameGraphicsContext& frameContext = myEngineInstance.GetGraphicsAPI().GetCurrentFrameContext();

	if (myWindow1)
	{
		{
			CONTEXT_ZONE(frameContext, "Clear");
			frameContext.ClearColor(myWindow1, Atrium::Color::Predefined::Black);
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
	Atrium::WindowManager::CreationParameters windowParams;
	windowParams.Title = "Window 1";
	windowParams.Size = { 640, 480 };
	auto window1 = myEngineInstance.GetWindowManager().NewWindow(windowParams);
	myWindow1 = myEngineInstance.GetGraphicsAPI().GetResourceManager().CreateRenderTextureForWindow(*window1);
	window1->Closed.Connect(nullptr, [&](Atrium::Window&) {
		myWindow1.reset();
		});

	myEngineInstance.InitializeImGui(*window1, myWindow1);
}

void ExampleGame::HandleInput(const Atrium::InputEvent& anInputEvent)
{
	for (const auto& controller : myChartPlayer.GetControllers())
		controller->HandleInput(anInputEvent);
}
