#include "engine.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main(int argc, char* argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //detect memory leaks
	//_CrtSetBreakAlloc(4967); // mem leak debugger
	Engine eng;
	eng.Init();
	eng.Run();
	eng.Shutdown();
	return 0;
}