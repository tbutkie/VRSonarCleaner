#pragma once
#include "BehaviorBase.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "LassoTool.h"
#include "arcball.h"

#define POINT_CLOUD_CLEAN_PROBE_ROTATION_RATE std::chrono::duration<float, std::milli>(2000)
#define POINT_CLOUD_HIGHLIGHT_BLINK_RATE std::chrono::duration<float, std::milli>(250)

class DesktopCleanBehavior :
	public InitializableBehavior
{
public:
	DesktopCleanBehavior(DataVolume* pointCloudVolume);
	virtual ~DesktopCleanBehavior();

	void init();
	void update();
	void draw();

	void activate();

private:
	DataVolume* m_pDataVolume;

	LassoTool *m_pLasso;
	ArcBall *m_pArcball;


private:
	unsigned int checkPoints();
};

