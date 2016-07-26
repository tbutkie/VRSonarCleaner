#ifndef __Quaternion_h__
#define __Quaternion_h__

#include <windows.h>		// Header File For Windows
#include "GL/glew.h"
#include <math.h>
//#include "Vec3.cpp" //DAMN circular includes!

#define PI			3.14159265358979323846

#pragma once

class Quaternion  
{
public:
	
	Quaternion();
	Quaternion(float x, float y, float z, float w);

	Quaternion operator *(Quaternion q);
	//Vec3 operator *(Vec3 vec);

	void CreateMatrix(float *pMatrix);
	void CreateFromAxisAngle(float x, float y, float z, float degrees);
	virtual ~Quaternion();
	void reset();
	float getMagnitude();
	void normalize();
	Quaternion getConjugate();
		

private:
	float m_w;
	float m_z;
	float m_y;
	float m_x;
};

#endif
