#pragma once
#include "SceneBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"
#include <SDL.h>

class FlowScene :
	public Scene
{
public:
	FlowScene(TrackedDeviceManager* pTDM);
	~FlowScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	FlowVolume* m_pFlowVolume;

	glm::vec3 m_vec3RoomSize;
};

