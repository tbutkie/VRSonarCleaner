#ifndef __FocalCamera_h__
#define __FocalCamera_h__

#define DTOR            0.0174532925

#include <windows.h>
#include <GL/glew.h>
#include "Vec3.h"
#include "Quaternion.h"
#include "Point3.h"
#include "stdio.h"


#pragma once

//extern float viewDist; ////<<<<<
//extern int winWidth, winHeight;

class FocalCamera  
{
public:
	FocalCamera();
	virtual ~FocalCamera();

	float moveSpeed;
	float aperture;
	float focalLength;

	float nearDist, farDist;

	////////////////////NEW FOCAL CAM STUFF/////////////////
	Vec3 position;
	Vec3 focalPoint;
	Vec3 nextFocalPoint;
	Quaternion orientation;
	float viewDistSpeed;
	float yaw;
	float pitch;
	void setFocalPoint(float x, float y, float z);
	void setViewAngle(float degreesYaw, float degreesPitch);
	void changeViewAngle(float degreesYaw, float degreesPitch);
	void setViewDistance(float Dist);
	void changeViewDistance(float Dist);
	void calcPosition();
	void calcOrientation();
	void moveFocalPoint(float x, float y, float z);
	void setYaw(float degreesYaw);
	void changeYaw(float degreesYaw);
	void setPitch(float degreesYaw);
	void changePitch(float degreesYaw);
	bool navigationConstraintsActive;

	float getDistanceFromCameraToGround();
	float getViewingAngle();

	Vec3 getInteractionVolumeCenter();
	float getInteractionVolumeSize();
	Vec3 getInteractionVolumeUp();
	void getRotationMatrix(float *matrix);
	///////////////////////////////////////////////////////


	Vec3 leftPosition;
	Vec3 rightPosition;
	float eyeSeparation;

	Vec3 leftVector;
	Vec3 directionVector;
	Vec3 upVector;

	void UpdateDirectionVectors();
	void UpdateEyePositions();
	
	
	void ChangeSpeed(float speed);
	void MoveForward(float distance);
	void MoveBackward(float distance);

	void SetVelocity(float speed);
	void StopMoving();
	bool moving;
	float velocity;
	float lastMove;


	//Vec3 focalPoint;
	//float viewDist;
	//Quaternion viewAngle;
	//void setFocalPoint;
	//void setViewAngle(float yaw, float pitch);



	void MoveUp(float distance);
	void MoveDown(float distance);
	void MoveLeft(float distance);
	void MoveRight(float distance);

	float getDistToFocalPoint();
	Vec3 getFocalPointOnGround();

	void TranslateAlongGroundPlane(float distanceX, float distanceY);
	Vec3 tempLeftPoint;


	void ChangeHeading(float degrees);
	void ChangePitch(float degrees);
	void ChangeYawPitch(float degreesYaw, float degreesPitch);
	//void SetPosition(float x, float y, float z);
	void SetYawPitch(float degreesYaw, float degreesPitch);
	void SetSpeed(float newSpeed);


	void ViewFocalPointFromAbove();

	float viewDist; ////<<<<<
	int winWidth, winHeight;

	void setWinSize(int width, int height);



};

#endif
