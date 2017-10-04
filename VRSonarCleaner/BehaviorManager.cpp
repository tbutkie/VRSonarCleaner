#include "BehaviorManager.h"

#include <iostream>
#include <string>

BehaviorManager::BehaviorManager()
	: m_bClearing(false)
{
}


BehaviorManager::~BehaviorManager()
{
	clearBehaviors();
}

void BehaviorManager::init()
{
}

void BehaviorManager::addBehavior(std::string name, BehaviorBase * pBehavior)
{
	std::map<std::string, BehaviorBase*>::iterator it = m_mappBehaviors.find(name);
	if (it != m_mappBehaviors.end())
	{
		printf("%s: Replacing existing behavior \"%s\" with new\n", __FUNCTION__, name.c_str());
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
		if (!m_bClearing)
			std::cout << __FUNCTION__ << ": Tried to remove behavior \"" << name << "\" which doesn't exist!\n";
		return false;
	}
	else
	{
		std::cout << __FUNCTION__ << ": Removing behavior \"" << name << "\"\n";
		m_mappBehaviors.erase(name);
		delete toDelete;
		return true;
	}
}

void BehaviorManager::clearBehaviors()
{
	m_bClearing = true;
	auto mapCopy = m_mappBehaviors;
	while (m_mappBehaviors.size() > 0u)
		removeBehavior(m_mappBehaviors.begin()->first);
	m_bClearing = false;
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
	clearBehaviors();
}
