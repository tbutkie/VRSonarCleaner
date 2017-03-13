#include "SingleControllerBehavior.h"



SingleControllerBehavior::SingleControllerBehavior(ViveController* controller)
	: m_pController(controller)
{
	m_pController->attach(this);
}


SingleControllerBehavior::~SingleControllerBehavior()
{
	m_pController->detach(this);
}
