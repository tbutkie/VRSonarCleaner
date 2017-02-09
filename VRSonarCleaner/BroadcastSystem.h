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
		EDIT_TRIGGER_CLICKED,
		EXIT_PLAY_AREA,
		ENTER_PLAY_AREA
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
		virtual void attach(Listener *obs)
		{
			m_vpListeners.push_back(obs);
		}

		virtual void detach(Listener *obs)
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