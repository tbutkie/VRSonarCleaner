#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "DataLogger.h"
#include "Renderer.h"

#include <queue>

class StudyTutorialDesktopBehavior :
	public InitializableEventBehavior
{
public:
	StudyTutorialDesktopBehavior();
	~StudyTutorialDesktopBehavior();

	void init();

	void processEvent(SDL_Event &ev);

	void update();

	void draw();

	void reset();

	void finish();

	void setupViews();

private:
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	bool m_bLeftMouseDown;
	bool m_bRightMouseDown;
	bool m_bMiddleMouseDown;

	unsigned int m_nPointsLeft;
	unsigned int m_nPointsCleaned;
	unsigned int m_nCleanedGoodPoints;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdate;

	bool m_bPointsCleaned;
};

