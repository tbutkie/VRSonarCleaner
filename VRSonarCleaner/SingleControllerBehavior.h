#pragma once
#include "BehaviorBase.h"

#include "ViveController.h"

class SingleControllerBehavior :
	public BehaviorBase
{
public:
	SingleControllerBehavior(ViveController* controller);
	~SingleControllerBehavior();

protected:
	ViveController* m_pController;
};

