#pragma once
#include "SceneBase.h"
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "Renderer.h"

class MotionCompensationScene :
	public Scene
{
public:
	MotionCompensationScene(TrackedDeviceManager* pTDM);
	~MotionCompensationScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;

	bool m_bMotionCompensation;
	std::vector<InitializableBehavior*> m_vTrials;
};

