#include "FlowProbe.h"

using namespace std::chrono_literals;

FlowProbe::FlowProbe(TrackedDeviceManager* pTDM, FlowVolume* flowVolume)
	: ProbeBehavior(pTDM, flowVolume)
	, m_bProbeActive(false)
	, m_pFlowVolume(flowVolume)
	, m_pEmitter(NULL)
{
	m_vec4ProbeColorDiff = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
}


FlowProbe::~FlowProbe()
{
	if (m_pEmitter)
		m_pFlowVolume->removeDyeEmitterClosestToWorldCoords(m_pTDM->getPrimaryController() ? getPosition() : m_pFlowVolume->getPosition());
}

void FlowProbe::update()
{
	if (!m_pEmitter)
	{
		m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(m_pTDM->getPrimaryController() ? getPosition() : m_pFlowVolume->getPosition());
		m_pEmitter->incrementColor();
		m_pEmitter->setRadius(0.25f);

		m_vec4ProbeActivateColorDiff = m_vec4ProbeColorSpec = glm::vec4(m_pEmitter->getColor(), 1.f);
	}

	ProbeBehavior::update();

	if (m_pTDM->getPrimaryController())
	{
		m_pEmitter->setRate(10.f + (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f) * 90.f);
		m_pEmitter->setRadius(0.f + (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f) * 0.5f);
		//m_pEmitter->setTrailTime(std::chrono::duration_cast<std::chrono::milliseconds>(2500ms - (2000ms) * (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f)));

		glm::vec3 innerPos = m_pDataVolume->convertToRawDomainCoords(getPosition());
		m_pEmitter->m_vec3Pos = innerPos;

		if (m_pTDM->getPrimaryController()->justPressedTouchpad())
		{
			glm::vec2 touchPt = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint();
			if (touchPt.y > 0.25f)
				placeDyePot();

			if (touchPt.y < -0.25f)
			{
				m_pEmitter->incrementColor();
				m_vec4ProbeActivateColorDiff = m_vec4ProbeColorSpec = glm::vec4(m_pEmitter->getColor(), 1.f);
			}
		}
	}
	else
	{
		m_pEmitter->setRate(0.f);
	}
}

void FlowProbe::draw()
{
	if (m_pTDM->getPrimaryController())
	{
		drawProbe(m_fProbeOffset);

		if (m_pEmitter->getRadius() > 0.f)
		{
			glm::dvec3 sphereRadVec(m_pFlowVolume->convertToWorldCoords(glm::dvec3(m_pEmitter->getRadius())) - m_pFlowVolume->convertToWorldCoords(glm::dvec3(0.)));

			glm::mat4 xForm = getProbeToWorldTransform() * glm::scale(glm::mat4(), glm::abs(glm::vec3(sphereRadVec)));

			Renderer::getInstance().drawPrimitive("icosphere", xForm, glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(m_pEmitter->getColor(), 1.f));
		}
	}
}

void FlowProbe::activateProbe()
{
	//if (m_pTDM->getPrimaryController())
	//{
	//	auto pos = getPosition();
	//	if (m_pDataVolume->isWorldCoordPointInDomainBounds(pos))
	//	{
	//		glm::dvec3 ptXform = m_pDataVolume->convertToRawDomainCoords(pos);
	//		printf("World Pos (%0.4f, %0.4f, %0.4f) is INSIDE the data volume at (%0.4f, %0.4f, %0.4f).\n", pos.x, pos.y, pos.z, ptXform.x, ptXform.y, ptXform.z);
	//	}
	//	else
	//		printf("World Pos (%0.4f, %0.4f, %0.4f) is outside the data volume.\n", pos.x, pos.y, pos.z);
	//}
}

void FlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}

void FlowProbe::placeDyePot()
{
	m_bProbeActive = true;
	m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(getPosition());

	do {
		m_pEmitter->incrementColor();
	} while (m_pEmitter->getColor() == glm::vec3(0.25f, 0.95f, 1.f));

	m_vec4ProbeActivateColorDiff = m_vec4ProbeColorSpec = glm::vec4(m_pEmitter->getColor(), 1.f);
}