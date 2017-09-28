#include "BehaviorManager.h"

BehaviorManager::BehaviorManager()
{
}


BehaviorManager::~BehaviorManager()
{
	for (auto &b : m_mappBehaviors)
	{
		delete b.second;
		b.second = NULL;
	}

	m_mappBehaviors.clear();
}

void BehaviorManager::init()
{
}

void BehaviorManager::addBehavior(std::string name, BehaviorBase * pBehavior)
{
	std::map<std::string, BehaviorBase*>::iterator it = m_mappBehaviors.find(name);
	if (it != m_mappBehaviors.end())
	{
		printf("%s: Replacing existing behavior \"%s\" with new\n", __FUNCTION__, name);
		delete it->second;
		it->second = NULL;
	}
	else
		printf("%s: Adding behavior \"%s\"\n", __FUNCTION__, name.c_str());

	m_mappBehaviors[name] = pBehavior;
}

BehaviorBase * BehaviorManager::getBehavior(std::string name)
{
	std::map<std::string, BehaviorBase*>::iterator it = m_mappBehaviors.find(name);
	if (it == m_mappBehaviors.end())
		return nullptr;
	else
		return it->second;
}

bool BehaviorManager::removeBehavior(std::string name)
{
	std::map<std::string, BehaviorBase*>::iterator it = m_mappBehaviors.find(name);
	if (it == m_mappBehaviors.end())
	{
		printf("%s: Tried to remove behavior \"%s\" which doesn't exist!\n", __FUNCTION__, name);
		return false;
	}
	else
	{
		delete it->second;
		it->second = NULL;
		m_mappBehaviors.erase(it);
		return true;
	}
}

void BehaviorManager::update()
{
	for (auto &b : m_mappBehaviors)
		b.second->update();
}

void BehaviorManager::draw()
{
	for (auto &b : m_mappBehaviors)
		b.second->draw();
}

void BehaviorManager::shutdown()
{
	for (auto &b : m_mappBehaviors)
		delete b.second;

	m_mappBehaviors.clear();
}
