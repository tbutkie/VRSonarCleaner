#pragma once

#include <map>
#include <thread>

enum ResourceStatus {
	RESOURCE_NOT_FOUND = 0,
	RESOURCE_LOADING,
	RESOURCE_LOADED
};

class ResourceManager
{
	ResourceManager();
	~ResourceManager();

public:	
	// Singleton instance access
	static ResourceManager& getInstance()
	{
		static ResourceManager s_instance;
		return s_instance;
	}
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	ResourceManager(ResourceManager const&) = delete;
	void operator=(ResourceManager const&) = delete;
	
	bool init();

	void shutdown();
};

