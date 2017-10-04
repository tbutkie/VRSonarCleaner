#include "DemoBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

#include "WelcomeBehavior.h"
#include "CloudEditControllerTutorial.h"

DemoBehavior::DemoBehavior(TrackedDeviceManager* pTDM, DataVolume* tableVolume, DataVolume* wallVolume)
	: m_pTDM(pTDM)
	, m_pTableVolume(tableVolume)
	, m_pWallVolume(wallVolume)
{
	createDemoQueue();
	tableVolume->setVisible(false);
	wallVolume->setVisible(false);
	BehaviorManager::getInstance().addBehavior(m_qTutorialQueue.front().first, m_qTutorialQueue.front().second);
	m_qTutorialQueue.front().second->init();
}


DemoBehavior::~DemoBehavior()
{
	if (m_qTutorialQueue.size() > 0u)
	{
		BehaviorManager::getInstance().removeBehavior(m_qTutorialQueue.front().first);
		m_qTutorialQueue.pop();
	}

	while (m_qTutorialQueue.size() > 0u)
	{
		delete m_qTutorialQueue.front().second;
		m_qTutorialQueue.pop();
	}
}

void DemoBehavior::update()
{
	if (m_qTutorialQueue.size() == 0u)
		return;

	if (!m_qTutorialQueue.front().second->isActive())
	{
		BehaviorManager::getInstance().removeBehavior(m_qTutorialQueue.front().first);
		m_qTutorialQueue.pop();

		if (m_qTutorialQueue.size() > 0u)
		{
			BehaviorManager::getInstance().addBehavior(m_qTutorialQueue.front().first, m_qTutorialQueue.front().second);
			m_qTutorialQueue.front().second->init();
		}
	}
	
}

void DemoBehavior::draw()
{
}

void DemoBehavior::createDemoQueue()
{
	m_qTutorialQueue.push(std::make_pair("Welcome", new WelcomeBehavior(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Edit Tut", new CloudEditControllerTutorial(m_pTDM)));

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

	m_qTutorialQueue.push(TutorialEntry(tf, uf, cf));
	*/
}