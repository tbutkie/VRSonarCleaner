#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "Vec3.h"
#include "MatrixUtils.h"
#include "../shared/glm/gtc/quaternion.hpp"
#include "../shared/glm/gtx/quaternion.hpp"
#include "../shared//glm/gtc/type_ptr.hpp"
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

	void startRotation(const float *mat4Controller);
	void continueRotation(const float *mat4Controller);
	void endRotation();
	bool isBeingRotated();	

	void resetPositionAndOrientation();

private:

	float sizeX, sizeY, sizeZ;
	glm::vec3 pos;
	//float minX, minY, minZ;
	//float maxX, maxY, maxZ;

	glm::quat orientation;
	glm::quat originalOrientation;
	float originalPosition[3];
	
	double innerMinX, innerMaxX, innerMinY, innerMaxY, innerMinZ, innerMaxZ;
	double innerSizeX, innerSizeY, innerSizeZ;

	double storedMatrix[16];
	float XZscale, depthScale;

	//rotate action
	bool rotationInProgress;
	glm::quat orientationAtRotationStart;
	glm::vec3 positionAtRotationStart;
	glm::quat controllerOrientationAtRotationStart;
	glm::vec3 controllerPositionAtRotationStart;
	glm::quat quatControllerToVolumeDifference;
	glm::mat4 mat4ControllerToVolumeDifference;
	glm::vec3 vectorControllerToVolume;
	glm::quat controllerOrientationLast;
	glm::vec3 controllerPositionLast;
	glm::mat4 controllerToVolume;
};