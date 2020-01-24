#pragma once

#include "Renderer.h"
#include "SceneBase.h"
#include "TrackedDeviceManager.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"
#include <SDL.h>

class SonarScene :
	public Scene
{
public:
	SonarScene(TrackedDeviceManager* pTDM);
	~SonarScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;

	DataVolume *m_pDataVolume;
	ColorScaler* m_pColorScalerTPU;

	std::vector<SonarPointCloud*> m_vpClouds;
	std::vector<DataVolume*> m_vpDataVolumes;

	bool m_bUseDesktop;

	bool m_bLeftMouseDown;
	bool m_bRightMouseDown;
	bool m_bMiddleMouseDown;

	bool m_bInitialColorRefresh;

	bool(*SonarScene::m_funcWindowEasing) (float, bool);

	float m_fTransitionRate; //sec


private:
	static bool easeIn(float transitionRate, bool reset = false);
	static bool easeOut(float transitionRate, bool reset = false);
	void refreshColorScale(ColorScaler* colorScaler, std::vector<SonarPointCloud*> clouds);
};

