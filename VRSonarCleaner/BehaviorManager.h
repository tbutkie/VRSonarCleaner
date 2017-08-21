#pragma once

#include "BehaviorBase.h"
#include <map>

class BehaviorManager
{
public:	
	static BehaviorManager& getInstance()
	{
		static BehaviorManager s_instance;
		return s_instance;
	}

	void init();

	void update();
	void draw();

private:
	BehaviorManager();
	~BehaviorManager();

	bool addBehavior(std::string name, BehaviorBase* pBehavior);
	BehaviorBase* getBehavior(std::string name);
	bool removeBehavior(std::string name);

	std::map<std::string, BehaviorBase*> m_mappBehaviors;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	BehaviorManager(BehaviorManager const&) = delete;
	void operator=(BehaviorManager const&) = delete;
};

