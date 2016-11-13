#pragma once
#include <vector>
#include <algorithm>

#include "Observer.h"

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
	void notify()
	{
		for (auto obs : observers) obs->update();
	}
	
private:
	std::vector<Observer *> observers;
};
