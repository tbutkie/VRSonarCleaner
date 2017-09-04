#ifndef __CloudCollection_h__
#define __CloudCollection_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "Dataset.h"
#include "ColorScaler.h"
#include <vector>
#include <map>
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
	glm::vec3 getCloudOffset(int index);

	void updateClouds();

	void resetMarksInAllClouds();

	float getMinDepthTPU();
	float getMaxDepthTPU();
	float getMinPositionalTPU();
	float getMaxPositionalTPU();
	
private:
	ColorScaler *m_pColorScaler;
	std::vector<SonarPointCloud*> m_vpClouds;
	std::map<SonarPointCloud*, glm::vec3> m_mapOffsets;

	float m_fMinDepthTPU, m_fMaxDepthTPU, m_fMinPositionalTPU, m_fMaxPositionalTPU;
};

#endif
#pragma once
