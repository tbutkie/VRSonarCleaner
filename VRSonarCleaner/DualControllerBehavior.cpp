#include "DualControllerBehavior.h"



DualControllerBehavior::DualControllerBehavior(ViveController* primaryController, ViveController* secondaryController)
	: m_pPrimaryController(primaryController)
	, m_pSecondaryController(secondaryController)
{
	m_pPrimaryController->attach(this);
	m_pSecondaryController->attach(this);
}


DualControllerBehavior::~DualControllerBehavior()
{
	m_pSecondaryController->detach(this);
	m_pPrimaryController->detach(this);
}
