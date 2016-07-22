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

	bool checkCleaningTable(Vector3 last, Vector3 current, float radius);

private:
	float roomSizeX, roomSizeY, roomSizeZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	HolodeckBackground* holodeck;

	DataVolume *wallVolume;
	DataVolume *tableVolume;


};