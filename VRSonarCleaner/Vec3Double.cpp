#include "Vec3Double.h"

double dotProduct(Vec3Double v1, Vec3Double v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec3Double crossProduct(Vec3Double v1, Vec3Double v2)
{
	Vec3Double crossProd;

	crossProd.x = v1.y*v2.z-v1.z*v2.y;
	crossProd.y = v1.z*v2.x-v1.x*v2.z;
	crossProd.z = v1.x*v2.y-v1.y*v2.x; 

	return crossProd;
}

Vec3Double faceNormal(Vec3Double v1, Vec3Double v2, Vec3Double v3)
{
	Vec3Double U;
	U.x = v2.x-v1.x;
	U.y = v2.y-v1.y;
	U.z = v2.z-v1.z;

	U.normalize();

	Vec3Double V;
	V.x = v3.x-v1.x;
	V.y = v3.y-v1.y;
	V.z = v3.z-v1.z;
                  
	V.normalize();

	Vec3Double normal = crossProduct(U, V);
	normal.normalize();
	return normal;
}


Vec3Double::Vec3Double()
{
	x = y = z = 0.0f;
}

Vec3Double::Vec3Double(double X, double Y, double Z)
{
	x = X;
	y = Y;
	z = Z;
}

Vec3Double::~Vec3Double()
{

}

void Vec3Double::operator *=(double scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

void Vec3Double::operator /=(double scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
}

void Vec3Double::operator +=(Vec3Double &other)
{
	x += other.x;
	y += other.y;
	z += other.z;
}

void Vec3Double::operator -=(Vec3Double &other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}


void Vec3Double::normalize()
{
	double len = length();
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

double Vec3Double::length()
{
	return sqrt( (x*x)+(y*y)+(z*z) );
}

void Vec3Double::invert()
{
	x = -x;
	y = -y;
	z = -z;
}

double Vec3Double::dist(Vec3Double other)
{
	return sqrt( (other.x-x)*(other.x-x) + (other.y-y)*(other.y-y) + (other.z-z)*(other.z-z));
}

//-1 = negative side, 0 = on plane, 1 = positive side
int Vec3Double::whichSideOfPlane(double A, double B, double C, double D)
{
	double result = A*x + B*y + C*z + D;
	if (result > 0)
		return 1;
	else if (result < 0)
		return -1;
	else
		return 0;
}