#include "StudyTutorialBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

#include "WelcomeBehavior.h"
#include "StudyIntroBehavior.h"
#include "GrabTutorial.h"
#include "ScaleTutorial.h"
#include "StudyEditTutorial.h"

StudyTutorialBehavior::StudyTutorialBehavior(TrackedDeviceManager* pTDM, DataVolume* tableVolume, DataVolume* wallVolume)
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


StudyTutorialBehavior::~StudyTutorialBehavior()
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

void StudyTutorialBehavior::update()
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

void StudyTutorialBehavior::draw()
{
}

void StudyTutorialBehavior::createDemoQueue()
{
	m_qTutorialQueue.push(std::make_pair("Welcome", new WelcomeBehavior(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Intro", new StudyIntroBehavior(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Grab Tut", new GrabTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Scale Tut", new ScaleTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Edit Tut", new StudyEditTutorial(m_pTDM)));
}