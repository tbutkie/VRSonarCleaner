#ifndef __CloudCollection_h__
#define __CloudCollection_h__

#include <windows.h>
#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "Vec3.h"
#include "Vec3Double.h"
#include "ColorScaler.h"
#include <vector>
#include "SonarPointCloud.h"

class CloudCollection
{
public:
	CloudCollection();
	~CloudCollection();

	void loadCloud(char* filename);
	void clearAllClouds();
	void calculateCloudBoundsAndAlign();

	SonarPointCloud* getCloud(int index);
	int getNumClouds();

	void drawCloud(int index);
	void drawAllClouds();

	//bounds access:
	double getXMin();
	double getXMax();
	double getYMin();
	double getYMax();
	double getMinDepth();
	double getMaxDepth();
	double getActualRemovedXMin();
	double getActualRemovedYMin();
	double getMinDepthTPU();
	double getMaxDepthTPU();
	double getMinPositionalTPU();
	double getMaxPositionalTPU();
	
private:
	
	std::vector <SonarPointCloud*> *clouds;

	double xMin, xMax, xRange;
	double yMin, yMax, yRange;
	double minDepth, maxDepth, rangeDepth;
	double actualRemovedXmin, actualRemovedYmin; //stores the actual x and y min of the original data, we subtract them to keep scaling easier for opengl
	float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;
};

#endif
#pragma once
