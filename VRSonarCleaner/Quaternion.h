#ifndef __Quaternion_h__
#define __Quaternion_h__

#include <windows.h>		// Header File For Windows
#include "GL/glew.h"
#include <math.h>

#include "../shared/Matrices.h"

#define PI			3.14159265358979323846

#pragma once

class Quaternion  
{
public:
	
	Quaternion();
	Quaternion(float x, float y, float z, float w);
	virtual ~Quaternion();
	
	Quaternion operator +(const Quaternion & b) const;
	Quaternion operator *(Quaternion q);
	Quaternion operator *(float s) const;
	//Vec3 operator *(Vec3 vec);

	// Convert Matrix to Quaternion by assignment
	Quaternion & operator =(const Matrix4 & m);

	void reset();

	void CreateMatrix(float *pMatrix);
	void CreateFromAxisAngle(float x, float y, float z, float degrees);
	float getMagnitude();
	Quaternion & normalize();
	float dot(const Quaternion & q2) const;
	Quaternion getConjugate();
	Quaternion& lerp(Quaternion q1, Quaternion q2, float a);
	Quaternion& slerp(const Quaternion q1, const Quaternion q2, float a);

private:
	float m_w;
	float m_z;
	float m_y;
	float m_x;
};

#endif
