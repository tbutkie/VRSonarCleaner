#pragma once
#include "SceneBase.h"

#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"
#include "Renderer.h"
#include <random>
#include <chrono>

class FishTankScene :
	public Scene
{
public:
	FishTankScene(TrackedDeviceManager* pTDM);
	~FishTankScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;

	glm::vec3 m_vec3ScreenCenter;
	glm::vec3 m_vec3ScreenNormal;
	glm::vec3 m_vec3ScreenUp;

	glm::vec2 m_vec2ScreenSizeMeters;

	glm::mat4 m_mat4ScreenToWorld;
	glm::mat4 m_mat4WorldToScreen;

	glm::mat4 m_mat4TrackerToEyeCenterOffset;

	bool m_bHeadTracking;

private:
	void calcWorldToScreen();
};
