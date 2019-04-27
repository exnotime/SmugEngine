#include "engine.h"
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
int main(int argc, char* argv[]) {
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //detect memory leaks
	//_CrtSetBreakAlloc(16001); // mem leak debugger
#endif

	smug::Engine eng;
	eng.Init();
	eng.Run();
	eng.Shutdown();

	return 0;
}