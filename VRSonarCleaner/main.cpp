#include "CMainApplication.h"
#include "LassoWindow.h"
#include <conio.h>
#include <cstdio> // fclose


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

	printf("Select Mode:\n\t0=VR Cleaner\n\t1=VR Flow\n\t2=Study-VR\n\t3=Study-Desktop\n");
	char selectedMode;
	selectedMode = getch();

	CMainApplication *pMainApplication = new CMainApplication(argc, argv, selectedMode);

	if (!pMainApplication->init())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();

	//this doesnt work here?
	fclose(stdout);
	FreeConsole();

	pMainApplication->Shutdown();

	exit(0);
	return 0;
}
