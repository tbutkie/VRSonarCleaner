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
	DesktopCleanBehavior(DataVolume* pointCloudVolume, Renderer::SceneViewInfo *sceneinfo, glm::ivec4 &viewport);
	virtual ~DesktopCleanBehavior();

	void init();
	void update();
	void draw();

	void activate();

private:
	DataVolume* m_pDataVolume;
	Renderer::SceneViewInfo *m_pDesktop3DViewInfo;
	glm::ivec4 m_ivec4Viewport;

	LassoTool *m_pLasso;
	ArcBall *m_pArcball;


private:
	unsigned int checkPoints();
};

