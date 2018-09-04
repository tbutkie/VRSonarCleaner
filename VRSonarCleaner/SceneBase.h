#pragma once

#include <SDL.h>

class Scene
{
public:
	Scene() {}
	virtual ~Scene() {}

	virtual void init() = 0;

	virtual void processSDLEvent(SDL_Event &ev) = 0;

	virtual void update() = 0;

	virtual void draw() = 0;
};