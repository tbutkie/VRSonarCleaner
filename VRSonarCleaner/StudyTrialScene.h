#pragma once
#include "SceneBase.h"
class StudyTrialScene :
	public Scene
{
public:
	StudyTrialScene();
	~StudyTrialScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();
};

