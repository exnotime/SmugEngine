#include <Core/engine.h>
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <Engine/Engine.h>
#include <Core/subsystem/SubSystemSet.h>

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

using namespace smug;
smug::SubSystemSet engineSubSystems;

int main(int argc, char* argv[]) {
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //detect memory leaks
	//_CrtSetBreakAlloc(18942); // mem leak debugger
#endif
	engine::LoadSubSystems(engineSubSystems);
	int i = 0;
	//smug::Engine eng;
	//eng.Init();
	//eng.Run();
	//eng.Shutdown();


	return 0;
}