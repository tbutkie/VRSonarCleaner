#include "GrabDataVolumeBehavior.h"
#include "InfoBoxManager.h"
#include "Renderer.h"

GrabDataVolumeBehavior::GrabDataVolumeBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_pDataVolume(dataVolume)
	, m_bPreGripping(false)
	, m_bGripping(false)
	, m_bRotationInProgress(false)
{
	InfoBoxManager::getInstance().addInfoBox(
		"Grab Label Controller",
		"manipctrlrlabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Grab Label Trigger",
		"grableftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.05f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);
}


GrabDataVolumeBehavior::~GrabDataVolumeBehavior()
{
	InfoBoxManager::getInstance().removeInfoBox("Grab Label Controller");
	InfoBoxManager::getInstance().removeInfoBox("Grab Label Trigger");
}



void GrabDataVolumeBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	updateState();

	if (m_bPreGripping)
	{
		preRotation(m_pTDM->getSecondaryController()->getTriggerPullAmount());
	}
	
	if (m_bGripping)
	{
		continueRotation();
	}

	m_pDataVolume->update();
}

void GrabDataVolumeBehavior::draw()
{
}

void GrabDataVolumeBehavior::updateState()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if (m_pTDM->getSecondaryController()->isTriggerEngaged())
		m_bPreGripping = true;		
	else
		m_bPreGripping = false;


	if (m_pTDM->getSecondaryController()->justClickedTrigger())
	{
		startRotation();
		m_bGripping = true;
	}

	if (m_pTDM->getSecondaryController()->justUnclickedTrigger())
	{
		endRotation();
		m_bGripping = false;
	}
}

void GrabDataVolumeBehavior::startRotation()
{
	if (!m_pTDM->getSecondaryController())
		return;

	m_mat4ControllerPoseAtRotationStart = m_pTDM->getSecondaryController()->getDeviceToWorldTransform();
	//m_mat4PoseAtRotationStart = glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4_cast(m_qOrientation);
	m_mat4DataVolumePoseAtRotationStart = glm::translate(glm::mat4(), m_pDataVolume->getPosition()) * glm::mat4_cast(m_pDataVolume->getOrientation());

	//save volume pose in controller space
	m_mat4ControllerToVolumeTransform = glm::inverse(m_mat4ControllerPoseAtRotationStart) * m_mat4DataVolumePoseAtRotationStart;

	m_bRotationInProgress = true;
}

void GrabDataVolumeBehavior::continueRotation()
{
	if (!m_pTDM->getSecondaryController() || !m_bRotationInProgress)
		return;

	glm::mat4 mat4ControllerPoseCurrent = m_pTDM->getSecondaryController()->getDeviceToWorldTransform();

	glm::mat4 newVolTrans = mat4ControllerPoseCurrent * m_mat4ControllerToVolumeTransform;
	glm::vec3 newVolPos((mat4ControllerPoseCurrent * m_mat4ControllerToVolumeTransform)[3]);

	m_pDataVolume->setPosition(newVolPos);
	m_pDataVolume->setOrientation(glm::quat_cast(newVolTrans));

	float cylThickness = 0.001f;

	glm::vec4 wStart = m_mat4DataVolumePoseAtRotationStart[3] - m_mat4ControllerPoseAtRotationStart[3];
	glm::vec4 uStart = glm::vec4(glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(glm::vec3(wStart))), 0.f);
	glm::vec4 vStart = glm::vec4(glm::cross(glm::normalize(glm::vec3(wStart)), glm::vec3(uStart)), 0.f);

	glm::mat4 transStart;
	transStart[0] = uStart * cylThickness;
	transStart[1] = vStart * cylThickness;
	transStart[2] = wStart;
	transStart[3] = m_mat4ControllerPoseAtRotationStart[3];

	Renderer::getInstance().drawPrimitive("cylinder", transStart, glm::vec4(0.f, 1.f, 0.f, 0.5f), glm::vec4(1.f), 32.f);

	glm::vec4 wCurrent = glm::vec4(newVolPos, 1.f) - mat4ControllerPoseCurrent[3];
	glm::vec4 uCurrent = glm::vec4(glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(glm::vec3(wCurrent))), 0.f);
	glm::vec4 vCurrent = glm::vec4(glm::cross(glm::normalize(glm::vec3(wCurrent)), glm::vec3(uCurrent)), 0.f);

	glm::mat4 transCurrent;
	transCurrent[0] = uCurrent * cylThickness;
	transCurrent[1] = vCurrent * cylThickness;
	transCurrent[2] = wCurrent;
	transCurrent[3] = mat4ControllerPoseCurrent[3];

	Renderer::getInstance().drawPrimitive("cylinder", transCurrent, glm::vec4(1.f, 0.f, 0.f, 0.5f), glm::vec4(1.f), 32.f);
}

void GrabDataVolumeBehavior::endRotation()
{
	m_bRotationInProgress = false;
	//could revert to old starting position and orientation here to have it always snap back in place
}

bool GrabDataVolumeBehavior::isBeingRotated()
{
	return m_bRotationInProgress;
}

void GrabDataVolumeBehavior::preRotation(float ratio)
{
	float cylThickness = 0.01f * (1.f - ratio);

	glm::mat4 mat4ControllerPoseCurrent = m_pTDM->getSecondaryController()->getDeviceToWorldTransform();
	
	glm::vec4 wCurrent = glm::vec4(m_pDataVolume->getPosition(), 1.f) - mat4ControllerPoseCurrent[3];
	glm::vec4 uCurrent = glm::vec4(glm::cross(glm::vec3(0.f, 1.f, 0.f), glm::normalize(glm::vec3(wCurrent))), 0.f);
	glm::vec4 vCurrent = glm::vec4(glm::cross(glm::normalize(glm::vec3(wCurrent)), glm::vec3(uCurrent)), 0.f);

	glm::mat4 transCurrent;
	transCurrent[0] = uCurrent * cylThickness;
	transCurrent[1] = vCurrent * cylThickness;
	transCurrent[2] = wCurrent * ratio;
	transCurrent[3] = mat4ControllerPoseCurrent[3];

	Renderer::getInstance().drawPrimitive("cylinder", transCurrent, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(1.f), 32.f);
}
