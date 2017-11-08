#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"

class SnellenTest :
	public InitializableBehavior
{
public:
	SnellenTest(TrackedDeviceManager* pTDM, float minutesOfArc);
	virtual ~SnellenTest();

	void init();

	void update();

	void draw();

	void newTest();

	void setVisualAngle(float minutesOfArc);

	float getVisualAngle();

private:
	std::string generateSnellenString();
	float getHeightForOptotype(float minutesOfArc);

	TrackedDeviceManager *m_pTDM;

	bool m_bWaitForTriggerRelease;
	std::vector<char> m_vSloanLetters;
	std::string m_strCurrent;
	float m_fVisualAngle;

	float s_fOptotypeGrating = 5.f;
	float s_fDistance = 6.f;
	int s_nCharacters = 6;
};

