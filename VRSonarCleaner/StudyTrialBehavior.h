#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"

class StudyTrialBehavior :
	public InitializableBehavior
{
public:
	StudyTrialBehavior(TrackedDeviceManager* pTDM, std::string fileName, DataLogger::LogHandle log);
	~StudyTrialBehavior();

	void init();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	DataLogger::LogHandle m_Log;
};

