#pragma once

class Scene
{
public:
	Scene() {}
	virtual ~Scene() {}

	virtual void init() = 0;

	virtual void update() = 0;

	virtual void draw() = 0;
};