#ifndef __CloudCollection_h__
#define __CloudCollection_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "ColorScaler.h"
#include <vector>
#include "SonarPointCloud.h"

class CloudCollection
{
public:
	CloudCollection(ColorScaler *colorScaler);
	~CloudCollection();

	void loadCloud(char* filename);
	void generateFakeTestCloud(float sizeX, float sizeY, float sizeZ, int numPoints);
	void clearAllClouds();
	void calculateCloudBoundsAndAlign();

	SonarPointCloud* getCloud(int index);
	int getNumClouds();

	void updateClouds();

	void resetMarksInAllClouds();

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
	ColorScaler *m_pColorScaler;
	std::vector <SonarPointCloud*> clouds;

	double xMin, xMax, xRange;
	double yMin, yMax, yRange;
	double minDepth, maxDepth, rangeDepth;
	double actualRemovedXmin, actualRemovedYmin; //stores the actual x and y min of the original data, we subtract them to keep scaling easier for opengl
	float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;
};

#endif
#pragma once
