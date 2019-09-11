#include "StudyTutorialHMDBehavior.h"

#include "InfoBoxManager.h"
#include "BehaviorManager.h"
#include "Renderer.h"

#include "WelcomeBehavior.h"
#include "StudyIntroBehavior.h"
#include "GrabTutorial.h"
#include "ScaleTutorial.h"
#include "StudyEditTutorial.h"

StudyTutorialHMDBehavior::StudyTutorialHMDBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{
	createDemoQueue();
	BehaviorManager::getInstance().addBehavior(m_qTutorialQueue.front().first, m_qTutorialQueue.front().second);
	m_qTutorialQueue.front().second->init();
}


StudyTutorialHMDBehavior::~StudyTutorialHMDBehavior()
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

void StudyTutorialHMDBehavior::update()
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

void StudyTutorialHMDBehavior::draw()
{
}

void StudyTutorialHMDBehavior::createDemoQueue()
{
	m_qTutorialQueue.push(std::make_pair("Welcome", new WelcomeBehavior(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Intro", new StudyIntroBehavior(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Grab Tut", new GrabTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Scale Tut", new ScaleTutorial(m_pTDM)));
	m_qTutorialQueue.push(std::make_pair("Edit Tut", new StudyEditTutorial(m_pTDM)));
}