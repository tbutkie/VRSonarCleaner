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
	BehaviorBase * toDelete = getBehavior(name);

	if (toDelete == NULL)
	{
		printf("%s: Tried to remove behavior \"%s\" which doesn't exist!\n", __FUNCTION__, name);
		return false;
	}
	else
	{
		printf("%s: Removing behavior \"%s\"\n", __FUNCTION__, name.c_str());
		delete toDelete;
		m_mappBehaviors.erase(name);
		return true;
	}
}

void BehaviorManager::clearBehaviors()
{
	auto mapCopy = m_mappBehaviors;
	for (auto &b : mapCopy)
		removeBehavior(b.first);
	m_mappBehaviors.clear();
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
