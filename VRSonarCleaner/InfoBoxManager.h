#pragma once

#include <openvr.h>

#include "shared/Texture.h"
#include "Observer.h"


class InfoBoxManager : public Observer
{
public:
	// Singleton instance access
	static InfoBoxManager& getInstance()
	{
		static InfoBoxManager instance;
		return instance;
	}

	virtual void update()
	{

	}

private:
	InfoBoxManager();
	~InfoBoxManager();
		
// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	InfoBoxManager(InfoBoxManager const&) = delete;
	void operator=(InfoBoxManager const&) = delete;
};

