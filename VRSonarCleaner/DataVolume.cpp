#include "DataVolume.h"
#include "DebugDrawer.h"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(float PosX, float PosY, float PosZ, int startingOrientation, float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	pos = glm::vec3(PosX, PosY, PosZ);

	//minX = PosX - (sizeX / 2);
	//minY = PosY - (sizeY / 2);
	//minZ = PosZ - (sizeZ / 2);

	//maxX = PosX + (sizeX / 2);
	//maxY = PosY + (sizeY / 2);
	//maxZ = PosZ + (sizeZ / 2);


	//orientation = new Quaternion();
	

	if (startingOrientation == 0)
		orientation = glm::angleAxis(0.f, glm::vec3(0, 0, 0));
	else
		orientation = glm::angleAxis(-90.f, glm::vec3(1, 0, 0));

	rotationInProgress = false;

	
	originalPosition[0] = pos.x;
	originalPosition[1] = pos.y;
	originalPosition[2] = pos.z;
	originalOrientation = orientation;

	//orientationAtRotationStart = new Quaternion();
	//controllerOrientationAtRotationStart = new Quaternion();
	//controllerOrientationCurrent = new Quaternion();
	//controllerRotationNeeded = new Quaternion();

}

DataVolume::~DataVolume()
{

}

void DataVolume::resetPositionAndOrientation()
{
	pos.x = originalPosition[0];
	pos.y = originalPosition[1];
	pos.z = originalPosition[2];
	orientation = originalOrientation;
}


void DataVolume::setSize(float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	//minX = posX - (sizeX / 2);
	//minY = posY - (sizeY / 2);
	//minZ = posZ - (sizeZ / 2);

	//maxX = posX + (sizeX / 2);
	//maxY = posY + (sizeY / 2);
	//maxZ = posZ + (sizeZ / 2);

	recalcScaling();
}

void DataVolume::setPosition(float PosX, float PosY, float PosZ)
{
	pos.x = PosX;
	pos.y = PosY;
	pos.z = PosZ;
	//minX = posX - (sizeX / 2);
	//minY = posY - (sizeY / 2);
	//minZ = posZ - (sizeZ / 2);

	//maxX = posX + (sizeX / 2);
	//maxY = posY + (sizeY / 2);
	//maxZ = posZ + (sizeZ / 2);

	recalcScaling();
}

void DataVolume::setInnerCoords(double MinX, double MaxX, double MinY, double MaxY, double MinZ, double MaxZ)
{
	innerMinX = MinX;
	innerMaxX = MaxX;
	innerMinY = MinY;
	innerMaxY = MaxY;
	innerMinZ = MinZ;
	innerMaxZ = MaxZ;
	innerSizeX = MaxX - MinX;
	innerSizeY = MaxY - MinY;
	innerSizeZ = MaxZ - MinZ;

	recalcScaling();
}

void DataVolume::recalcScaling()
{
	XZscale = (float)std::min((double)sizeX / innerSizeX, (double)sizeZ / innerSizeZ);
	depthScale = sizeY / (float)innerSizeY;
	printf("Scaling Factor is XZ: %f, Y: %f", XZscale, depthScale);
	
	/*
	if (orientation == 0) //z-up table
	{
		XZscale = (float)min((double)sizeX / innerSizeX, (double)sizeZ / innerSizeZ);
		//float newWidth = innerSizeX*XYscale;
		//float newHeight = innerSizeY*XYscale;

		depthScale = sizeY / (float)innerSizeY;
	}
	else if (orientation == 1) //rotated wall view
	{
		XZscale = (float)min((double)sizeX / innerSizeX, (double)sizeY / innerSizeZ);
		
		depthScale = sizeZ / (float)innerSizeY;
	}

	printf("Scaling Factor for Orientation %d is XZ: %f, Y: %f", orientation, XZscale, depthScale);
	*/

}//end recalc scaling

void DataVolume::convertToInnerCoords(float xWorld, float yWorld, float zWorld, float *xInner, float *yInner, float *zInner)
{
	double inverted[16];
	InvertMat(storedMatrix, inverted);
	
	//Vec3 pointIn(xWorld, yWorld, zWorld);
	double pointIn[3];
	double pointOut[3];

	pointIn[0] = xWorld;
	pointIn[1] = yWorld;
	pointIn[2] = zWorld;

	MVMult(inverted, pointIn, pointOut, true);

	*xInner = pointOut[0];
	*yInner = pointOut[1];
	*zInner = pointOut[2];
}

void DataVolume::convertToWorldCoords(float xInner, float yInner, float zInner, float *xWorld, float *yWorld, float *zWorld)
{
	double pointIn[3];
	double pointOut[3];

	pointIn[0] = xInner;
	pointIn[1] = yInner;
	pointIn[2] = zInner;

	MVMult(storedMatrix, pointIn, pointOut, true);

	*xWorld = pointOut[0];
	*yWorld = pointOut[1];
	*zWorld = pointOut[2];
}

void DataVolume::activateTransformationMatrix()
{
	//store current matrix so we can restore it later in deactivateTransformationMatrix(), this isn't working right now for some reason
	glMatrixMode(GL_MODELVIEW);
	//glGetDoublev(GL_MODELVIEW, storedMatrix);

	glTranslatef(pos.x, pos.y, pos.z);

	//old if

	//rotate here
	
	glMultMatrixf(glm::value_ptr(glm::mat4_cast(orientation)));
	//scale/trans

	glScalef(XZscale, depthScale, XZscale);
	//glTranslatef((-((float)innerSizeX) / 2) - innerMinX, (((float)innerSizeY) / 2) + innerMaxY, (-((float)innerSizeZ) / 2) - innerMinZ); //why y different? to flip depth?
	glTranslatef((-((float)innerSizeX) / 2) - innerMinX, -(((float)innerSizeY) / 2) - innerMinY, (-((float)innerSizeZ) / 2) - innerMinZ); //tried this and it seems fine for now

	glMatrixMode(GL_MODELVIEW);
	glGetDoublev(GL_MODELVIEW_MATRIX, storedMatrix);

	/*
	if (orientation == 0) //table view 
	{
		//glScalef(-1.0, 1.0, 1.0);
		//glRotatef(180.0, 0.0, 0.0, 1.0);
		//glRotatef(90.0, 1.0, 0.0, 0.0);
		glScalef(XZscale, depthScale, XZscale);
		glTranslatef((-((float)innerSizeX) / 2)-innerMinX, (((float)innerSizeY) / 2) + innerMaxY, (-((float)innerSizeZ) / 2) - innerMinZ);
		
		glMatrixMode(GL_MODELVIEW);
		glGetDoublev(GL_MODELVIEW_MATRIX, storedMatrix);
	}
	if (orientation == 1) //wall view
	{
		//glScalef(-1.0, 1.0, 1.0);
		//glRotatef(180.0, 0.0, 0.0, 1.0);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		//glScalef(XZscale, XZscale, depthScale);
		glScalef(XZscale, depthScale, XZscale);
		//glTranslatef(-((float)innerSizeX) / 2, ((float)innerSizeY) / 2, -((float)innerSizeZ) / 2);
		glTranslatef((-((float)innerSizeX) / 2) - innerMinX, (((float)innerSizeY) / 2) + innerMaxY, (-((float)innerSizeZ) / 2) - innerMinZ);
	}
	*/

}




void DataVolume::drawBBox()
{
	//printf("In Holodeck Draw()\n");
	//glColor3f(0.75, 0.0, 0.0);
	glColor4f(0.22, 0.25, 0.34, 1.0);
	glLineWidth(1.0);
	
	float BBox[6];
	BBox[0] = pos.x-(sizeX / 2);
	BBox[1] = pos.x+(sizeX / 2);
	BBox[2] = pos.y-(sizeY / 2);
	BBox[3] = pos.y+(sizeY / 2);
	BBox[4] = pos.z-(sizeZ / 2);
	BBox[5] = pos.z+(sizeZ / 2);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glMultMatrixf(glm::value_ptr(glm::mat4_cast(orientation)));
	glTranslatef(-pos.x, -pos.y, -pos.z);
	
	glBegin(GL_LINES);

	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[5]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[4]);



	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);


	glVertex3f(BBox[0], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);


	glVertex3f(BBox[1], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[1], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glEnd();

	glPopMatrix();

	/*

	float BBox[6];
	BBox[0] = minX;
	BBox[1] = maxX;
	BBox[2] = minY;
	BBox[3] = maxY;
	BBox[4] = minZ;
	BBox[5] = maxZ;


	glBegin(GL_LINES);

	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[5]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[4]);



	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);


	glVertex3f(BBox[0], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);


	glVertex3f(BBox[1], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[1], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glEnd();

	*/



}

void DataVolume::drawBacking()
{
	//printf("In Holodeck Draw()\n");
	glColor4f(0.22, 0.25, 0.34, 1.0);

	float BBox[6];
	BBox[0] = pos.x-(sizeX / 2);
	BBox[1] = pos.x+(sizeX / 2);
	BBox[2] = pos.y -(sizeY / 2);
	BBox[3] = pos.y+(sizeY / 2);
	BBox[4] = pos.z-(sizeZ / 2);
	BBox[5] = pos.z+(sizeZ / 2);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glMultMatrixf(glm::value_ptr(glm::mat4_cast(orientation)));                 
	glTranslatef(-pos.x, -pos.y, -pos.z);

	glBegin(GL_QUADS);

		glVertex3f(BBox[0], BBox[2], BBox[4]);
		glVertex3f(BBox[1], BBox[2], BBox[4]);
		glVertex3f(BBox[1], BBox[2], BBox[5]);
		glVertex3f(BBox[0], BBox[2], BBox[5]);
	
	glEnd();

	glPopMatrix();
}

void DataVolume::drawAxes()
{
	//printf("In Holodeck Draw()\n");
	//glColor3f(0.75, 0.0, 0.0);
	//glLineWidth(1.0);

	//float smallSize = sizeX / 40;
	//float bigSize = sizeX / 20;

	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glTranslatef(posX, posY, posZ);
	//glMultMatrixf(glm::value_ptr(glm::mat4_cast(orientation)));
	//glTranslatef(-posX, -posY, -posZ);

	//glLineWidth(2.0);
	//glBegin(GL_LINES);
	//glColor3f(1.0, 0.0, 0.0);
	//glVertex3f(posX-smallSize, posY, posZ);
	//glVertex3f(posX+bigSize, posY, posZ);

	//glColor3f(0.0, 1.0, 0.0);
	//glVertex3f(posX, posY -smallSize, posZ);
	//glVertex3f(posX, posY+bigSize, posZ);

	//glColor3f(0.0, 0.0, 1.0);
	//glVertex3f(posX, posY, posZ -smallSize);
	//glVertex3f(posX, posY, posZ+bigSize);
	//glEnd();

	//glPopMatrix();

	DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), glm::vec3(pos.x, pos.y, pos.z)) * glm::mat4_cast(orientation));
	DebugDrawer::getInstance().drawTransform(0.1f);
}

void DataVolume::startRotation(const float *mat4Controller)
{
	//save volume's starting position and orientation 
	orientationAtRotationStart = orientation;
	positionAtRotationStart = pos;

	//save controllers starting position and orientation
	controllerPositionAtRotationStart = glm::vec3(glm::make_mat4(mat4Controller)[3]);	
	controllerOrientationAtRotationStart = glm::quat_cast(glm::make_mat4(mat4Controller));
	
	//save orientation difference and positional offset
	quatControllerToVolumeDifference = controllerOrientationAtRotationStart * glm::inverse(orientationAtRotationStart);
	vectorControllerToVolume = glm::inverse(glm::mat3_cast(controllerOrientationAtRotationStart)) * (positionAtRotationStart - controllerPositionAtRotationStart); // model space

	// in controller space
	controllerToVolume = glm::inverse(glm::make_mat4(mat4Controller)) * (glm::translate(glm::mat4(), pos) * glm::mat4_cast(orientation));

	rotationInProgress = true;

	DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawLine(controllerPositionAtRotationStart, controllerPositionAtRotationStart + vectorControllerToVolume, glm::vec3(0.f, 1.f, 0.f));
}

void DataVolume::continueRotation(const float *mat4Controller)
{
	if (!rotationInProgress)
		return;

	glm::quat controllerOrientationCurrent = glm::quat_cast(glm::make_mat4(mat4Controller));
	glm::vec3 controllerPositionCurrent = glm::vec3(glm::make_mat4(mat4Controller)[3]);	

	glm::vec3 vectorNewControllerToVolume = glm::mat3_cast(controllerOrientationCurrent) * vectorControllerToVolume;

	pos = controllerPositionCurrent + vectorNewControllerToVolume;
	orientation = quatControllerToVolumeDifference * controllerOrientationCurrent;

	//pos = glm::vec3(glm::inverse(controllerToVolume) * glm::make_mat4(mat4Controller) * glm::vec4(0.f, 0.f, 0.f, 1.f));
	//orientation = glm::quat_cast(controllerToVolume * glm::make_mat4(mat4Controller));

	DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawLine(controllerPositionAtRotationStart, controllerPositionAtRotationStart + glm::mat3_cast(controllerOrientationAtRotationStart) * vectorControllerToVolume, glm::vec3(0.f, 1.f, 0.f));
	DebugDrawer::getInstance().drawLine(controllerPositionCurrent, controllerPositionCurrent + vectorNewControllerToVolume, glm::vec3(1.f, 0.f, 0.f));
}

void DataVolume::endRotation()
{
	rotationInProgress = false;

	//could revert to old starting position and orientation here to have it always snap back in place
}

bool DataVolume::isBeingRotated()
{
	return rotationInProgress;
}