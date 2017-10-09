#include "DemoBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

#include "WelcomeBehavior.h"
#include "GrabTutorial.h"
#include "ScaleTutorial.h"
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
	m_qTutorialQueue.push(std::make_pair("Grab Tut", new GrabTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Scale Tut", new ScaleTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Edit Tut", new CloudEditControllerTutorial(m_pTDM)));
}