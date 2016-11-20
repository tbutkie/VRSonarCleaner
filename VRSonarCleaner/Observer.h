#pragma once

class TrackedDevice;

class Observer
{
public:
	virtual ~Observer() {}
	virtual void receiveEvent(TrackedDevice* device, const int event) = 0;

public:
	virtual enum EVENT {
		EDIT_TRIGGER_CLICKED,
		OUT_OF_PLAY_AREA,
		INSIDE_PLAY_AREA
	};
};
