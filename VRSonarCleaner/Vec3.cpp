#include "Vec3.h"

double dotProduct(Vec3 v1, Vec3 v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec3 crossProduct(Vec3 v1, Vec3 v2)
{
	Vec3 crossProd;

	crossProd.x = v1.y*v2.z-v1.z*v2.y;
	crossProd.y = v1.z*v2.x-v1.x*v2.z;
	crossProd.z = v1.x*v2.y-v1.y*v2.x; 

	return crossProd;
}

Vec3 faceNormal(Vec3 v1, Vec3 v2, Vec3 v3)
{
	Vec3 U;
	U.x = v2.x-v1.x;
	U.y = v2.y-v1.y;
	U.z = v2.z-v1.z;

	U.normalize();

	Vec3 V;
	V.x = v3.x-v1.x;
	V.y = v3.y-v1.y;
	V.z = v3.z-v1.z;
                  
	V.normalize();

	Vec3 normal = crossProduct(U, V);
	normal.normalize();
	return normal;
}


Vec3::Vec3()
{
	x = y = z = 0.0f;
}

Vec3::Vec3(double X, double Y, double Z)
{
	x = X;
	y = Y;
	z = Z;
}

Vec3::~Vec3()
{

}

void Vec3::operator *=(double scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

void Vec3::operator /=(double scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
}

void Vec3::operator +=(Vec3 &other)
{
	x += other.x;
	y += other.y;
	z += other.z;
}

void Vec3::operator -=(Vec3 &other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}


void Vec3::normalize()
{
	float len = length();
	if (len == 0)
	{
		x = 0;
		y = 0;
		z = 0;
	}
	else
	{
		x /= len;
		y /= len;
		z /= len;
	}
}

double Vec3::length()
{
	return sqrt( (x*x)+(y*y)+(z*z) );
}

void Vec3::invert()
{
	x = -x;
	y = -y;
	z = -z;
}

double Vec3::dist(Vec3 other)
{
	return sqrt( (other.x-x)*(other.x-x) + (other.y-y)*(other.y-y) + (other.z-z)*(other.z-z));
}

//-1 = negative side, 0 = on plane, 1 = positive side
int Vec3::whichSideOfPlane(double A, double B, double C, double D)
{
	double result = A*x + B*y + C*z + D;
	if (result > 0)
		return 1;
	else if (result < 0)
		return -1;
	else
		return 0;
}