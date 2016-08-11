#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include <vector>
#include "CloudCollection.h"

//#include <SDL_opengl.h>
//#include <gl/glu.h>
#include <stdio.h>
//#include <string>
//#include <cstdlib>

//#include <openvr.h>

//#include "../shared/lodepng.h"
#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

extern CloudCollection *clouds;

class CleaningRoom
{
public:
	CleaningRoom();
	virtual ~CleaningRoom();

	float cylTest(const Vector4 & pt1, const Vector4 & pt2, float lengthsq, float radius_sq, const Vector3 & testpt);

	void draw();

	void setRoomSize(float RoomSizeX, float RoomSizeY, float RoomSizeZ);

	void recalcVolumeBounds();

	bool checkCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, unsigned int sensitivity);
	bool gripCleaningTable(const Matrix4 *controllerPose);

private:
	float roomSizeX, roomSizeY, roomSizeZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	HolodeckBackground* holodeck;

	DataVolume *wallVolume;
	DataVolume *tableVolume;
};