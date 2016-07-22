#ifndef __MatrixUtils_h__
#define __MatrixUtils_h__

#include <math.h>

// Multiply matrices in OpenGL (column-major) order
void MMult(double *M1, double *M2, double *Mout);

// Multiply a vector by a matrix
void MVMult(double *M, double *V, double *Vout, bool apply_trans = true);

// generate identity matrix
void IdentityMat(double *Mout);

// generate translation matrix
void TransMat(double x, double y, double z, double *Mout);

// generate yaw matrix
void YawMat(double degrees, double *Mout);

// generate pitch matrix
void PitchMat(double degrees, double *Mout);


// generate roll matrix
void RollMat(double degrees, double *Mout);

//invert matrix
bool InvertMat(double m[16], double *invOut);

#endif