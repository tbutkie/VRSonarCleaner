#pragma once

class TrackedDevice;

class Observer
{
public:
	virtual ~Observer() {}
	virtual void update(TrackedDevice* device, const int event) = 0;

public:
	virtual enum NOTIFICATION {
		EVENT_EDIT_TRIGGER_PRESSED
	};
};
