#include "CMainApplication.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"


SonarPointCloud* cloud;
ColorScaler *colorScalerTPU;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{

	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

	colorScalerTPU = new ColorScaler();
	colorScalerTPU->setColorScale(2);

	cloud = new SonarPointCloud();
	cloud->loadFromSonarTxt("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt");
	


	//cloud->loadFromSonarTxt("H126.txt");
	

	CMainApplication *pMainApplication = new CMainApplication( argc, argv );

	if (!pMainApplication->BInit())
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
