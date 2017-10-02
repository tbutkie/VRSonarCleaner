#include "TutorialBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

TutorialBehavior::TutorialBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume)
	: DualControllerBehavior(gripController, scaleController)
	, m_pDataVolume(dataVolume)
	, m_bPreGripping(false)
	, m_bGripping(false)
	, m_bScaling(false)
	, m_bRotationInProgress(false)
{
	createTutorialQueue();
	std::get<0>(m_qTutorialQueue.front())();
}


TutorialBehavior::~TutorialBehavior()
{
}

void TutorialBehavior::update()
{
	if (m_qTutorialQueue.size() > 0u && !std::get<1>(m_qTutorialQueue.front())())
	{
		std::get<2>(m_qTutorialQueue.front())();
		m_qTutorialQueue.pop();

		if (m_qTutorialQueue.size() > 0u)
			std::get<0>(m_qTutorialQueue.front())();
	}
}

void TutorialBehavior::draw()
{
}

void TutorialBehavior::createTutorialQueue()
{
	InitFunc tf = [&]() {
		InfoBoxManager::getInstance().addInfoBox(
			"Activate Label (Primary)",
			"activaterightlabel.png",
			0.075f,
			glm::translate(glm::mat4(), glm::vec3(-0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
			InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
			false);
		InfoBoxManager::getInstance().addInfoBox(
			"Activate Label (Secondary)",
			"activateleftlabel.png",
			0.075f,
			glm::translate(glm::mat4(), glm::vec3(0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
			InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
			false);
	};

	UpdateFunc uf = [&]() -> bool {
		if (m_pPrimaryController->isTriggerClicked() && m_pSecondaryController->isTriggerClicked())
			return false;
		else
			return true;
	};

	CleanupFunc cf = [&]() {
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Primary)");
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Secondary)");
	};

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));

	tf = [&]() {
		InfoBoxManager::getInstance().addInfoBox(
			"Test 1",
			"cube_texture.png",
			1.f,
			glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -2.f)),
			InfoBoxManager::RELATIVE_TO::HMD,
			false);
	};

	uf = [&]() -> bool {
		if (m_pPrimaryController->isTriggerClicked() && m_pSecondaryController->isTriggerClicked())
			return false;
		else
			return true;
	};

	cf = [&]() {
		InfoBoxManager::getInstance().removeInfoBox("Test 1");
	};

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));

	tf = [&]() {
		InfoBoxManager::getInstance().addInfoBox(
			"Editing Label",
			"editctrlrlabel.png",
			0.1f,
			glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
			InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
			false);
		InfoBoxManager::getInstance().addInfoBox(
			"Manipulation Label",
			"manipctrlrlabel.png",
			0.1f,
			glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
			InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
			false);
	};

	uf = [&]() -> bool {
		if (m_pPrimaryController->isTriggerClicked() && m_pSecondaryController->isTriggerClicked())
			return false;
		else
			return true;
	};

	cf = [&]() {
		InfoBoxManager::getInstance().removeInfoBox("Test 1");
	};

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));

	tf = [&]() {
		InfoBoxManager::getInstance().addInfoBox(
			"Test 2",
			"test.png",
			1.f,
			glm::translate(glm::mat4(), glm::vec3(1.f, 2.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)),
			InfoBoxManager::RELATIVE_TO::WORLD,
			true);
	};

	uf = [&]() -> bool {
		if (m_pPrimaryController->isTriggerClicked() && m_pSecondaryController->isTriggerClicked())
			return false;
		else
			return true;
	};

	cf = [&]() {
		InfoBoxManager::getInstance().removeInfoBox("Test 2");
	};

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));

	tf = [&]() {
		InfoBoxManager::getInstance().addInfoBox(
			"Test 3",
			"test.png",
			2.f,
			glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -1.f)),
			InfoBoxManager::RELATIVE_TO::HMD,
			false);
	};

	uf = [&]() -> bool {
		if (m_pPrimaryController->isTriggerClicked() && m_pSecondaryController->isTriggerClicked())
			return false;
		else
			return true;
	};

	cf = [&]() {
		InfoBoxManager::getInstance().removeInfoBox("Test 3");
	};

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));
}

float TutorialBehavior::controllerDistance()
{
	return glm::length(m_pPrimaryController->getDeviceToWorldTransform()[3] - m_pSecondaryController->getDeviceToWorldTransform()[3]);
}


void TutorialBehavior::startRotation()
{
	m_mat4ControllerPoseAtRotationStart = m_pPrimaryController->getDeviceToWorldTransform();
	//m_mat4PoseAtRotationStart = glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4_cast(m_qOrientation);
	m_mat4DataVolumePoseAtRotationStart = glm::translate(glm::mat4(), m_pDataVolume->getPosition()) * glm::mat4_cast(m_pDataVolume->getOrientation());

	//save volume pose in controller space
	m_mat4ControllerToVolumeTransform = glm::inverse(m_mat4ControllerPoseAtRotationStart) * m_mat4DataVolumePoseAtRotationStart;

	m_bRotationInProgress = true;
}

void TutorialBehavior::continueRotation()
{
	if (!m_bRotationInProgress)
		return;

	glm::mat4 mat4ControllerPoseCurrent = m_pPrimaryController->getDeviceToWorldTransform();

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

void TutorialBehavior::endRotation()
{
	m_bRotationInProgress = false;
	//could revert to old starting position and orientation here to have it always snap back in place
}

bool TutorialBehavior::isBeingRotated()
{
	return m_bRotationInProgress;
}

void TutorialBehavior::preRotation(float ratio)
{
	float cylThickness = 0.01f * (1.f - ratio);

	glm::mat4 mat4ControllerPoseCurrent = m_pPrimaryController->getDeviceToWorldTransform();
	
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
