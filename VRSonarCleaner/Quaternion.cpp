#include "Quaternion.h"

Quaternion::Quaternion()
{
	m_x = m_y = m_z = 0.0f;
	m_w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

Quaternion::~Quaternion()
{

}

void Quaternion::reset()
{
	m_x = m_y = m_z = 0.0f;
	m_w = 1.0f;
}

void Quaternion::CreateFromAxisAngle(float x, float y, float z, float degrees)
{
	// First we want to convert the degrees to radians 
	// since the angle is assumed to be in radians
	float angle = float((degrees / 180.0f) * PI);

	// Here we calculate the sin( theta / 2) once for optimization
	float result = (float)sin( angle / 2.0f );
		
	// Calcualte the w value by cos( theta / 2 )
	m_w = (float)cos( angle / 2.0f );

	// Calculate the x, y and z of the quaternion
	m_x = float(x * result);
	m_y = float(y * result);
	m_z = float(z * result);
}

void Quaternion::CreateMatrix(float *pMatrix)
{
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix) return;

	// First row
	pMatrix[ 0] = 1.0f - 2.0f * ( m_y * m_y + m_z * m_z ); 
	pMatrix[ 1] = 2.0f * (m_x * m_y + m_z * m_w);
	pMatrix[ 2] = 2.0f * (m_x * m_z - m_y * m_w);
	pMatrix[ 3] = 0.0f;  

	// Second row
	pMatrix[ 4] = 2.0f * ( m_x * m_y - m_z * m_w );  
	pMatrix[ 5] = 1.0f - 2.0f * ( m_x * m_x + m_z * m_z ); 
	pMatrix[ 6] = 2.0f * (m_z * m_y + m_x * m_w );  
	pMatrix[ 7] = 0.0f;  

	// Third row
	pMatrix[ 8] = 2.0f * ( m_x * m_z + m_y * m_w );
	pMatrix[ 9] = 2.0f * ( m_y * m_z - m_x * m_w );
	pMatrix[10] = 1.0f - 2.0f * ( m_x * m_x + m_y * m_y );  
	pMatrix[11] = 0.0f;  

	// Fourth row
	pMatrix[12] = 0;  
	pMatrix[13] = 0;  
	pMatrix[14] = 0;  
	pMatrix[15] = 1.0f;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}

Quaternion Quaternion::operator *(Quaternion q)
{
	Quaternion r;
	
	r.m_x = m_w*q.m_x + m_x*q.m_w + m_y*q.m_z - m_z*q.m_y;
	r.m_y = m_w*q.m_y + m_y*q.m_w + m_z*q.m_x - m_x*q.m_z;
	r.m_z = m_w*q.m_z + m_z*q.m_w + m_x*q.m_y - m_y*q.m_x;
	r.m_w = m_w*q.m_w - m_x*q.m_x - m_y*q.m_y - m_z*q.m_z;
	
	return(r);
}

//Vec3 Quaternion::operator *(Vec3 vec)
//{
//	Vec3 vn(vec.x, vec.y, vec.z);
//	vn.normalize();
//
//	Quaternion vecQuat;
//	Quaternion resQuat;
//	vecQuat.m_x = vn.x;
//	vecQuat.m_y = vn.y;
//	vecQuat.m_z = vn.z;
//	vecQuat.m_w = 0.0f;
//
//	resQuat = vecQuat * getConjugate();
//	resQuat = *this * resQuat;
//
//	return Vec3(resQuat.m_x, resQuat.m_y, resQuat.m_z);
//}

float Quaternion::getMagnitude()
{
	return sqrt(m_w*m_w + m_x*m_x + m_y*m_y + m_z*m_z);
}

void Quaternion::normalize()
{
	float mag = getMagnitude();
	if (mag !=0)
	{
		m_w /= mag;
		m_x /= mag;
		m_y /= mag;
		m_z /= mag;      
	}  
}

Quaternion Quaternion::getConjugate()
{
	tquat<T, P> z = y;

	T cosTheta = dot(x, y);

	// If cosTheta < 0, the interpolation will take the long way around the sphere. 
	// To fix this, one quat must be negated.
	if (cosTheta < T(0))
	{
		z = -y;
		cosTheta = -cosTheta;
	}

	// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
	if (cosTheta > T(1) - epsilon<T>())
	{
		// Linear interpolation
		return tquat<T, P>(
			mix(x.w, z.w, a),
			mix(x.x, z.x, a),
			mix(x.y, z.y, a),
			mix(x.z, z.z, a));
	}
	else
	{
		// Essential Mathematics, page 467
		T angle = acos(cosTheta);
		return (sin((T(1) - a) * angle) * x + sin(a * angle) * z) / sin(angle);
	}
}

Quaternion Quaternion::slerp(const Quaternion & q1, const Quaternion & q2, float amount)
{
	return Quaternion();
}

Quaternion Quaternion::castMatrixToQuat(const Matrix & m)
{

	float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	float biggestVal = sqrt(fourBiggestSquaredMinus1 + T(1)) * T(0.5);
	float mult = static_cast<T>(0.25) / biggestVal;

	Quaternion Result;
	switch (biggestIndex)
	{
	case 0:
		Result.m_w = biggestVal;
		Result.m_x = (m[1][2] - m[2][1]) * mult;
		Result.m_y = (m[2][0] - m[0][2]) * mult;
		Result.m_z = (m[0][1] - m[1][0]) * mult;
		break;
	case 1:
		Result.m_w = (m[1][2] - m[2][1]) * mult;
		Result.m_x = biggestVal;
		Result.m_y = (m[0][1] + m[1][0]) * mult;
		Result.m_z = (m[2][0] + m[0][2]) * mult;
		break;
	case 2:
		Result.m_w = (m[2][0] - m[0][2]) * mult;
		Result.m_x = (m[0][1] + m[1][0]) * mult;
		Result.m_y = biggestVal;
		Result.m_z = (m[1][2] + m[2][1]) * mult;
		break;
	case 3:
		Result.m_w = (m[0][1] - m[1][0]) * mult;
		Result.m_x = (m[2][0] + m[0][2]) * mult;
		Result.m_y = (m[1][2] + m[2][1]) * mult;
		Result.m_z = biggestVal;
		break;

	return Result;	
}

