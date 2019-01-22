#include "GrabObjectBehavior.h"
#include "InfoBoxManager.h"
#include "Renderer.h"
#include "DataLogger.h"

GrabObjectBehavior::GrabObjectBehavior(TrackedDeviceManager* pTDM, Object3D* object)
	: m_pTDM(pTDM)
	, m_pObject(object)
	, m_bPreGripping(false)
	, m_bGripping(false)
	, m_bRotationInProgress(false)
{
}


GrabObjectBehavior::~GrabObjectBehavior()
{
}



void GrabObjectBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	updateState();
	
	if (m_bGripping)
	{
		continueRotation();
	}
}

void GrabObjectBehavior::draw()
{
	if (m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->valid())
	{
		glm::mat4 triggerTextAnchorTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.025f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		Renderer::getInstance().drawText(
			"Grab",
			glm::mix(glm::vec4(1.f), glm::vec4(1.f, 1.f, 0.f, 1.f), m_pTDM->getSecondaryController()->getTriggerPullAmount()),
			triggerTextAnchorTrans[3],
			glm::quat(triggerTextAnchorTrans),
			0.0075f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_LEFT
		);

		Renderer::getInstance().drawDirectedPrimitive("cylinder",
			triggerTextAnchorTrans[3],
			(m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.03f, 0.05f)))[3],
			0.001f,
			glm::vec4(1.f, 1.f, 1.f, 0.75f)
		);

		if (m_bPreGripping)
		{
			float cylThickness = 0.02f * (1.f - m_pTDM->getSecondaryController()->getTriggerPullAmount());

			glm::vec3 controllerToVolumeVec = m_pObject->getPosition() - glm::vec3(m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3]);
			glm::vec3 start = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::vec3 end = start + controllerToVolumeVec * m_pTDM->getSecondaryController()->getTriggerPullAmount();

			Renderer::getInstance().drawDirectedPrimitive("cylinder", start, end, cylThickness, glm::mix(glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(1.f, 1.f, 1.f, 0.25f), m_pTDM->getSecondaryController()->getTriggerPullAmount()));
		}

		if (m_bGripping)
		{
			Renderer::getInstance().drawDirectedPrimitive("cylinder", m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3], m_pObject->getPosition(), 0.001f, glm::vec4(1.f, 1.f, 0.f, 0.25f));
			Renderer::getInstance().drawDirectedPrimitive("cylinder", m_mat4ControllerPoseAtRotationStart[3], m_mat4DataVolumePoseAtRotationStart[3], 0.001f, glm::vec4(1.f, 1.f, 1.f, 0.25f));
		}
	}
}

void GrabObjectBehavior::updateState()
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

void GrabObjectBehavior::startRotation()
{
	if (!m_pTDM->getSecondaryController())
		return;

	m_mat4ControllerPoseAtRotationStart = m_pTDM->getSecondaryController()->getDeviceToWorldTransform();
	//m_mat4PoseAtRotationStart = glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4_cast(m_qOrientation);
	m_mat4DataVolumePoseAtRotationStart = glm::translate(glm::mat4(), m_pObject->getPosition()) * glm::mat4_cast(m_pObject->getOrientation());

	//save volume pose in controller space
	m_mat4ControllerToVolumeTransform = glm::inverse(m_mat4ControllerPoseAtRotationStart) * m_mat4DataVolumePoseAtRotationStart;

	m_bRotationInProgress = true;

	if (DataLogger::getInstance().logging())
	{
		glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
		glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());

		std::stringstream ss;

		ss << "Manipulation Begin" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pObject->getPosition().x << "," << m_pObject->getPosition().y << "," << m_pObject->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pObject->getOrientation().x << "," << m_pObject->getOrientation().y << "," << m_pObject->getOrientation().z << "," << m_pObject->getOrientation().w << "\"";
		ss << ";";
		ss << "vol-dims:\"" << m_pObject->getDimensions().x << "," << m_pObject->getDimensions().y << "," << m_pObject->getDimensions().z << "\"";
		ss << ";";
		ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
		ss << ";";
		ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";

		if (m_pTDM->getPrimaryController())
		{
			glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
			glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			ss << ";";
			ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		}

		if (m_pTDM->getSecondaryController())
		{
			glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			ss << ";";
			ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		}

		DataLogger::getInstance().logMessage(ss.str());
	}
}

void GrabObjectBehavior::continueRotation()
{
	if (!m_pTDM->getSecondaryController() || !m_bRotationInProgress)
		return;

	glm::mat4 mat4ControllerPoseCurrent = m_pTDM->getSecondaryController()->getDeviceToWorldTransform();

	glm::mat4 newVolTrans = mat4ControllerPoseCurrent * m_mat4ControllerToVolumeTransform;
	glm::vec3 newVolPos((mat4ControllerPoseCurrent * m_mat4ControllerToVolumeTransform)[3]);

	m_pObject->setPosition(newVolPos);
	m_pObject->setOrientation(glm::quat_cast(newVolTrans));
}

void GrabObjectBehavior::endRotation()
{
	m_bRotationInProgress = false;
	//could revert to old starting position and orientation here to have it always snap back in place

	if (DataLogger::getInstance().logging())
	{
		glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
		glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());

		std::stringstream ss;

		ss << "Manipulation End" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pObject->getPosition().x << "," << m_pObject->getPosition().y << "," << m_pObject->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pObject->getOrientation().x << "," << m_pObject->getOrientation().y << "," << m_pObject->getOrientation().z << "," << m_pObject->getOrientation().w << "\"";
		ss << ";";
		ss << "vol-dims:\"" << m_pObject->getDimensions().x << "," << m_pObject->getDimensions().y << "," << m_pObject->getDimensions().z << "\"";
		ss << ";";
		ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
		ss << ";";
		ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";

		if (m_pTDM->getPrimaryController())
		{
			glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
			glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			ss << ";";
			ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		}

		if (m_pTDM->getSecondaryController())
		{
			glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			ss << ";";
			ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		}

		DataLogger::getInstance().logMessage(ss.str());
	}
}

bool GrabObjectBehavior::isBeingRotated()
{
	return m_bRotationInProgress;
}
