#pragma once
#include "BehaviorBase.h"
#include "ViveController.h"
#include "DataVolume.h"

class SelectAreaBehavior :
	public BehaviorBase
{
public:
	SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* selectionVolume, DataVolume* displayVolume);
	virtual ~SelectAreaBehavior();

	void update();

	void draw();

	void reset();

private:
	ViveController *m_pPrimaryController;
	ViveController *m_pSecondaryController;
	DataVolume* m_pDataVolumeSelection;
	DataVolume* m_pDataVolumeDisplay;

	bool m_bActive; // has the pointer been activated by the user?
	bool m_bShowCursor; // show the cursor?
	bool m_bSelectingArea; // is an area currently being selected?
	bool m_bMovingArea; // is the custom area currently being moved around?
	bool m_bCustomAreaSet; // has a custom domain been set?
	bool m_bRayHitPlane;
	bool m_bRayHitDomain;
	bool m_bRayHitCustomDomain;

	glm::vec3 m_vec3CurrentLocationOnPlane; // world pos of cursor on plane
	glm::vec3 m_vec3LastSelectedLocationOnPlaneWithinDomain;

	glm::dvec3 m_dvec3MinBound; // raw data coordinated for min boundary
	glm::dvec3 m_dvec3MaxBound; // raw data coordinated for max boundary

	glm::vec3 m_vec3BeginBoundOnPlane; // world pos of bbox start on selection plane
	glm::vec3 m_vec3EndBoundOnPlane; // world pos of current bbox bound on selection plane

	glm::vec3 m_vec3BeginDragOnPlane; // world pos of cursor on selection plane when dragging action started
	glm::dvec3 m_dvec3MinBoundAtDragStart;
	glm::dvec3 m_dvec3MaxBoundAtDragStart;
	
	glm::vec3 m_vec3CursorSize;

private:
	void updateState();
	glm::vec3 calcHits();
	bool castRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planeOrigin, glm::vec3 planeNormal, glm::vec3* locationOnPlane);
};

