#include "Engine.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	AllocConsole();

	Engine *pEngine = new Engine();

	if (!pEngine->init())
	{
		pEngine->Shutdown();
		return 1;
	}

	pEngine->RunMainLoop();

	pEngine->Shutdown();

	FreeConsole();

	return 0;
}
