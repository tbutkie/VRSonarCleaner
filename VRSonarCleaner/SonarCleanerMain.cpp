#include "CMainApplication.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"
#include "CloudCollection.h"
#include "LassoWindow.h"

CloudCollection *clouds;
ColorScaler *colorScalerTPU;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	bool lassoMode = false;

	std::string inputBuffer;


	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONIN$", "r", stdin);

	while (1)
	{
		std::cout << "Enter VR Cleaning Mode (\"vr\") or Desktop Lasso Mode (\"lasso\")? ";
		std::cin >> inputBuffer;
		if (inputBuffer.compare("lasso") == 0)
		{
			lassoMode = true;
			break;
		}

		if (inputBuffer.compare("vr") == 0)
		{
			lassoMode = false;
			break;
		}
	}

	FreeConsole();

	AllocConsole();
	freopen("CONOUT$", "wb", stdout);

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

	if (lassoMode)
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
	else //run regular VR Sonar Cleaner
	{
		CMainApplication *pMainApplication = new CMainApplication(argc, argv);

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

	exit(0);
	return 0;
}
