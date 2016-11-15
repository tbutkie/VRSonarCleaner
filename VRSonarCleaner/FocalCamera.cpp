
#include "FocalCamera.h"


// Multiply a vector by a matrix
void MVMult(float *M, float *V, float *Vout)
{
	Vout[0] = M[0] * V[0] + M[4] * V[1] + M[8] * V[2];
	Vout[1] = M[1] * V[0] + M[5] * V[1] + M[9] * V[2];
	Vout[2] = M[2] * V[0] + M[6] * V[1] + M[10] * V[2];
	Vout[0] += M[12];
	Vout[1] += M[13];
	Vout[2] += M[14];
}

FocalCamera::FocalCamera()
{
	//eyeSeparation = 63;
	eyeSeparation = 100;


	aperture = 45;
	focalLength = 100;//5000;

	nearDist = 10;
	farDist = 50000;

	moveSpeed = 10;


	//defaults
	viewDist = 1000;
	position.x = 1000;
	position.y = 1000;
	position.z = 1000;
	focalPoint.x = 1000;
	focalPoint.y = 1000;
	focalPoint.z = 0;
	nextFocalPoint = focalPoint;

	viewDistSpeed = 100;

	//moving = false;

	navigationConstraintsActive = true;
}

FocalCamera::~FocalCamera()
{

}

//calc direction, up, and left vectors from the matrix
void FocalCamera::UpdateDirectionVectors()
{
	float matrix[16];
	orientation.CreateMatrix(matrix);
	
	//extract direction vector
	float origin[3] = {0,0,0};
	float newOrigin[3];
	float forwardUnit[3] = {0,0,1};
	float newForwardUnit[3];

	MVMult(&matrix[0], &origin[0], &newOrigin[0]);
	MVMult(&matrix[0], &forwardUnit[0], &newForwardUnit[0]);

	directionVector.x = newForwardUnit[0] - newOrigin[0];
	directionVector.y = newForwardUnit[1] - newOrigin[1];
	directionVector.z = newForwardUnit[2] - newOrigin[2];
	directionVector.normalize();

	//extract up vector
	float upUnit[3] = {0,1,0};
	float newUpUnit[3];

	MVMult(&matrix[0], &origin[0], &newOrigin[0]);
	MVMult(&matrix[0], &upUnit[0], &newUpUnit[0]);

	upVector.x = newUpUnit[0] - newOrigin[0];
	upVector.y = newUpUnit[1] - newOrigin[1];
	upVector.z = newUpUnit[2] - newOrigin[2];
	upVector.normalize();

	//calc left vector
	leftVector = crossProduct(upVector, directionVector);
	leftVector.normalize();

}

void FocalCamera::UpdateEyePositions()
{
	//swap in next focal point, this is necessary because the focal point can be modified by another thread, e.g. touch callbacks, and this could cause eyes to render with different focal points
	focalPoint = nextFocalPoint;
	calcOrientation();
	UpdateDirectionVectors();
	calcPosition();
	nextFocalPoint = focalPoint;

	leftPosition.x = position.x + (eyeSeparation*0.5)*leftVector.x;
	leftPosition.y = position.y + (eyeSeparation*0.5)*leftVector.y;
	leftPosition.z = position.z + (eyeSeparation*0.5)*leftVector.z;

	rightPosition.x = position.x + (eyeSeparation*0.5)*-leftVector.x;
	rightPosition.y = position.y + (eyeSeparation*0.5)*-leftVector.y;
	rightPosition.z = position.z + (eyeSeparation*0.5)*-leftVector.z;
}


void FocalCamera::calcOrientation()
{
	Quaternion tempPitch;
	Quaternion tempYaw;
	tempYaw.CreateFromAxisAngle(0.0f, 0.0f, 1.0f, yaw);
	
	float matrix[16];
	tempYaw.CreateMatrix(matrix);
	
	//extract axis vector to yaw around
	float origin[3] = {0,0,0};
	float newOrigin[3];
	float axisUnit[3] = {1.0,0.0,0.0};
	float newAxisUnit[3];

	MVMult(&matrix[0], &origin[0], &newOrigin[0]);
	MVMult(&matrix[0], &axisUnit[0], &newAxisUnit[0]);

	//store matrix for later in interaction volume work
//	for (int i=0;i<16;i++)
	//	lastMatrix[i] = matrix[i]; 

	Vec3 axisVec;
	axisVec.x = newAxisUnit[0] - newOrigin[0];
	axisVec.y = newAxisUnit[1] - newOrigin[1];
	axisVec.z = newAxisUnit[2] - newOrigin[2];
	axisVec.normalize();

	tempPitch.CreateFromAxisAngle(axisVec.x, axisVec.y, axisVec.z, pitch);

	//tempPitch.CreateFromAxisAngle(1.0f, 0.0f, 0.0f, pitch);
	//tempYaw.CreateFromAxisAngle(0.0f, 1.0f, 0.0f, yaw);
	orientation.reset();
	orientation = tempPitch * tempYaw;
	orientation.normalize();
}

void FocalCamera::calcPosition()
{
	position.x = focalPoint.x - viewDist*directionVector.x;
	position.y = focalPoint.y - viewDist*directionVector.y;
	position.z = focalPoint.z - viewDist*directionVector.z;

	viewDistSpeed = 75 + abs(position.z) / 7;
}



void FocalCamera::ChangeSpeed(float speed)
{
	moveSpeed += speed;
}

void FocalCamera::SetSpeed(float newSpeed)
{
	moveSpeed = newSpeed;
}


void FocalCamera::MoveForward(float distance)
{
	position.x += distance*directionVector.x;
	position.y += distance*directionVector.y;
	position.z += distance*directionVector.z;
}

void FocalCamera::MoveBackward(float distance)
{
	position.x -= distance*directionVector.x;
	position.y -= distance*directionVector.y;
	position.z -= distance*directionVector.z;
}

void FocalCamera::MoveLeft(float distance)
{
	position.x = position.x + (distance)*leftVector.x;
	position.y = position.y + (distance)*leftVector.y;
	position.z = position.z + (distance)*leftVector.z;
}

void FocalCamera::MoveRight(float distance)
{
	position.x = position.x + (distance)*-leftVector.x;
	position.y = position.y + (distance)*-leftVector.y;
	position.z = position.z + (distance)*-leftVector.z;
}

void FocalCamera::MoveUp(float distance)
{
	position.x = position.x + (distance)*-upVector.x; //why this -1?
	position.y = position.y + (distance)*-upVector.y;
	position.z = position.z + (distance)*-upVector.z;
}

void FocalCamera::MoveDown(float distance)
{
	position.x = position.x + (distance)*upVector.x;
	position.y = position.y + (distance)*upVector.y;
	position.z = position.z + (distance)*upVector.z;
}

float FocalCamera::getDistToFocalPoint()
{
	/*float dotNP = dotProduct(Vec3(0,0,-1), Vec3(position.x, position.y, position.z));
	float dotNV = dotProduct(Vec3(0,0,-1), directionVector);
	float NPoNV = dotNP / dotNV;
	return (-1.0 * NPoNV * directionVector.length());*/
	return position.dist(focalPoint);
}

float FocalCamera::getDistanceFromCameraToGround()
{
	return abs(position.z);
}


Vec3 FocalCamera::getFocalPointOnGround()
{
	//float distToGroundAlongLookVector = getDistToFocalPoint();
	//
	//Vec3 focalPoint(position.x, position.y, position.z);
	//focalPoint.x += distToGroundAlongLookVector*directionVector.x;
	//focalPoint.y += distToGroundAlongLookVector*directionVector.y;
	//focalPoint.z += distToGroundAlongLookVector*directionVector.z;

	return focalPoint;
}


void FocalCamera::TranslateAlongGroundPlane(float distanceX, float distanceY)
{
	//printf("need to move %f, %f\n", distanceX, distanceY);
	//figure out how far in ground coordinates this number of pixels is

	float ratio = (float)winWidth/(float)winHeight;
	float screenWidthInGroundCoords = 2 * ( (getDistToFocalPoint()+20) * tan( 0.0174532925*getViewingAngle()) );
	

	//printf("dtg = %f, swigc = %f\n", getDistToFocalPoint(), screenWidthInGroundCoords);
	float unitsPerPixel = screenWidthInGroundCoords / winWidth;
	///printf("upp = %f\n", unitsPerPixel);

	distanceX *= unitsPerPixel;
	distanceY *= unitsPerPixel;



	//printf("going to move %f, %f\n", distanceX, distanceY);
	//figure out how far to translate based on camera distance
//	float translateFactor = .25;
//	distanceX *= 20+sqrt(viewDist)*translateFactor;
//	distanceY *= 20+sqrt(viewDist)*translateFactor;

	//get point on ground along viewing direction vector
	float distToFocalPoint = focalPoint.dist(position);

	//get point on ground along viewing direction vector from point translated along left vector
	Vec3 leftSome;
	//printf("Position %f, %f, %f\n", position.x, position.y, position.z);
	leftSome.x = position.x + leftVector.x*200;
	leftSome.y = position.y + leftVector.y*200;
	leftSome.z = position.z + leftVector.z*200;
	//float dotNP = dotProduct(Vec3(0,0,1), Vec3(leftSome.x, leftSome.y, leftSome.z));
	//float dotNV = dotProduct(Vec3(0,0,1), directionVector);
	//float NPoNV = dotNP / dotNV;
	//float distanceToGround = (-1.0 * NPoNV * directionVector.length()); ///gives dist to zero plane?
	float distanceToGround = distToFocalPoint; ///maybe this works better?

	Vec3 leftOnGround(leftSome.x, leftSome.y, leftSome.z);
	//printf("LeftSome %f, %f, %f\n", leftOnGround.x, leftOnGround.y, leftOnGround.z);
	//printf("DistToG %f DirectV %f, %f, %f\n", distanceToGround, directionVector.x, directionVector.y, directionVector.z);
	leftOnGround.x += distanceToGround*directionVector.x;
	leftOnGround.y += distanceToGround*directionVector.y;
	leftOnGround.z += distanceToGround*directionVector.z;

	tempLeftPoint.x = leftOnGround.x;
	tempLeftPoint.y = leftOnGround.y;
	tempLeftPoint.z = leftOnGround.z;
	//printf("FocalOnGround %f, %f, %f\n", focalPoint.x, focalPoint.y, focalPoint.z);
	//printf("LeftOnGround %f, %f, %f\n", leftOnGround.x, leftOnGround.y, leftOnGround.z);

	//get vector from left point to focal point, this is the horizontal (EAST/WEST) vector
	Vec3 horizontalVector;
	horizontalVector.x = leftOnGround.x - focalPoint.x;
	horizontalVector.y = leftOnGround.y - focalPoint.y;
	horizontalVector.z = leftOnGround.z - focalPoint.z;

	//printf("HV %f, %f, %f\n", horizontalVector.x, horizontalVector.y, horizontalVector.z);

	horizontalVector.normalize();
	
	//derive vertical (North/South) vector
	//if a = negative plane vector (into map), b = left/west vector, then a x b = up/north vector
	Vec3 northVector = crossProduct(Vec3(0,0,1),horizontalVector);
	northVector.normalize();

	//get new focal point location
	nextFocalPoint.x = nextFocalPoint.x + (distanceX * horizontalVector.x) + (distanceY * northVector.x);
	nextFocalPoint.y = nextFocalPoint.y + (distanceX * horizontalVector.y) + (distanceY * northVector.y);
	//nextFocalPoint.z = focalPoint.z + (distanceX * horizontalVector.z) + (distanceY * northVector.z);
	//printf("new focal point is %f,%f,%f\n", nextFocalPoint.x, nextFocalPoint.y,nextFocalPoint.z);
}



void FocalCamera::setFocalPoint(float x, float y, float z)
{
	focalPoint.x = x;
	focalPoint.y = y;
	focalPoint.z = z;
	nextFocalPoint.x = x;
	nextFocalPoint.y = y;
	nextFocalPoint.z = z;
	//calcPosition();
}

void FocalCamera::setViewAngle(float degreesYaw, float degreesPitch)
{
	yaw = degreesYaw;
	pitch = degreesPitch;
	//calcOrientation();
}

void FocalCamera::setYaw(float degreesYaw)
{
	yaw = degreesYaw;
	//calcOrientation();
}

void FocalCamera::setPitch(float degreesPitch)
{
	pitch = degreesPitch;
	//calcOrientation();
}

void FocalCamera::changePitch(float degreesPitch)
{
	pitch += degreesPitch;
	if (navigationConstraintsActive)
	{
		if (pitch > 180)
			pitch = 180;
		if (pitch < 93)
			pitch = 93;
	}
}

void FocalCamera::changeYaw(float degreesYaw)
{
	yaw += degreesYaw;
	if (navigationConstraintsActive)
	{
		if (yaw > 360)
			yaw -= 360;
		if (yaw < 0)
			yaw += 360;
	}
}

void FocalCamera::setViewDistance(float Dist)
{
	viewDist = Dist;
	//calcPosition();
}

void FocalCamera::setWinSize(int width, int height)
{
	winWidth = width;
	winHeight = height;
}

void FocalCamera::changeViewDistance(float degreesDist)
{
	if (viewDist > 1000)
		viewDist += degreesDist;
	else if (viewDist > 300)
		viewDist += degreesDist/5;
	else
		viewDist += degreesDist/25;
	if (navigationConstraintsActive)
	{
		if (viewDist < 10)
			viewDist = 10;
		if (viewDist > 50000)
			viewDist = 50000;
	}
}

void FocalCamera::changeViewAngle(float degreesYaw, float degreesPitch)
{
	yaw += degreesYaw;
	pitch += degreesPitch;

	if (navigationConstraintsActive)
	{
		if (yaw > 360)
			yaw -= 360;
		if (yaw < 0)
			yaw += 360;
		if (pitch > 180)
			pitch = 180;
		if (pitch < 93)
			pitch = 93;
	}
	
	
	//calcOrientation();
	//calcPosition();
}



void FocalCamera::moveFocalPoint(float x, float y, float z)
{
	focalPoint.x += x;
	focalPoint.y += y;
	focalPoint.z += z;
	//calcPosition();
}

float FocalCamera::getViewingAngle()
{
	return aperture;
}

Vec3 FocalCamera::getInteractionVolumeCenter()
{
	return focalPoint;
}

float FocalCamera::getInteractionVolumeSize()
{
	return 1 * viewDist * tan(DTOR * aperture);		
}

Vec3 FocalCamera::getInteractionVolumeUp()
{
	return upVector;
}

void FocalCamera::getRotationMatrix(float *matrix)
{
	orientation.CreateMatrix(matrix);
	//for (int i=0;i<16;i++)
		//matrix[i] = lastMatrix[i];
	//return matrix;
}