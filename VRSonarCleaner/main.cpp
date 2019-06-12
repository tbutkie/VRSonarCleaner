#include "Engine.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	AllocConsole();

	bool bVR = false;
	bool bNoHMD = false;
	bool bStereoGL = false;
	bool bDemo = false;
	int nScreenIndex = 0;
	float fScreenDiagInches = 24.f;

	for (int i = 1; i < argc; i++)
	{
		if (!stricmp(argv[i], "-vr"))
		{
			bVR = true;
		}
		else if (!stricmp(argv[i], "-nohmd"))
		{
			bNoHMD = true;
		}
		else if (!stricmp(argv[i], "-stereogl"))
		{
			bStereoGL = true;
		}
		if (!stricmp(argv[i], "-demo"))
		{
			bDemo = true;
		}
		else if (!stricmp(argv[i], "-display") && (argc > i + 1) && (*argv[i + 1] != '-'))
		{
			nScreenIndex = atoi(argv[i + 1]);
			i++;
		}
		else if (!stricmp(argv[i], "-diagonal") && (argc > i + 1) && (*argv[i + 1] != '-'))
		{
			fScreenDiagInches = static_cast<float>(atof(argv[i + 1]));
			i++;
		}
		else if (!stricmp(argv[i], "-h") || !stricmp(argv[i], "-help"))
		{
			printf_s("CCOM VR Engine\n");
			printf_s("\t-vr\tEnable OpenVR\n");
			printf_s("\t-nohmd\tEnable OpenVR but do not render to the HMD\n");
			printf_s("\t-stereogl\tEnable Quad-Buffered Stereo OpenGL Context\n");
			printf_s("\t-display i\tPut window on display i\n");
			printf_s("\t-diagonal d\tSet display diagonal d in inches\n");
			printf_s("\n");
		}
	}

	Engine *pEngine = new Engine(bDemo, bVR, !bNoHMD, bStereoGL, nScreenIndex, fScreenDiagInches);

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
