#pragma once
#include "BehaviorBase.h"

#include "ViveController.h"

class DualControllerBehavior :
	public BehaviorBase
{
public:
	DualControllerBehavior(ViveController* primaryController, ViveController* secondaryController);
	~DualControllerBehavior();

protected:
	ViveController* m_pPrimaryController;
	ViveController* m_pSecondaryController;
};

