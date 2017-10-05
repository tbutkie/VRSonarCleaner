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

	unsigned int m_uiBadPoint1;
	unsigned int m_uiBadPoint2;
	unsigned int m_uiBadPoint3;
	unsigned int m_uiBadPoint4;

	void refreshColorScale();

	void makeBadDataLabels(float width);
	void cleanupBadDataLabels();
};

