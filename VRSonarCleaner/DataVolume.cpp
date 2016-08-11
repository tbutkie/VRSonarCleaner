#include "DataVolume.h"

#include <iostream>

DataVolume::DataVolume(float PosX, float PosY, float PosZ, int startingOrientation, float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	posX = PosX;
	posY = PosY;
	posZ = PosZ;

	//minX = PosX - (sizeX / 2);
	//minY = PosY - (sizeY / 2);
	//minZ = PosZ - (sizeZ / 2);

	//maxX = PosX + (sizeX / 2);
	//maxY = PosY + (sizeY / 2);
	//maxZ = PosZ + (sizeZ / 2);


	//orientation = new Quaternion();
	

	if (startingOrientation == 0)
		orientation.createFromAxisAngle(0, 0, 0, 0);
	else
		orientation.createFromAxisAngle(1, 0, 0, -90);

	rotationInProgress = false;

	
	//orientationAtRotationStart = new Quaternion();
	//controllerOrientationAtRotationStart = new Quaternion();
	//controllerOrientationCurrent = new Quaternion();
	//controllerRotationNeeded = new Quaternion();

}

DataVolume::~DataVolume()
{

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
	posX = PosX;
	posY = PosY;
	posZ = PosZ;
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
	XZscale = (float)min((double)sizeX / innerSizeX, (double)sizeZ / innerSizeZ);
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

	glTranslatef(posX, posY, posZ);

	//old if

	//rotate here
	
	orientation.applyOpenGLRotationMatrix();
	
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
	BBox[0] = posX-(sizeX / 2);
	BBox[1] = posX+(sizeX / 2);
	BBox[2] = posY-(sizeY / 2);
	BBox[3] = posY+(sizeY / 2);
	BBox[4] = posZ-(sizeZ / 2);
	BBox[5] = posZ+(sizeZ / 2);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(posX, posY, posZ);
	orientation.applyOpenGLRotationMatrix();
	glTranslatef(-posX, -posY, -posZ);
	
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
	BBox[0] = posX-(sizeX / 2);
	BBox[1] = posX+(sizeX / 2);
	BBox[2] = posY -(sizeY / 2);
	BBox[3] = posY+(sizeY / 2);
	BBox[4] = posZ-(sizeZ / 2);
	BBox[5] = posZ+(sizeZ / 2);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(posX, posY, posZ);
	orientation.applyOpenGLRotationMatrix();
	glTranslatef(-posX, -posY, -posZ);

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
	glColor3f(0.75, 0.0, 0.0);
	glLineWidth(1.0);

	float smallSize = sizeX / 40;
	float bigSize = sizeX / 20;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(posX, posY, posZ);
	orientation.applyOpenGLRotationMatrix();
	glTranslatef(-posX, -posY, -posZ);

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(posX-smallSize, posY, posZ);
	glVertex3f(posX+bigSize, posY, posZ);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(posX, posY -smallSize, posZ);
	glVertex3f(posX, posY+bigSize, posZ);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(posX, posY, posZ -smallSize);
	glVertex3f(posX, posY, posZ+bigSize);
	glEnd();

	glPopMatrix();

	/*
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(posX - smallSize, 0, 0);
	glVertex3f(posX + bigSize, 0, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, posX - smallSize, 0);
	glVertex3f(0, posX + bigSize, 0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, posX - smallSize);
	glVertex3f(0, 0, posX + bigSize);
	glEnd();
	*/
}

void DataVolume::startRotation(double *mat4x4)
{
	//orientation.printValues("orientation");

	//save volume's starting position and orientation 
	orientationAtRotationStart.copy(orientation);
	positionAtRotationStart[0] = posX;
	positionAtRotationStart[1] = posY;
	positionAtRotationStart[2] = posZ;

	//orientationAtRotationStart.printValues("orientationAtRotationStart");

	//save controllers starting position and orientation
	controllerPositionAtRotationStart[0] = mat4x4[12];
	controllerPositionAtRotationStart[1] = mat4x4[13];
	controllerPositionAtRotationStart[2] = mat4x4[14];
	//printf("Extracted controller starting position: %f, %f, %f\n", controllerPositionAtRotationStart[0], controllerPositionAtRotationStart[1], controllerPositionAtRotationStart[2]);
	controllerOrientationAtRotationStart.createFromOpenGLMatrix(mat4x4);

	//controllerOrientationAtRotationStart.printValues("controllerOrientationAtRotationStart");

	controllerOrientationAtRotationStartInverted.copy(controllerOrientationAtRotationStart);
	controllerOrientationAtRotationStartInverted.invert();

	//controllerOrientationAtRotationStartInverted.printValues("controllerOrientationAtRotationStartInverted");

	//save positional offset
	vectorControllerToVolume[0] = positionAtRotationStart[0] - controllerPositionAtRotationStart[0];
	vectorControllerToVolume[1] = positionAtRotationStart[1] - controllerPositionAtRotationStart[1];
	vectorControllerToVolume[2] = positionAtRotationStart[2] - controllerPositionAtRotationStart[2];

	rotationInProgress = true;
}

void DataVolume::continueRotation(double *mat4x4)
{
	if (!rotationInProgress)
		return;

	controllerOrientationCurrent.createFromOpenGLMatrix(mat4x4);
	
	//controllerOrientationAtRotationStartInverted.printValues("controllerOrientationAtRotationStartInverted");
	//controllerOrientationCurrent.printValues("controllerOrientationCurrent");

	//controllerRotationNeeded = controllerOrientationAtRotationStartInverted * controllerOrientationCurrent;
	controllerRotationNeeded.createByMultiplyingTwoQuaternions(controllerOrientationAtRotationStartInverted, controllerOrientationCurrent);
	
	//controllerRotationNeeded.printValues("controllerRotationNeeded");

	double tempMat[16];
	IdentityMat(tempMat);
	tempMat[12] = vectorControllerToVolume[0];
	tempMat[13] = vectorControllerToVolume[1];
	tempMat[14] = vectorControllerToVolume[2];
	double tempRotMat[16];
	controllerRotationNeeded.createOpenGLRotationMatrix(tempRotMat);
	double tempOutMat[16];
	MMult(tempMat, tempRotMat, tempOutMat);
	posX = tempOutMat[12] + mat4x4[12];
	posY = tempOutMat[13] + mat4x4[13];
	posZ = tempOutMat[14] + mat4x4[14];
	
	//orientation.createFromOpenGLMatrix(tempOutMat);

	//orientation.printValues("orientation");

	//orientation = orientationAtRotationStart * controllerRotationNeeded;
	orientation.createByMultiplyingTwoQuaternions(orientationAtRotationStart, controllerRotationNeeded);
	
	//orientation.printValues("orientation");
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