#include "ManipulateDataVolumeBehavior.h"

#include "Renderer.h"

ManipulateDataVolumeBehavior::ManipulateDataVolumeBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume)
	: m_pGripController(gripController)
	, m_pScaleController(scaleController)
	, m_pDataVolume(dataVolume)
	, m_bPreGripping(false)
	, m_bGripping(false)
	, m_bScaling(false)
	, m_bRotationInProgress(false)
{
}


ManipulateDataVolumeBehavior::~ManipulateDataVolumeBehavior()
{
}



void ManipulateDataVolumeBehavior::update()
{
	updateState();

	if (m_bScaling)
	{
		float currentDist = controllerDistance();
		float delta = currentDist - m_fInitialDistance;
		m_pDataVolume->setDimensions(glm::vec3(exp(delta * 10.f) * m_vec3InitialDimensions));
	}

	if (m_bPreGripping)
	{
		preRotation(m_pGripController->getTriggerPullAmount());
	}
	
	if (m_bGripping)
	{
		continueRotation();
	}
}

void ManipulateDataVolumeBehavior::draw()
{
}

void ManipulateDataVolumeBehavior::updateState()
{
	if (m_pGripController->isTriggerEngaged())
		m_bPreGripping = true;		
	else
		m_bPreGripping = false;


	if (m_pGripController->justClickedTrigger())
	{
		startRotation();
		m_bGripping = true;
	}

	if (m_pGripController->justUnclickedTrigger())
	{
		endRotation();
		m_bGripping = false;
		m_bScaling = false;
	}
	
	if ((m_pGripController->justPressedGrip() && m_pScaleController->isGripButtonPressed()) ||
		(m_pScaleController->justPressedGrip() && m_pGripController->isGripButtonPressed()))
	{
		m_bScaling = true;

		m_fInitialDistance = controllerDistance();
		m_vec3InitialDimensions = m_pDataVolume->getDimensions();
	}

	if (!m_pGripController->isGripButtonPressed() || !m_pScaleController->isGripButtonPressed())
		m_bScaling = false;
}

float ManipulateDataVolumeBehavior::controllerDistance()
{
	return glm::length(m_pGripController->getDeviceToWorldTransform()[3] - m_pScaleController->getDeviceToWorldTransform()[3]);
}


void ManipulateDataVolumeBehavior::startRotation()
{
	m_mat4ControllerPoseAtRotationStart = m_pGripController->getDeviceToWorldTransform();
	//m_mat4PoseAtRotationStart = glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4_cast(m_qOrientation);
	m_mat4DataVolumePoseAtRotationStart = glm::translate(glm::mat4(), m_pDataVolume->getPosition()) * glm::mat4_cast(m_pDataVolume->getOrientation());

	//save volume pose in controller space
	m_mat4ControllerToVolumeTransform = glm::inverse(m_mat4ControllerPoseAtRotationStart) * m_mat4DataVolumePoseAtRotationStart;

	m_bRotationInProgress = true;
}

void ManipulateDataVolumeBehavior::continueRotation()
{
	if (!m_bRotationInProgress)
		return;

	glm::mat4 mat4ControllerPoseCurrent = m_pGripController->getDeviceToWorldTransform();

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

void ManipulateDataVolumeBehavior::endRotation()
{
	m_bRotationInProgress = false;
	//could revert to old starting position and orientation here to have it always snap back in place
}

bool ManipulateDataVolumeBehavior::isBeingRotated()
{
	return m_bRotationInProgress;
}

void ManipulateDataVolumeBehavior::preRotation(float ratio)
{
	float cylThickness = 0.01f * (1.f - ratio);

	glm::mat4 mat4ControllerPoseCurrent = m_pGripController->getDeviceToWorldTransform();
	
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
