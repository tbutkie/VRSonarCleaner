#include "FlowProbe.h"
#include <gtc\random.hpp>

using namespace std::chrono_literals;

FlowProbe::FlowProbe(ViveController* pController, FlowVolume* flowVolume)
	: ProbeBehavior(pController, flowVolume)
	, m_bProbeActive(false)
	, m_pFlowVolume(flowVolume)
	, m_pEmitter(NULL)
	, m_fTipEmitterRadius(0.f)
{
	m_vec4ProbeColorDiff = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
}


FlowProbe::~FlowProbe()
{
	if (m_pEmitter)
		m_pFlowVolume->removeDyeEmitterClosestToWorldCoords(m_pController ? getPosition() : m_pFlowVolume->getPosition());
}

void FlowProbe::update()
{
	if (!m_pEmitter)
	{
		m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(m_pController ? getPosition() : m_pFlowVolume->getPosition());
		m_pEmitter->incrementColor();
		m_pEmitter->setRadius(m_fTipEmitterRadius);

		m_vec4ProbeActivateColorDiff = m_vec4ProbeColorSpec = glm::vec4(m_pEmitter->getColor(), 1.f);
	}

	ProbeBehavior::update();

	if (m_pController && m_pController->valid())
	{
		m_pEmitter->setRate(10.f + (m_pController->getTriggerPullAmount() / 0.85f) * 90.f);
		m_fTipEmitterRadius = 0.f + (m_pController->getTriggerPullAmount() / 0.85f) * 0.5f;
		m_pEmitter->setRadius(m_fTipEmitterRadius);
		//m_pEmitter->setTrailTime(std::chrono::duration_cast<std::chrono::milliseconds>(2500ms - (2000ms) * (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f)));

		glm::vec3 innerPos = m_pDataVolume->convertToRawDomainCoords(getPosition());
		m_pEmitter->m_vec3Pos = innerPos;

		if (m_pController->justPressedTouchpad())
		{
			glm::vec2 touchPt = m_pController->getCurrentTouchpadTouchPoint();
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
	if (m_pController)
	{
		drawProbe();

		if (m_pEmitter && m_pEmitter->getRadius() > 0.f)
		{
			glm::vec3 sizer = glm::vec3(glm::dvec3(m_pDataVolume->getDimensions()) / m_pDataVolume->getDataDimensions());
			glm::mat4 xForm = getTransformProbeToWorld() * glm::scale(glm::mat4(), glm::abs(glm::vec3(m_fTipEmitterRadius * sizer)));

			Renderer::getInstance().drawPrimitive("icosphere", xForm, glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(m_pEmitter->getColor(), 1.f));
		}

		glm::vec3 touchDividerStart = glm::vec3(m_pController->getDeviceToWorldTransform() * m_pController->c_vec4TouchPadLeft);
		glm::vec3 touchDividerEnd = glm::vec3(m_pController->getDeviceToWorldTransform() * m_pController->c_vec4TouchPadRight);

		Renderer::getInstance().drawDirectedPrimitiveLit("cylinder", touchDividerStart, touchDividerEnd, 0.0005f, glm::vec4(1.f, 1.f, 0.f, 1.f));

		glm::vec3 placeDyeLabel = glm::vec3(m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.001f, 0.f)) * (m_pController->c_vec4TouchPadCenter + (m_pController->c_vec4TouchPadTop - m_pController->c_vec4TouchPadCenter)/2.f));
		glm::vec4 placeDyeLabelColor = (m_pController->isTouchpadTouched() && m_pController->getCurrentTouchpadTouchPoint().y > 0.25f) ? glm::linearRand(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 1.f)) : glm::vec4(1.f);

		Renderer::getInstance().drawText(
			"PLACE\nDYE",
			placeDyeLabelColor,
			placeDyeLabel,
			glm::quat(m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f))),
			0.01f,
			Renderer::TextSizeDim::HEIGHT
		);

		glm::vec3 changeColorLabel = glm::vec3(m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.001f, 0.f)) * (m_pController->c_vec4TouchPadCenter + (m_pController->c_vec4TouchPadBottom - m_pController->c_vec4TouchPadCenter) / 2.f));
		glm::vec4 changeColorLabelColor = (m_pController->isTouchpadTouched() && m_pController->getCurrentTouchpadTouchPoint().y < -0.25f) ? glm::linearRand(glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 1.f)) : glm::vec4(1.f);

		Renderer::getInstance().drawText(
			"DYE\nCOLOR",
			changeColorLabelColor,
			changeColorLabel,
			glm::quat(m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f))),
			0.01f,
			Renderer::TextSizeDim::HEIGHT
		);
		if (m_pFlowVolume->checkSwirlwWorldCoords(getPosition()))
		{
			glm::vec3 swirlLabelPos((glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.01f)) * getTransformProbeToWorld())[3]);
			Renderer::getInstance().drawText(
				"SWIRL",
				m_vec4ProbeActivateColorDiff = glm::vec4(0.f, 1.f, 0.f, 1.f),
				swirlLabelPos,
				glm::quat(getTransformProbeToWorld()),
				0.025f,
				Renderer::TextSizeDim::HEIGHT
			);
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