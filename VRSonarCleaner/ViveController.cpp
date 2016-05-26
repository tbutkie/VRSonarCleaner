#include "ViveController.h"

ViveController::ViveController()
{
	posX = 0;
	posY = 0;
	posZ = 0;
}

ViveController::~ViveController()
{

}

void ViveController::setLocation(float x, float y, float z)
{
	posX = x;
	posY = y;
	posZ = z;
}

void ViveController::draw()
{
	glColor3f(0.0, 1.0, 0.0);
	glPointSize(5.0);
	glBegin(GL_POINTS);
		glVertex3f(posX, posY, posZ);
	glEnd();
}