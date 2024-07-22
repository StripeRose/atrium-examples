#include "ExampleGame.hpp"

#include <Engine_Instance.hpp>

#include <Windows.h>

#if defined(_CONSOLE)
int main(int, const char**)
#else
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
#endif
{
	if (std::unique_ptr<RoseGold::EngineInstance> engineInstance = RoseGold::EngineInstance::Create())
	{
		ExampleGame game(*engineInstance);
		engineInstance->Run();
	}

	return 0;
}
