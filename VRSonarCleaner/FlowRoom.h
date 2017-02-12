#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "CoordinateScaler.h"
#include <vector>

#include <stdio.h>
#include <algorithm>
#include "FlowGrid.h"

#include "BroadcastSystem.h"

#include <shared/glm/glm.hpp>

class FlowRoom : public BroadcastSystem::Listener
{
public:
	FlowRoom();
	virtual ~FlowRoom();

	void draw();

	void setRoomSize(float RoomSizeX, float RoomSizeY, float RoomSizeZ);

	void recalcVolumeBounds();

	void preRenderUpdates();

	//bool checkCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, unsigned int sensitivity);
	bool probeModel(const glm::mat4 & currentCursorPose, const glm::mat4 & lastCursorPose, float radius, bool clearPoints);
	bool gripModel(const glm::mat4 &controllerPose);
	void releaseModel();

	void reset();

	virtual void receiveEvent(TrackedDevice* device, const int event, void* data);

private:
	glm::vec3 m_vec3RoomSize;
	glm::vec3 m_vec3Min, m_vec3Max;
	
	HolodeckBackground *m_pHolodeck;

	CoordinateScaler *m_pScaler;

	float m_fFlowRoomTime;
	float m_fFlowRoomMinTime, m_fFlowRoomMaxTime;
	ULONGLONG m_ullLastTimeUpdate;

	DataVolume *m_pMainModelVolume;
		
	std::vector<FlowGrid*> m_vpFlowGridCollection;
			
	IllustrativeParticleSystem *m_pParticleSystem;
};