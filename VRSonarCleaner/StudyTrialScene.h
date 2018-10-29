#pragma once
#include "SceneBase.h"

#include "TrackedDeviceManager.h"
#include "VectorFieldGenerator.h"
#include "DataVolume.h"
#include "DataLogger.h"

class StudyTrialScene :
	public Scene
{
public:
	StudyTrialScene(TrackedDeviceManager* pTDM);
	~StudyTrialScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	VectorFieldGenerator* m_pVFG;

	glm::vec3 m_vvvec3Streamlines;
};

