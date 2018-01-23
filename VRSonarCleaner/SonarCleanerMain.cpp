#include "CMainApplication.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"
#include "CloudCollection.h"
#include "LassoWindow.h"
#include <conio.h>
#include <cstdio> // fclose

CloudCollection *clouds;
ColorScaler *colorScalerTPU;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	//AllocConsole();
	//freopen("CONOUT$", "wb", stdout);

	printf("CCOM VR Project Modes:\n\n\t0: VR Cleaner\n\t1: VR Flow\n\t2: Study-VR\n\t3: Study-Desktop\n\nSelect Mode: ");
	int selectedMode;
	scanf("%d", &selectedMode);
	while (getchar() != '\n');
	//scanf("%d", &selectedMode);


	//bool lassoMode = false;
	if (selectedMode == 0) //VR Cleaner
	{
		colorScalerTPU = new ColorScaler();
		colorScalerTPU->setColorScale(2);
		colorScalerTPU->setBiValueScale(1);

		clouds = new CloudCollection();
		clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt");
		///	clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_XL_901_1458.txt");  //TO BIG AND LONG at 90 degree angle to others
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt");	
		clouds->calculateCloudBoundsAndAlign();

		CMainApplication *pMainApplication = new CMainApplication(argc, argv, 0);

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
	}
	else if (selectedMode == 1) //VR Flow
	{
		CMainApplication *pMainApplication = new CMainApplication(argc, argv, 1);

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
	}
	else if (selectedMode == 2) //Study VR
	{
		colorScalerTPU = new ColorScaler();
		colorScalerTPU->setColorScale(2);
		colorScalerTPU->setBiValueScale(1);

		clouds = new CloudCollection();
		clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt");
		///	clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_XL_901_1458.txt");  //TO BIG AND LONG at 90 degree angle to others
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt");	
		clouds->calculateCloudBoundsAndAlign();

		CMainApplication *pMainApplication = new CMainApplication(argc, argv, 0);

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
	}
	else if (selectedMode == 3)
	{
		LassoWindow *lassoWindow = new LassoWindow(argc, argv);

		if (!lassoWindow->BInit())
		{
			lassoWindow->Shutdown();
			return 1;
		}

		lassoWindow->RunMainLoop();

		//this doesnt work here?
		fclose(stdout);
		FreeConsole();

		lassoWindow->Shutdown();

	}

	exit(0);
	return 0;
}
