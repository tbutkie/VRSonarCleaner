#ifndef __CloudCollection_h__
#define __CloudCollection_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "Dataset.h"
#include "ColorScaler.h"
#include <vector>
#include "SonarPointCloud.h"

class CloudCollection : public Dataset
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

	double getMinDepthTPU();
	double getMaxDepthTPU();
	double getMinPositionalTPU();
	double getMaxPositionalTPU();
	
private:
	ColorScaler *m_pColorScaler;
	std::vector <SonarPointCloud*> clouds;

	float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;
};

#endif
#pragma once
