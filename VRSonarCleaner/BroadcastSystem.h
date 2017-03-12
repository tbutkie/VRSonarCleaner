#pragma once

#include <vector>
#include <algorithm>

class TrackedDevice;

namespace BroadcastSystem
{
	enum EVENT {
		MOUSE_CLICK,
		MOUSE_UNCLICK,
		MOUSE_MOVE,
		MOUSE_SCROLL,
		KEY_PRESS,
		KEY_UNPRESS,
		KEY_REPEAT,
		EXIT_PLAY_AREA,
		ENTER_PLAY_AREA,

		VIVE_SYSTEM_BUTTON_DOWN, // System button pressed
		VIVE_SYSTEM_BUTTON_UP,	 // System button unpressed
		VIVE_MENU_BUTTON_DOWN,	 // Menu button pressed
		VIVE_MENU_BUTTON_UP,	 // Menu button unpressed
		VIVE_GRIP_DOWN,			 // Grip button pressed
		VIVE_GRIP_UP,			 // Grip button unpressed
		VIVE_TRIGGER_ENGAGE,	 // Trigger analog initial engagement (float = pullAmount)
		VIVE_TRIGGER_PULL,		 // Trigger analog continued engagement (float = pullAmount)
		VIVE_TRIGGER_DISENGAGE,	 // Trigger analog disengagement
		VIVE_TRIGGER_DOWN,		 // Trigger button pressed (clicked)
		VIVE_TRIGGER_UP,		 // Trigger button unpressed (unclicked) (float = pullAmount)
		VIVE_TOUCHPAD_ENGAGE,	 // Touchpad initial touch (vec2 = initialTouchPoint)
		VIVE_TOUCHPAD_TOUCH,	 // Touchpad continued touch (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_DISENGAGE, // Touchpad untouch (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_DOWN,		 // Touchpad pressed (clicked) (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_UP		 // Touchpad unpressed (unclicked) (vec2[2] = { initialTouchPoint, lastTouchPoint })
	};



	enum VIVE_ACTION {
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void receiveEvent(TrackedDevice* device, const int event, void* data) = 0;
	};

	class Broadcaster
	{
	public:
		void attach(Listener *obs)
		{
			m_vpListeners.push_back(obs);
		}

		void detach(Listener *obs)
		{
			m_vpListeners.erase(std::remove(m_vpListeners.begin(), m_vpListeners.end(), obs), m_vpListeners.end());
		}

	protected:
		virtual void notify(TrackedDevice* device, const int event, void* data = NULL)
		{
			for (auto obs : m_vpListeners) obs->receiveEvent(device, event, data);
		}

	protected:
		std::vector<Listener *> m_vpListeners;
	};
 }