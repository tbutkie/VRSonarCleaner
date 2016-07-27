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

Quaternion Quaternion::operator +(const Quaternion& b) const
{
	return Quaternion(m_x + b.m_x, m_y + b.m_y, m_z + b.m_z, m_w + b.m_w);
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

Quaternion Quaternion::operator*(float s) const
{
	return Quaternion(s*m_x, s*m_y, s*m_z, s*m_w);
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

Quaternion & Quaternion::operator=(const Matrix4 & m)
{
	const float diag = m[0] + m[5] + m[10] + 1;

	if (diag > 0.0f)
	{
		const float scale = sqrtf(diag) * 2.0f; // get scale from diagonal

											  // TODO: speed this up
		m_x = (m[6] - m[9]) / scale;
		m_y = (m[8] - m[2]) / scale;
		m_z = (m[1] - m[4]) / scale;
		m_w = 0.25f * scale;
	}
	else
	{
		if (m[0]>m[5] && m[0]>m[10])
		{
			// 1st element of diag is greatest value
			// find scale according to 1st element, and double it
			const float scale = sqrtf(1.0f + m[0] - m[5] - m[10]) * 2.0f;

			// TODO: speed this up
			m_x = 0.25f * scale;
			m_y = (m[4] + m[1]) / scale;
			m_z = (m[2] + m[8]) / scale;
			m_w = (m[6] - m[9]) / scale;
		}
		else if (m[5]>m[10])
		{
			// 2nd element of diag is greatest value
			// find scale according to 2nd element, and double it
			const float scale = sqrtf(1.0f + m[5] - m[0] - m[10]) * 2.0f;

			// TODO: speed this up
			m_x = (m[4] + m[1]) / scale;
			m_y = 0.25f * scale;
			m_z = (m[9] + m[6]) / scale;
			m_w = (m[8] - m[2]) / scale;
		}
		else
		{
			// 3rd element of diag is greatest value
			// find scale according to 3rd element, and double it
			const float scale = sqrtf(1.0f + m[10] - m[0] - m[5]) * 2.0f;

			// TODO: speed this up
			m_x = (m[8] + m[2]) / scale;
			m_y = (m[9] + m[6]) / scale;
			m_z = 0.25f * scale;
			m_w = (m[1] - m[4]) / scale;
		}
	}

	return normalize();
}

void Quaternion::reset()
{
	m_x = m_y = m_z = 0.0f;
	m_w = 1.0f;
}

Matrix4 Quaternion::createMatrix(const Vector3 & center)
{
	return createMatrix(center.x, center.y, center.z);
}

Matrix4 Quaternion::createMatrix(float ctr_x, float ctr_y, float ctr_z)
{
	Matrix4 pMatrix;

	// First row
	pMatrix[0] = 1.f - 2.f * (m_y * m_y + m_z * m_z);
	pMatrix[1] = 2.f * (m_x * m_y + m_z * m_w);
	pMatrix[2] = 2.f * (m_x * m_z - m_y * m_w);
	pMatrix[3] = 0.f;

	// Second row
	pMatrix[4] = 2.0f * (m_x * m_y - m_z * m_w);
	pMatrix[5] = 1.0f - 2.0f * (m_x * m_x + m_z * m_z);
	pMatrix[6] = 2.0f * (m_z * m_y + m_x * m_w);
	pMatrix[7] = 0.0f;

	// Third row
	pMatrix[8] = 2.0f * (m_x * m_z + m_y * m_w);
	pMatrix[9] = 2.0f * (m_y * m_z - m_x * m_w);
	pMatrix[10] = 1.0f - 2.0f * (m_x * m_x + m_y * m_y);
	pMatrix[11] = 0.0f;

	// Fourth row (a.k.a matrix center location)
	pMatrix[12] = ctr_x;
	pMatrix[13] = ctr_y;
	pMatrix[14] = ctr_z;
	pMatrix[15] = 1.f;

	return pMatrix;
}

void Quaternion::createFromAxisAngle(float x, float y, float z, float degrees)
{
	// First we want to convert the degrees to radians 
	// since the angle is assumed to be in radians
	float angle = float((degrees / 180.0f) * PI);

	// Here we calculate the sin( theta / 2) once for optimization
	float result = (float)sin(angle / 2.0f);

	// Calcualte the w value by cos( theta / 2 )
	m_w = (float)cos(angle / 2.0f);

	// Calculate the x, y and z of the quaternion
	m_x = float(x * result);
	m_y = float(y * result);
	m_z = float(z * result);
}

float Quaternion::getMagnitude()
{
	return sqrt(m_w*m_w + m_x*m_x + m_y*m_y + m_z*m_z);
}

 Quaternion & Quaternion::normalize()
{
	const float n = m_x*m_x + m_y*m_y + m_z*m_z + m_w*m_w;

	if (n == 1)
		return *this;

	//n = 1.0f / sqrtf(n);
	return (*this = *this * (1.f / sqrtf(n)));
}

float Quaternion::dot(const Quaternion & q2) const
{
	return (m_x * q2.m_x) + (m_y * q2.m_y) + (m_z * q2.m_z) + (m_w * q2.m_w);
}

Quaternion Quaternion::getConjugate()
{
	return Quaternion(-m_x, -m_y, -m_z, -m_w);	
}

Quaternion & Quaternion::lerp(Quaternion q1, Quaternion q2, float a)
{
	const float scale = 1.0f - a;
	return (*this = (q1*scale) + (q2*a));
}

Quaternion & Quaternion::slerp(Quaternion q1, Quaternion q2, float a)
{
	float angle = q1.dot(q2);
	float threshold = 0.05f;

	// make sure we use the short rotation
	if (angle < 0.0f)
	{
		q1 = q1 * -1.0f;
		angle *= -1.0f;
	}

	if (angle <= (1 - threshold)) // spherical interpolation
	{
		const float theta = acosf(angle);
		const float invsintheta = 1.f / sinf(theta);
		const float scale = sinf(theta * (1.0f - a)) * invsintheta;
		const float invscale = sinf(theta * a) * invsintheta;
		return (*this = (q1*scale) + (q2*invscale));
	}
	else // linear interpolation
		return lerp(q1, q2, a);

	return Quaternion();
}

