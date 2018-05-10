#pragma once

#include "utilities.h"

#include <gtx/intersect.hpp>
#include <gtx/vector_angle.hpp>

namespace ccomutils
{
	std::vector<glm::vec3> transformStereoscopicPoints(glm::vec3 centerOfProjL, glm::vec3 centerOfProjR, glm::vec3 viewPosL, glm::vec3 viewPosR, glm::vec3 screenCtr, glm::vec3 screenNorm, const std::vector<glm::vec3> pts)
	{
		std::vector<glm::vec3> iL(getScreenIntersections(centerOfProjL, screenCtr, screenNorm, pts));
		std::vector<glm::vec3> iR(getScreenIntersections(centerOfProjR, screenCtr, screenNorm, pts));

		std::vector<glm::vec3> ret;
		for (int i = 0; i < pts.size(); ++i)
		{
			glm::vec3 pa, pb;
			float mua, mub;
			LineLineIntersect(viewPosL, iL[i], viewPosR, iR[i], &pa, &pb, &mua, &mub);
			ret.push_back((pa + pb) / 2.f);
		}

		return ret;
	}

	std::vector<glm::vec3> getScreenIntersections(glm::vec3 centerOfProjection, glm::vec3 screenCenter, glm::vec3 screenNormal, const std::vector<glm::vec3> pts)
	{
		std::vector<glm::vec3> ret;

		for (auto pt : pts)
		{
			float ptDist;
			glm::intersectRayPlane(centerOfProjection, pt - centerOfProjection, screenCenter, screenNormal, ptDist);
			ret.push_back(centerOfProjection + (pt - centerOfProjection) * ptDist);
		}
		return ret;
	}

	/////////////////////////////////////////////////////////////////////////////
	// from http://paulbourke.net/geometry/pointlineplane/lineline.c		   //
	// 																		   //
	// Calculate the line segment PaPb that is the shortest route between	   //
	// two lines P1P2 and P3P4. Calculate also the values of mua and mub where //
	// Pa = P1 + mua (P2 - P1)												   //
	// Pb = P3 + mub (P4 - P3)												   //
	// Return false if no solution exists.									   //
	/////////////////////////////////////////////////////////////////////////////
	bool LineLineIntersect(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 *pa, glm::vec3 *pb, float *mua, float *mub)
	{
		glm::vec3 p13, p43, p21;
		float d1343, d4321, d1321, d4343, d2121;
		float numer, denom;

		p13.x = p1.x - p3.x;
		p13.y = p1.y - p3.y;
		p13.z = p1.z - p3.z;
		p43.x = p4.x - p3.x;
		p43.y = p4.y - p3.y;
		p43.z = p4.z - p3.z;
		if (glm::abs(p43.x) < glm::epsilon<float>() && glm::abs(p43.y) < glm::epsilon<float>() && glm::abs(p43.z) < glm::epsilon<float>())
			return false;
		p21.x = p2.x - p1.x;
		p21.y = p2.y - p1.y;
		p21.z = p2.z - p1.z;
		if (glm::abs(p21.x) < glm::epsilon<float>() && glm::abs(p21.y) < glm::epsilon<float>() && glm::abs(p21.z) < glm::epsilon<float>())
			return false;

		d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
		d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
		d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
		d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
		d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

		denom = d2121 * d4343 - d4321 * d4321;
		if (glm::abs(denom) < glm::epsilon<float>())
			return false;
		numer = d1343 * d4321 - d1321 * d4343;

		*mua = numer / denom;
		*mub = (d1343 + d4321 * (*mua)) / d4343;

		pa->x = p1.x + *mua * p21.x;
		pa->y = p1.y + *mua * p21.y;
		pa->z = p1.z + *mua * p21.z;
		pb->x = p3.x + *mub * p43.x;
		pb->y = p3.y + *mub * p43.y;
		pb->z = p3.z + *mub * p43.z;

		return true;
	}

	glm::mat4 getBillBoardTransform(const glm::vec3 & pos, const glm::vec3 & at, const glm::vec3 &up, bool lockToUpVector)
	{
		glm::vec3 f(glm::normalize(at - pos));
		glm::vec3 s(glm::normalize(cross(up, f)));
		glm::vec3 u(glm::cross(f, s));

		if (lockToUpVector)
		{
			u = up;
			f = glm::normalize(glm::cross(s, u));
		}

		glm::mat4 result;
		result[0] = glm::vec4(s, 0.f);
		result[1] = glm::vec4(u, 0.f);
		result[2] = glm::vec4(f, 0.f);
		result[3] = glm::vec4(pos, 1.f);

		return result;
	}

	// TODO: fix this function. Everything up to the perspective divide is good.
	glm::mat4 getUnprojectionMatrix(glm::mat4 & proj, glm::mat4 & view, glm::mat4 & model, glm::ivec4 & vp)
	{
		glm::mat4 inv = glm::inverse(proj * view);

		glm::mat4 screenToNDC =
			glm::translate(glm::mat4(), glm::vec3(-1.f)) *
			glm::scale(glm::mat4(), glm::vec3(2.f / vp[2], 2.f / vp[3], 2.f)) *
			glm::translate(glm::mat4(), glm::vec3(-vp[0], -vp[1], 0.f));

		glm::mat4 beforePerspDivide = inv * screenToNDC;

		float perspDiv = 1.f / beforePerspDivide[3].w;
		glm::mat4 perspDivMat(1);
		perspDivMat[0][0] = perspDiv;
		perspDivMat[1][1] = perspDiv;
		perspDivMat[2][2] = perspDiv;
		perspDivMat[3][3] = perspDiv;

		return perspDivMat * beforePerspDivide;
	}
}
