#pragma once
#include "BehaviorBase.h"

#include <vector>

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include "SonarPointCloud.h"
#include "ColorScaler.h"

class CloudEditControllerTutorial :
	public InitializableBehavior
{
public:
	CloudEditControllerTutorial(TrackedDeviceManager* pTDM);
	virtual ~CloudEditControllerTutorial();

	void init();
	
	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume *m_pDemoVolume;

	SonarPointCloud* m_pDemoCloud;
	ColorScaler* m_pColorScaler;

	glm::dvec4 m_dvec4BadPointPos1;
	glm::dvec4 m_dvec4BadPointPos2;
	glm::dvec4 m_dvec4BadPointPos3;

	void refreshColorScale();

	void makeBadDataLabels();
	void cleanupBadDataLabels();
};

