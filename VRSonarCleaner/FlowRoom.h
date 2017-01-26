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

class FlowRoom
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

private:
	float roomSizeX, roomSizeY, roomSizeZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	HolodeckBackground* holodeck;

	CoordinateScaler *scaler;

	float flowRoomTime;
	float flowRoomMinTime, flowRoomMaxTime;
	ULONGLONG lastTimeUpdate;

	DataVolume *mainModelVolume;
		
	std::vector <FlowGrid *> *flowGridCollection;
			
	IllustrativeParticleSystem *particleSystem;

	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fPtHighlightAmt;
};