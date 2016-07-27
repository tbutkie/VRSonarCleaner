#pragma once

//#include <SDL.h>
#include <GL/glew.h>
//#include <math.h>
//#include <SDL_opengl.h>
//#include <gl/glu.h>
//#include <stdio.h>
//#include <string>
//#include <cstdlib>

//#include <openvr.h>

//#include "../shared/lodepng.h"
//#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

#include "TrackedDevice.h"

class ViveController
{
public:
	ViveController();
	virtual ~ViveController();
	
	void draw();

	void setLocation(float x, float y, float z);

private:
	float posX, posY, posZ;
	


};