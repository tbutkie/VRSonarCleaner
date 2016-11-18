#pragma once
#include <vector>
#include <algorithm>

#include "Observer.h"

class TrackedDevice;

class Subject
{
public:
	void attach(Observer *obs)
	{
		observers.push_back(obs);
	}

	void detach(Observer *obs)
	{
		observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
	}

protected:
	virtual void notify(TrackedDevice* device, const int event)
	{
		for (auto obs : observers) obs->receiveEvent(device, event);
	}
	
private:
	std::vector<Observer *> observers;
};
