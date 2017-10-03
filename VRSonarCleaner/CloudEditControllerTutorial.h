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

	std::vector<SonarPointCloud*> m_vpClouds;
	ColorScaler* m_pColorScaler;

	void refreshColorScale();
};

