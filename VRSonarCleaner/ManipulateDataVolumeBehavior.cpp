#include "ManipulateDataVolumeBehavior.h"

#include "DebugDrawer.h"

ManipulateDataVolumeBehavior::ManipulateDataVolumeBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume)
	: DualControllerBehavior(gripController, scaleController)
	, m_pGripController(gripController)
	, m_pScaleController(scaleController)
	, m_pDataVolume(dataVolume)
	, m_bGripping(false)
	, m_bScaling(false)
	, m_bRotationInProgress(false)
{
	scaleController->attach(this);
}


ManipulateDataVolumeBehavior::~ManipulateDataVolumeBehavior()
{
}

void ManipulateDataVolumeBehavior::update()
{
	if (m_bScaling)
	{
		float currentDist = controllerDistance();
		float delta = currentDist - m_fInitialDistance;
		m_pDataVolume->setSize(glm::vec3(exp(delta * 10.f) * m_vec3InitialScale));
	}
	else if (m_bGripping)
		continueRotation();
}

void ManipulateDataVolumeBehavior::draw()
{
	// Draw Axes
	if (false)
	{
		DebugDrawer::getInstance().setTransform(m_pPrimaryController->getDeviceToWorldTransform());
		DebugDrawer::getInstance().drawTransform(0.1f);
	}

	// Draw Touchpad line
	if (m_pPrimaryController->isTouchpadTouched())
	{

		glm::vec4 start = glm::vec4(m_pPrimaryController->getInitialTouchpadTouchPoint(), 1.f);
		glm::vec4 startColor(.9f, .2f, .1f, 0.75f);
		glm::vec4 end = glm::vec4(m_pPrimaryController->getCurrentTouchpadTouchPoint(), 1.f);
		glm::vec4 endColor(.1f, .2f, .9f, 0.75f);

		DebugDrawer::getInstance().setTransform(m_pPrimaryController->getDeviceToWorldTransform());
		DebugDrawer::getInstance().drawLine(glm::vec3(start), glm::vec3(end), startColor, endColor);
	}
}

void ManipulateDataVolumeBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		BroadcastSystem::Payload::Trigger payload;
		memcpy(&payload, payloadData, sizeof(BroadcastSystem::Payload::Trigger));
		if (payload.m_pSelf == m_pGripController)
		{
			startRotation();
			m_bGripping = true;
		}
		else
		{
			if (m_bGripping)
			{
				m_bGripping = false;
				m_bScaling = true;

				m_fInitialDistance = controllerDistance();
				m_vec3InitialScale = m_pDataVolume->getSize();
			}
		}
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		BroadcastSystem::Payload::Trigger payload;
		memcpy(&payload, payloadData, sizeof(BroadcastSystem::Payload::Trigger));
		if (payload.m_pSelf == m_pGripController)
		{
			endRotation();
			m_bGripping = false;
			m_bScaling = false;
		}
		else
		{
			if (m_bScaling)
			{
				m_bScaling = false;
				startRotation();
				m_bGripping = true;
			}
		}
		break;
	}
	default:
		break;
	}
}

float ManipulateDataVolumeBehavior::controllerDistance()
{
	return glm::length(m_pGripController->getDeviceToWorldTransform()[3] - m_pScaleController->getDeviceToWorldTransform()[3]);
}


void ManipulateDataVolumeBehavior::startRotation()
{
	m_mat4ControllerPoseAtRotationStart = m_pPrimaryController->getDeviceToWorldTransform();
	//m_mat4PoseAtRotationStart = glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4_cast(m_qOrientation);
	m_mat4DataVolumePoseAtRotationStart = m_pDataVolume->getCurrentVolumeTransform();

	//save volume pose in controller space
	m_mat4ControllerToVolumePose = glm::inverse(m_mat4ControllerPoseAtRotationStart) * m_mat4DataVolumePoseAtRotationStart;

	m_bRotationInProgress = true;
}

void ManipulateDataVolumeBehavior::continueRotation()
{
	if (!m_bRotationInProgress)
		return;

	glm::mat4 mat4ControllerPoseCurrent = m_pPrimaryController->getDeviceToWorldTransform();

	m_pDataVolume->setPosition(glm::vec3((mat4ControllerPoseCurrent * m_mat4ControllerToVolumePose)[3]));
	m_pDataVolume->setOrientation(mat4ControllerPoseCurrent * m_mat4ControllerToVolumePose);

	DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawLine(
		glm::vec3(m_mat4ControllerPoseAtRotationStart[3]),
		glm::vec3(m_mat4DataVolumePoseAtRotationStart[3]),
		glm::vec4(0.f, 1.f, 0.f, 1.f)
	);
	DebugDrawer::getInstance().drawLine(
		glm::vec3(mat4ControllerPoseCurrent[3]), 
		glm::vec3(m_pDataVolume->getCurrentVolumeTransform()[3]),
		glm::vec4(1.f, 0.f, 0.f, 1.f)
	);
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