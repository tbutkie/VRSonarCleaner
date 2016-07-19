#ifndef __Vec3_h__
#define __Vec3_h__

#include <windows.h>
//#include "glew.h"
#include <math.h>

//#pragma once


class Vec3
{
public:
	void operator *=(double scalar);
	void operator /=(double scalar);
	void operator +=(Vec3 &other);
	void operator -=(Vec3 &other);
	Vec3 operator -(Vec3 &other )
	{
		return Vec3(x-other.x, y-other.y, z-other.z);
	}
	Vec3 operator +(Vec3 &other )
	{
		return Vec3(x+other.x, y+other.y, z+other.z);
	}
	Vec3 operator *(double scalar)
	{
		return Vec3(x*scalar, y*scalar, z*scalar);
	}
	Vec3 operator /(double scalar)
	{
		return Vec3(x/scalar, y/scalar, z/scalar);
	}


	
	Vec3();
	Vec3(double X, double Y, double Z);
	virtual ~Vec3();

	double length();

	double dist(Vec3 other);

	void normalize();

	void invert();

	double x;
	double y;
	double z;

	//-1 = negative side, 0 = on plane, 1 = positive side
	int whichSideOfPlane(double A, double B, double C, double D);

};


double dotProduct(Vec3 v1, Vec3 v2);
Vec3 crossProduct(Vec3 v1, Vec3 v2);
Vec3 faceNormal(Vec3 v1, Vec3 v2, Vec3 v3);


#endif