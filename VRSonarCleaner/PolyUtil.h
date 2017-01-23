#ifndef POLYUTIL_H
#define POLYUTIL_H

#include <vector>

#include <shared/glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159f
#endif

#ifndef M_2PI
#define M_2PI 6.28318f
#endif

class PolyUtil
{
public:
	static int inMultiPartPoly2(std::vector<float> xs, std::vector<float> ys, int startVert, int endVert, float testX, float testY)
	{
		int i, j, c = 0;
		for (i = startVert, j = endVert; i < endVert; j = i++)
		{
			if (((ys.at(i)>testY) != (ys.at(j)>testY)) && (testX < (xs.at(j) - xs.at(i)) * (testY - ys.at(i)) / (ys.at(j) - ys.at(i)) + xs.at(i)))
				c = !c;
		}
		return c;
	}

	/*
	Return the angle between two vectors on a plane
	The angle is from vector 1 to vector 2, positive anticlockwise
	The result is between -pi -> pi
	*/
	static float Angle2D(glm::vec2 pt1, glm::vec2 pt2)
	{
		double dtheta, theta1, theta2;

		theta1 = atan2(pt1.y, pt1.x);
		theta2 = atan2(pt2.y, pt2.x);
		dtheta = theta2 - theta1;
		while (dtheta > M_PI)
			dtheta -= M_2PI;
		while (dtheta < -M_PI)
			dtheta += M_2PI;

		return(dtheta);
	}

	static bool inMultiPartPoly(const std::vector<glm::vec2> &polyPts, const glm::vec2 &testPt)
	{
		int i;
		float angle = 0.f;
		glm::vec2 p1, p2;
		int n = polyPts.size();

		for (i = 0; i < n; i++)
		{
			p1 = polyPts.at(i) - testPt;
			p2 = polyPts.at((i + 1) % n) - testPt;
			angle += Angle2D(p1, p2);
		}

		if (abs(angle) < M_PI)
			return false;
		else
			return true;
	}
};

#endif
