#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"

class SnellenTest :
	public InitializableBehavior
{
public:
	SnellenTest(TrackedDeviceManager* pTDM);
	virtual ~SnellenTest();

	void init();

	void update();

	void draw();

private:
	std::string generateSnellenString(int len = 6);

	TrackedDeviceManager *m_pTDM;

	bool m_bWaitForTriggerRelease;
	std::vector<char> m_vSloanLetters;
	std::string m_strCurrent;
};

