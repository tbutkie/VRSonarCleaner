#include "ScaleDataVolumeBehavior.h"
#include "InfoBoxManager.h"
#include "Renderer.h"

#include <gtc/random.hpp>

ScaleDataVolumeBehavior::ScaleDataVolumeBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_pDataVolume(dataVolume)
	, m_bScaling(false)
{
}


ScaleDataVolumeBehavior::~ScaleDataVolumeBehavior()
{
}



void ScaleDataVolumeBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	updateState();

	if (m_bScaling)
	{
		float delta = controllerDistance() - m_fInitialDistance;
		m_pDataVolume->setDimensions(glm::vec3(exp(delta * 10.f) * m_vec3InitialDimensions));
		m_pDataVolume->update();
	}
}

void ScaleDataVolumeBehavior::draw()
{
	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->readyToRender() &&
		m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->readyToRender())
	{
		bool flash = !m_bScaling && (m_pTDM->getPrimaryController()->isGripButtonPressed() || m_pTDM->getSecondaryController()->isGripButtonPressed());
		std::vector<ViveController*> controllers({ m_pTDM->getPrimaryController() , m_pTDM->getSecondaryController() });

		glm::vec3 primScalePos = (m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.015f, 0.085f)))[3];
		glm::vec3 secScalePos = (m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.015f, 0.085f)))[3];
		glm::vec3 controllerToControllerVec = secScalePos - primScalePos;
		bool rightHanded = glm::dot(glm::cross(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]) - primScalePos, controllerToControllerVec), glm::vec3(m_pTDM->getHMDToWorldTransform()[1])) < 0.f;

		for (auto const &c : controllers)
		{
			glm::mat4 rightGripTextAnchorTrans = c->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.025f, -0.015f, 0.085f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
			glm::mat4 leftGripTextAnchorTrans = c->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(-0.025f, -0.015f, 0.085f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

			glm::vec4 labelColor(1.f);

			if (c->isGripButtonPressed())
				labelColor = glm::vec4(0.2f, 0.88f, 0.88f, 1.f);
			else
				if (flash)
					labelColor = glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f);

			Renderer::getInstance().drawText(
				"Scale",
				labelColor,
				rightGripTextAnchorTrans[3],
				glm::quat(rightGripTextAnchorTrans),
				0.0075f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAnchor::CENTER_LEFT
			);
			Renderer::getInstance().drawText(
				"Scale",
				labelColor,
				leftGripTextAnchorTrans[3],
				glm::quat(leftGripTextAnchorTrans),
				0.0075f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAnchor::CENTER_RIGHT
			);

			Renderer::getInstance().drawConnector(
				rightGripTextAnchorTrans[3],
				(c->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.015f, -0.015f, 0.085f)))[3],
				0.001f,
				glm::vec4(1.f, 1.f, 1.f, 0.75f)
			);
			Renderer::getInstance().drawConnector(
				leftGripTextAnchorTrans[3],
				(c->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(-0.015f, -0.015f, 0.085f)))[3],
				0.001f,
				glm::vec4(1.f, 1.f, 1.f, 0.75f)
			);
		}

		if (m_bScaling)
		{
			glm::vec4 color;
			bool enlarging = controllerDistance() > m_fInitialDistance;

			if (enlarging)
				color = glm::vec4(0.88f, 0.1f, 0.2f, 0.5f);
			else
				color = glm::vec4(0.2f, 0.1f, 0.88f, 0.5f);

			float thickness = 0.0025f;

			glm::vec3 primConAttachPt = (m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(rightHanded ? -0.015f : 0.015f, -0.015f, 0.085f)))[3];
			glm::vec3 secConAttachPt = (m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(rightHanded ? 0.015f : -0.015f, -0.015f, 0.085f)))[3];

			Renderer::getInstance().drawConnector(
				primConAttachPt,
				m_pDataVolume->getPosition(),
				thickness,
				glm::vec4(1.f, 1.f, 1.f, 0.25f)
			); Renderer::getInstance().drawConnector(
				secConAttachPt,
				m_pDataVolume->getPosition(),
				thickness,
				glm::vec4(1.f, 1.f, 1.f, 0.25f)
			);

			glm::vec3 attachVec = secConAttachPt - primConAttachPt;
			glm::vec3 attachVec_n = glm::normalize(attachVec);
			glm::vec3 midPt = primConAttachPt + attachVec * 0.5f;

			if (enlarging)
			{
				Renderer::getInstance().drawConnector(
					midPt - attachVec_n * m_fInitialDistance * 0.5f,
					midPt + attachVec_n * m_fInitialDistance * 0.5f,
					thickness * 2.f,
					glm::vec4(1.f, 1.f, 1.f, 0.25f)
				);
				Renderer::getInstance().drawConnector(
					primConAttachPt,
					midPt - attachVec_n * m_fInitialDistance * 0.5f,
					thickness,
					color
				);
				Renderer::getInstance().drawConnector(
					secConAttachPt,
					midPt + attachVec_n * m_fInitialDistance * 0.5f,
					thickness,
					color
				);
			}
			else
			{
				Renderer::getInstance().drawConnector(
					midPt - attachVec_n * m_fInitialDistance * 0.5f,
					primConAttachPt,
					thickness * 2.f,
					glm::vec4(1.f, 1.f, 1.f, 0.25f)
				);
				Renderer::getInstance().drawConnector(
					midPt + attachVec_n * m_fInitialDistance * 0.5f,
					secConAttachPt,
					thickness * 2.f,
					glm::vec4(1.f, 1.f, 1.f, 0.25f)
				);
				Renderer::getInstance().drawConnector(
					primConAttachPt,
					secConAttachPt,
					thickness,
					color
				);
			}

			glm::vec3 u = glm::normalize(rightHanded ? -attachVec : attachVec);
			glm::vec3 v = glm::normalize(glm::cross(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]) - midPt, u));
			glm::vec3 w = glm::normalize(glm::cross(u, v));

			glm::mat3 rot;
			rot[0] = u;
			rot[1] = v;
			rot[2] = w;

			std::string scaleMagStr(std::to_string(exp((controllerDistance() - m_fInitialDistance) * 10.f)) + "x");

			float textHeight = 0.025f;

			Renderer::getInstance().drawText(
				scaleMagStr,
				color,
				midPt + w * 0.025f,
				glm::quat(rot),
				textHeight,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAnchor::CENTER
			);
		}
	}
}

void ScaleDataVolumeBehavior::updateState()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;
		
	if ((m_pTDM->getSecondaryController()->justPressedGrip() && m_pTDM->getPrimaryController()->isGripButtonPressed()) ||
		(m_pTDM->getPrimaryController()->justPressedGrip() && m_pTDM->getSecondaryController()->isGripButtonPressed()))
	{
		m_bScaling = true;

		m_fInitialDistance = controllerDistance();
		m_vec3InitialDimensions = m_pDataVolume->getDimensions();
	}

	if (!m_pTDM->getSecondaryController()->isGripButtonPressed() || !m_pTDM->getPrimaryController()->isGripButtonPressed())
		m_bScaling = false;
}

float ScaleDataVolumeBehavior::controllerDistance()
{
	return glm::length(m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3] - m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3]);
}
