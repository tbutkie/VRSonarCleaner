#pragma once
#include "BehaviorBase.h"

#include <vector>

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include "SonarPointCloud.h"
#include "ColorScaler.h"

#include "PointCleanProbe.h"

class StudyEditTutorial :
	public InitializableBehavior
{
public:
	StudyEditTutorial(TrackedDeviceManager* pTDM);
	virtual ~StudyEditTutorial();

	void init();
	
	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume *m_pDemoVolume;

	SonarPointCloud* m_pDemoCloud;
	ColorScaler* m_pColorScaler;

	PointCleanProbe *m_pProbe;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpTimestamp;

	std::vector<glm::vec3> m_vvec3BadPoints;

	void cleanup();

	void refreshColorScale();
};

