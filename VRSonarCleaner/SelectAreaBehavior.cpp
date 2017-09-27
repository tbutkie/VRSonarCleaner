#include "SelectAreaBehavior.h"

#include "DebugDrawer.h"

SelectAreaBehavior::SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* dataVolume)
	: DualControllerBehavior(primaryController, secondaryController)
	, m_pDataVolume(dataVolume)
{
}


SelectAreaBehavior::~SelectAreaBehavior()
{
}

void SelectAreaBehavior::update()
{
}

void SelectAreaBehavior::draw()
{
	glm::vec3 planeOrigin = m_pDataVolume->getPosition() + glm::rotate(m_pDataVolume->getOrientation(), glm::vec3(0.f, 0.f, 0.5f)) * m_pDataVolume->getDimensions().z;
	glm::vec3 planeNormal = glm::rotate(m_pDataVolume->getOrientation(), glm::vec3(0.f, 0.f, 1.f));
	glm::vec3 rayOrigin = glm::vec3(m_pPrimaryController->getPose()[3]);
	glm::vec3 rayDirection = glm::normalize(glm::vec3(-m_pPrimaryController->getPose()[2]));
	float distance = glm::dot(planeNormal, (planeOrigin - rayOrigin)) / glm::dot(planeNormal, rayDirection);

	if (distance >= 0.f)
	{
		glm::vec3 location = rayOrigin + (rayDirection * distance);

		glm::vec3 size(0.01f);

		glm::mat4 trans(glm::translate(glm::mat4(), location) * glm::scale(glm::mat4(), size));

		Renderer::getInstance().drawPrimitive("icosphere", trans, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(1.f, 1.f, 1.f, 0.5f), 10.f);
	}
}

void SelectAreaBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	default:
		break;
	}
}