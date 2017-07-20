#include "CMainApplication.h"
#include <conio.h>
#include <cstdio> // fclose


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

	printf("Select Mode:\n");
	printf("\tVirtual Reality\n\t\t1: Sonar Data Cleaner\n\t\t2: Sonar Data Cleaner (Study Mode)\n\t\t3: Flow Vis 4D (Random Flow Field)\n\t\t4: Flow Vis 4D (Great Bay Flow Model)\n\t\t5: Flow Vis 4D (Study Mode)\n");
	printf("\tStandard Desktop Window\n\t\t6: Sonar Data Cleaner\n\t\t7: Sonar Data Cleaner (Study Mode)\n\t\t8: Flow Vis 4D (Random Flow Field)\n\t\t9: Flow Vis 4D (Great Bay Flow Model)\n");
	char selectedMode;
	selectedMode = getch();

	CMainApplication *pMainApplication = new CMainApplication(argc, argv, (int)(selectedMode - '0'));

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
