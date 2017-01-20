#ifndef POLYUTIL_H
#define POLYUTIL_H

#include <vector>
#include <shared/glm/glm.hpp>

class PolyUtil
{
public:
	int inMultiPartPoly2(std::vector<float> xs, std::vector<float> ys, int startVert, int endVert, float testX, float testY)
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
	double Angle2D(double x1, double y1, double x2, double y2)
	{
		double dtheta, theta1, theta2;

		theta1 = atan2(y1, x1);
		theta2 = atan2(y2, x2);
		dtheta = theta2 - theta1;
		while (dtheta > 3.14159)
			dtheta -= 6.28318;
		while (dtheta < -3.14159)
			dtheta += 6.28318;

		return(dtheta);
	}

	int inMultiPartPoly(std::vector<float> xs, std::vector<float> ys, int startVert, int endVert, float testX, float testY)
	{
		int i;
		double angle = 0;
		float p1x, p1y, p2x, p2y;
		int n = endVert - startVert;

		for (i = startVert; i<endVert; i++)
		{
			p1x = xs.at(i) - testX;
			p1y = ys.at(i) - testY;
			p2x = xs.at(startVert + ((i + 1) % n)) - testX;
			p2y = ys.at(startVert + ((i + 1) % n)) - testY;
			angle += Angle2D(p1x, p1y, p2x, p2y);
		}

		if (abs(angle) < 3.14159)
			return 0;
		else
			return 1;
	}
};

#endif
