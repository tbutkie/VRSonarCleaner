#pragma once
#include "SceneBase.h"

#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"
#include "Renderer.h"
#include <random>
#include <chrono>

class SonarStudyScene :
	public Scene
{
public:
	SonarStudyScene(TrackedDeviceManager* pTDM, float screenDiagInches);
	~SonarStudyScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;

	glm::vec3 m_vec3COPOffsetTrackerSpace;

	glm::vec3 m_vec3ScreenCenter;
	glm::vec3 m_vec3ScreenNormal;
	glm::vec3 m_vec3ScreenUp;

	float m_fScreenDiagonalMeters;

	glm::vec2 m_vec2ScreenSizeMeters;

	glm::mat4 m_mat4ScreenToWorld;
	glm::mat4 m_mat4WorldToScreen;

	glm::mat4 m_mat4TrackerToEyeCenterOffset;

	bool m_bEditMode;
	bool m_bHeadTracking;
	bool m_bStereo;
	float m_fEyeSeparationCentimeters;

	bool m_bInitialColorRefresh;

private:
	void calcWorldToScreen();
	void setupViews();
};
