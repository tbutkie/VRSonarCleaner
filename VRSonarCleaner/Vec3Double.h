#ifndef __Vec3Double_h__
#define __Vec3Double_h__

#include <windows.h>
//#include "glew.h"
#include <math.h>

//#pragma once


class Vec3Double
{
public:
	void operator *=(double scalar);
	void operator /=(double scalar);
	void operator +=(Vec3Double &other);
	void operator -=(Vec3Double &other);
	Vec3Double operator -(Vec3Double &other )
	{
		return Vec3Double(x-other.x, y-other.y, z-other.z);
	}
	Vec3Double operator +(Vec3Double &other )
	{
		return Vec3Double(x+other.x, y+other.y, z+other.z);
	}
	Vec3Double operator *(double scalar)
	{
		return Vec3Double(x*scalar, y*scalar, z*scalar);
	}
	Vec3Double operator /(double scalar)
	{
		return Vec3Double(x/scalar, y/scalar, z/scalar);
	}


	
	Vec3Double();
	Vec3Double(double X, double Y, double Z);
	virtual ~Vec3Double();

	double length();

	double dist(Vec3Double other);

	void normalize();

	void invert();

	double x;
	double y;
	double z;

	//-1 = negative side, 0 = on plane, 1 = positive side
	int whichSideOfPlane(double A, double B, double C, double D);

};


double dotProduct(Vec3Double v1, Vec3Double v2);
Vec3Double crossProduct(Vec3Double v1, Vec3Double v2);
Vec3Double faceNormal(Vec3Double v1, Vec3Double v2, Vec3Double v3);


#endif