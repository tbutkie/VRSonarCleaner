#include "DualControllerBehavior.h"

DualControllerBehavior::DualControllerBehavior(ViveController* primaryController, ViveController* secondaryController)
	: m_pPrimaryController(primaryController)
	, m_pSecondaryController(secondaryController)
{
}


DualControllerBehavior::~DualControllerBehavior()
{
}
