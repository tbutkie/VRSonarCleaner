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
	SonarScene(SDL_Window* pWindow, TrackedDeviceManager* pTDM);
	~SonarScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	SDL_Window * m_pWindow;
	TrackedDeviceManager* m_pTDM;
	DataVolume *m_pTableVolume, *m_pWallVolume;

	glm::vec3 m_vec3RoomSize;

	ColorScaler* m_pColorScalerTPU;

	std::vector<SonarPointCloud*> m_vpClouds;
	std::vector<DataVolume*> m_vpDataVolumes;

	bool m_bUseDesktop;
	bool m_bUseVR;
	bool m_bStudyMode;

	bool m_bLeftMouseDown;
	bool m_bRightMouseDown;
	bool m_bMiddleMouseDown;

	bool m_bInitialColorRefresh;

private:
	void refreshColorScale(ColorScaler* colorScaler, std::vector<SonarPointCloud*> clouds);
};

