#pragma once

#include <SDL.h>

class BehaviorBase
{
public:
	BehaviorBase() : m_bActive(true) {}
	virtual ~BehaviorBase() {}

	virtual void update() = 0;

	virtual void draw() = 0;

	bool isActive() { return m_bActive; }

protected:
	bool m_bActive;
};

class InitializableBehavior 
	: public BehaviorBase
{
public:
	InitializableBehavior() : m_bInitialized(false) {}
	virtual ~InitializableBehavior() {}

	virtual void init() = 0;

	bool isInitialized() { return m_bInitialized; }

protected:
	bool m_bInitialized;
};

class InitializableEventBehavior
	: public InitializableBehavior
{
public:
	InitializableEventBehavior() : m_bInitialized(false) {}
	virtual ~InitializableEventBehavior() {}

	virtual void init() = 0;

	virtual void processEvent(SDL_Event &ev) = 0;

	bool isInitialized() { return m_bInitialized; }

protected:
	bool m_bInitialized;
};

