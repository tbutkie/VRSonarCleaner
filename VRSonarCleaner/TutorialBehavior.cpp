#include "TutorialBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

#include "WelcomeBehavior.h"
#include "TutorialIntroduction.h"

TutorialBehavior::TutorialBehavior(TrackedDeviceManager* pTDM, DataVolume* tableVolume, DataVolume* wallVolume)
	: m_pTDM(pTDM)
	, m_pTableVolume(tableVolume)
	, m_pWallVolume(wallVolume)
{
	createTutorialQueue();
	tableVolume->setVisible(false);
	wallVolume->setVisible(false);
	m_qTutorialQueue.front()->init();
}


TutorialBehavior::~TutorialBehavior()
{
}

void TutorialBehavior::update()
{
	if (m_qTutorialQueue.size() == 0u)
		return;

	m_qTutorialQueue.front()->update();

	if (!m_qTutorialQueue.front()->isActive())
	{
		delete m_qTutorialQueue.front();
		m_qTutorialQueue.pop();

		if (m_qTutorialQueue.size() > 0u)
			m_qTutorialQueue.front()->init();
	}
}

void TutorialBehavior::draw()
{
	if (m_qTutorialQueue.size() > 0u)
		m_qTutorialQueue.front()->draw();
}

void TutorialBehavior::createTutorialQueue()
{
	m_qTutorialQueue.push(new WelcomeBehavior(m_pTDM));
	m_qTutorialQueue.push(new TutorialIntroduction(m_pTDM, m_pTableVolume));

	//

	/*


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

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));*/
}