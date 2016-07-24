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

	void draw();

	void setRoomSize(float RoomSizeX, float RoomSizeY, float RoomSizeZ);

	bool checkCleaningTable(Vector3 cursorCtr, Vector3 forwardRadiusPos, Vector3 cylAxisEndPos);

	float cylTest(const Vector3 & pt1, const Vector3 & pt2, float lengthsq, float radius_sq, const Vector3 & testpt);

private:
	float roomSizeX, roomSizeY, roomSizeZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	HolodeckBackground* holodeck;

	DataVolume *wallVolume;
	DataVolume *tableVolume;


};