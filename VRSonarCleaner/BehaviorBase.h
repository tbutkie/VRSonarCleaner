#pragma once

class BehaviorBase
{
public:
	BehaviorBase() {};
	virtual ~BehaviorBase() {};

	virtual void update() = 0;

	virtual void draw() = 0;
};

