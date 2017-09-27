#pragma once
#include "DualControllerBehavior.h"

#include "DataVolume.h"

class SelectAreaBehavior :
	public DualControllerBehavior
{
public:
	SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* selectionVolume, DataVolume* displayVolume);
	~SelectAreaBehavior();

	void update();

	void draw();

private:
	DataVolume* m_pDataVolumeSelection;
	DataVolume* m_pDataVolumeDisplay;

	glm::dvec3 m_dvec3MinBound;
	glm::dvec3 m_dvec3MaxBound;

	bool m_bShowCursor;
	bool m_bSelectingArea;
	glm::vec3 m_vec3StartLocationOnPlane;
	glm::vec3 m_vec3CurrentLocationOnPlane;
	
	glm::vec3 m_vec3CursorSize;

private:
	void receiveEvent(const int event, void* payloadData);

	bool castRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planeOrigin, glm::vec3 planeNormal, glm::vec3* locationOnPlane);
};

