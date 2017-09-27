#pragma once
#include "DualControllerBehavior.h"

#include "DataVolume.h"

class SelectAreaBehavior :
	public DualControllerBehavior
{
public:
	SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* dataVolume);
	~SelectAreaBehavior();

	void update();

	void draw();

private:
	DataVolume* m_pDataVolume;

	glm::vec3 m_vec3MinBound;
	glm::vec3 m_vec3MaxBound;

	bool m_bSelectingArea;
	float m_fDistanceToPlane;
	glm::vec3 m_vec3LocationOnPlane;

	glm::vec3 m_vec3CursorSize;

private:
	void receiveEvent(const int event, void* payloadData);
};

