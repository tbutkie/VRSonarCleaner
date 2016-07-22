#include "MatrixUtils.h"

// Multiply matrices in OpenGL (column-major) order
void MMult(double *M1, double *M2, double *Mout)
{
	Mout[ 0] = M1[ 0]*M2[ 0]+M1[ 4]*M2[ 1]+M1[ 8]*M2[ 2]+M1[12]*M2[ 3];
	Mout[ 1] = M1[ 1]*M2[ 0]+M1[ 5]*M2[ 1]+M1[ 9]*M2[ 2]+M1[13]*M2[ 3];
	Mout[ 2] = M1[ 2]*M2[ 0]+M1[ 6]*M2[ 1]+M1[10]*M2[ 2]+M1[14]*M2[ 3];
	Mout[ 3] = M1[ 3]*M2[ 0]+M1[ 7]*M2[ 1]+M1[11]*M2[ 2]+M1[15]*M2[ 3];
	Mout[ 4] = M1[ 0]*M2[ 4]+M1[ 4]*M2[ 5]+M1[ 8]*M2[ 6]+M1[12]*M2[ 7];
	Mout[ 5] = M1[ 1]*M2[ 4]+M1[ 5]*M2[ 5]+M1[ 9]*M2[ 6]+M1[13]*M2[ 7];
	Mout[ 6] = M1[ 2]*M2[ 4]+M1[ 6]*M2[ 5]+M1[10]*M2[ 6]+M1[14]*M2[ 7];
	Mout[ 7] = M1[ 3]*M2[ 4]+M1[ 7]*M2[ 5]+M1[11]*M2[ 6]+M1[15]*M2[ 7];
	Mout[ 8] = M1[ 0]*M2[ 8]+M1[ 4]*M2[ 9]+M1[ 8]*M2[10]+M1[12]*M2[11];
	Mout[ 9] = M1[ 1]*M2[ 8]+M1[ 5]*M2[ 9]+M1[ 9]*M2[10]+M1[13]*M2[11];
	Mout[10] = M1[ 2]*M2[ 8]+M1[ 6]*M2[ 9]+M1[10]*M2[10]+M1[14]*M2[11];
	Mout[11] = M1[ 3]*M2[ 8]+M1[ 7]*M2[ 9]+M1[11]*M2[10]+M1[15]*M2[11];
	Mout[12] = M1[ 0]*M2[12]+M1[ 4]*M2[13]+M1[ 8]*M2[14]+M1[12]*M2[15];
	Mout[13] = M1[ 1]*M2[12]+M1[ 5]*M2[13]+M1[ 9]*M2[14]+M1[13]*M2[15];
	Mout[14] = M1[ 2]*M2[12]+M1[ 6]*M2[13]+M1[10]*M2[14]+M1[14]*M2[15];
	Mout[15] = M1[ 3]*M2[12]+M1[ 7]*M2[13]+M1[11]*M2[14]+M1[15]*M2[15];
};

// Multiply a vector by a matrix
void MVMult(double *M, double *V, double *Vout, bool apply_trans)
{
	Vout[0] = M[0] * V[0] + M[4] * V[1] + M[8] * V[2];
	Vout[1] = M[1] * V[0] + M[5] * V[1] + M[9] * V[2];
	Vout[2] = M[2] * V[0] + M[6] * V[1] + M[10] * V[2];
	if (apply_trans) {
		Vout[0] += M[12];
		Vout[1] += M[13];
		Vout[2] += M[14];
	}
};

// generate identity matrix
void IdentityMat(double *Mout)
{
	Mout[ 0] = 1;
	Mout[ 1] = 0;
	Mout[ 2] = 0;
	Mout[ 3] = 0;
	Mout[ 4] = 0;
	Mout[ 5] = 1;
	Mout[ 6] = 0;
	Mout[ 7] = 0;
	Mout[ 8] = 0;
	Mout[ 9] = 0;
	Mout[10] = 1;
	Mout[11] = 0;
	Mout[12] = 0;
	Mout[13] = 0;
	Mout[14] = 0;
	Mout[15] = 1;
};

// generate translation matrix
void TransMat(double x, double y, double z, double *Mout)
{/*
	Mout[ 0] = 1;
	Mout[ 1] = 0;
	Mout[ 2] = 0;
	Mout[ 3] = x;
	Mout[ 4] = 0;
	Mout[ 5] = 1;
	Mout[ 6] = 0;
	Mout[ 7] = y;
	Mout[ 8] = 0;
	Mout[ 9] = 0;
	Mout[10] = 1;
	Mout[11] = z;
	Mout[12] = 0;
	Mout[13] = 0;
	Mout[14] = 0;
	Mout[15] = 1;
*/
	Mout[ 0] = 1;
	Mout[ 1] = 0;
	Mout[ 2] = 0;
	Mout[ 3] = 0;
	Mout[ 4] = 0;
	Mout[ 5] = 1;
	Mout[ 6] = 0;
	Mout[ 7] = 0;
	Mout[ 8] = 0;
	Mout[ 9] = 0;
	Mout[10] = 1;
	Mout[11] = 0;
	Mout[12] = x;
	Mout[13] = y;
	Mout[14] = z;
	Mout[15] = 1;
};

// generate yaw matrix
void YawMat(double degrees, double *Mout)
{
	double rads = degrees * 0.0174532925;
	double c = cos(rads);
	double s = sin(rads);
	Mout[ 0] = c;
	Mout[ 1] = s;
	Mout[ 2] = 0;
	Mout[ 3] = 0;
	Mout[ 4] = -s;
	Mout[ 5] = c;
	Mout[ 6] = 0;
	Mout[ 7] = 0;
	Mout[ 8] = 0;
	Mout[ 9] = 0;
	Mout[10] = 1;
	Mout[11] = 0;
	Mout[12] = 0;
	Mout[13] = 0;
	Mout[14] = 0;
	Mout[15] = 1;
};

// generate pitch matrix
void PitchMat(double degrees, double *Mout)
{
	double rads = degrees * 0.0174532925;
	double c = cos(rads);
	double s = sin(rads);
	Mout[ 0] = 1;
	Mout[ 1] = 0;
	Mout[ 2] = 0;
	Mout[ 3] = 0;
	Mout[ 4] = 0;
	Mout[ 5] = c;
	Mout[ 6] = s;
	Mout[ 7] = 0;
	Mout[ 8] = 0;
	Mout[ 9] = -s;
	Mout[10] = c;
	Mout[11] = 0;
	Mout[12] = 0;
	Mout[13] = 0;
	Mout[14] = 0;
	Mout[15] = 1;
};


// generate roll matrix
void RollMat(double degrees, double *Mout)
{

	double rads = degrees * 0.0174532925;
	double c = cos(rads);
	double s = sin(rads);
	Mout[ 0] = c;
	Mout[ 1] = 0;
	Mout[ 2] = -s;
	Mout[ 3] = 0;
	Mout[ 4] = 0;
	Mout[ 5] = 1;
	Mout[ 6] = 0;
	Mout[ 7] = 0;
	Mout[ 8] = s;
	Mout[ 9] = 0;
	Mout[10] = c;
	Mout[11] = 0;
	Mout[12] = 0;
	Mout[13] = 0;
	Mout[14] = 0;
	Mout[15] = 1;
};

bool InvertMat(double m[16], double *invOut)
{
	double inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}