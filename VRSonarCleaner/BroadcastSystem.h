#pragma once

#include <vector>
#include <algorithm>

#include <shared/glm/glm.hpp>

class TrackedDevice;
class ViveController;

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

		VIVE_CONTROLLER_ATTACHED, // System button pressed
		VIVE_CONTROLLER_DETACHED, // System button pressed

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
		VIVE_TOUCHPAD_ENGAGE,	 // Touchpad initial touch (vec2[2] = initialTouchPoint, initialTouchpoint)
		VIVE_TOUCHPAD_TOUCH,	 // Touchpad continued touch (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_DISENGAGE, // Touchpad untouch (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_DOWN,		 // Touchpad pressed (clicked) (vec2[2] = { initialTouchPoint, lastTouchPoint })
		VIVE_TOUCHPAD_UP		 // Touchpad unpressed (unclicked) (vec2[2] = { initialTouchPoint, lastTouchPoint })
	};

	class Payload {
	public:
		struct HMD {
			TrackedDevice* m_pSelf;
			glm::mat4 m_mat4Pose;
		};

		struct Controller {
			ViveController* m_pSelf;
		};

		struct Trigger {
			ViveController* m_pSelf;
			float m_fPullAmount;
		};

		struct Touchpad {
			ViveController* m_pSelf;
			glm::vec2 m_vec2InitialTouch;
			glm::vec2 m_vec2CurrentTouch;
		};
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void receiveEvent(const int event, void* payloadData) = 0;
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
		virtual void notify(const int event, void* payloadData = NULL)
		{
			for (auto obs : m_vpListeners) obs->receiveEvent(event, payloadData);
		}

	protected:
		std::vector<Listener *> m_vpListeners;
	};
 }