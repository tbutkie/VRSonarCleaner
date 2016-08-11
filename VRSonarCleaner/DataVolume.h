#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "Vec3.h"
#include "MatrixUtils.h"
#include "Quaternion.h"
//#include <string>
//#include <cstdlib>

class DataVolume
{
public:
	DataVolume(float PosX, float PosY, float PosZ, int startingOrientation, float SizeX, float SizeY, float SizeZ);
	virtual ~DataVolume();

	void drawBBox();
	void drawBacking();
	void drawAxes();

	void setSize(float SizeX, float SizeY, float SizeZ);
	void setPosition(float PosX, float PosY, float PosZ);
	void setInnerCoords(double MinX, double MaxX, double MinY, double MaxY, double MinZ, double MaxZ);


	void recalcScaling();

	void convertToInnerCoords(float xWorld, float yWorld, float zWorld, float *xInner, float *yInner, float *zInner);
	void convertToWorldCoords(float xInner, float yInner, float zInner, float *xWorld, float *yWorld, float *zWorld);

	void activateTransformationMatrix();

	void startRotation(double *mat4x4);
	void continueRotation(double *mat4x4);
	void endRotation();
	bool isBeingRotated();	

private:

	float sizeX, sizeY, sizeZ;
	float posX, posY, posZ;
	//float minX, minY, minZ;
	//float maxX, maxY, maxZ;

	//int orientation; //0 z-up, 1 z-wall-worldy
	Quaternion *orientation;

	double innerMinX, innerMaxX, innerMinY, innerMaxY, innerMinZ, innerMaxZ;
	double innerSizeX, innerSizeY, innerSizeZ;

	double storedMatrix[16];
	float XZscale, depthScale;

	//rotate action
	bool rotationInProgress;
	Quaternion *rotStart;
	Quaternion *rotCurrent;
	Quaternion *rotLast;
	Quaternion *rotLastInverse;
	double rotMatStart[16];
	double rotMatCurrent[16];
	double rotMatLast[16];

};