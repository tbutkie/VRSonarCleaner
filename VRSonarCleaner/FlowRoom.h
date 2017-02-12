#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "CoordinateScaler.h"
#include <vector>

//#include <SDL_opengl.h>
//#include <gl/glu.h>
#include <stdio.h>
//#include <string>
//#include <cstdlib>
#include <chrono>
#include <algorithm>
#include "FlowGrid.h"
//#include <openvr.h>

//#include "../shared/lodepng.h"
#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

#include "BroadcastSystem.h"

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
	bool probeModel(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, bool clearPoints);
	bool gripModel(const Matrix4 *controllerPose);

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

	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fPtHighlightAmt;
};