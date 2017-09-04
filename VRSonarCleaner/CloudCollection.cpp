#include "CloudCollection.h"

#include <limits>

CloudCollection::CloudCollection(ColorScaler *colorScaler)
	: Dataset(false)
	, m_pColorScaler(colorScaler)
	, m_fMinPositionalTPU(std::numeric_limits<float>::max())
	, m_fMaxPositionalTPU(std::numeric_limits<float>::min())
	, m_fMinDepthTPU(std::numeric_limits<float>::max())
	, m_fMaxDepthTPU(std::numeric_limits<float>::min())
{
}

CloudCollection::~CloudCollection()
{
	clearAllClouds();
	m_vpClouds.clear();
}

void CloudCollection::loadCloud(char* filename)
{
	m_vpClouds.push_back(new SonarPointCloud(m_pColorScaler));
	m_vpClouds.back()->loadFromSonarTxt(filename);
}

void CloudCollection::generateFakeTestCloud(float sizeX, float sizeY, float sizeZ, int numPoints)
{
	SonarPointCloud* cloud;
	cloud = new SonarPointCloud(m_pColorScaler);
	cloud->generateFakeCloud(sizeX, sizeY, sizeZ, numPoints);
	m_vpClouds.push_back(cloud);
}

void CloudCollection::calculateCloudBoundsAndAlign()
{
	// Figure out boundaries for all clouds
	for (auto const &cloud : m_vpClouds)
	{
		glm::dvec3 minPos = cloud->getRawMinBounds();
		glm::dvec3 maxPos = cloud->getRawMaxBounds();

		checkNewRawPosition(minPos);
		checkNewRawPosition(maxPos);

		if (cloud->getMinDepthTPU() < m_fMinDepthTPU)
			m_fMinDepthTPU = cloud->getMinDepthTPU();
		if (cloud->getMaxDepthTPU() > m_fMaxDepthTPU)
			m_fMaxDepthTPU = cloud->getMaxDepthTPU();

		if (cloud->getMinPositionalTPU() < m_fMinPositionalTPU)
			m_fMinPositionalTPU = cloud->getMinPositionalTPU();
		if (cloud->getMaxPositionalTPU() > m_fMaxPositionalTPU)
			m_fMaxPositionalTPU = cloud->getMaxPositionalTPU();
	}

	// Assign offsets to each cloud to position it within the collection bounds
	for (auto const &cloud : m_vpClouds)
	{
		m_mapOffsets[cloud] = cloud->getRawMinBounds() - getRawMinBounds();
	}

	printf("Final Adjusted/Aligned Boundaries:\n");
	printf("X Min: %f Max: %f\n", getAdjustedXMin(), getAdjustedXMax());
	printf("Y Min: %f Max: %f\n", getAdjustedYMin(), getAdjustedYMax());
	printf("Z Min: %f Max: %f\n", getAdjustedZMin(), getAdjustedZMax());
}//end calculateCloudBoundsAndAlign()

void CloudCollection::clearAllClouds()
{
	for (auto &cloud : m_vpClouds)
		delete cloud;

	m_vpClouds.clear();
}

int CloudCollection::getNumClouds()
{
	return m_vpClouds.size();
}

glm::vec3 CloudCollection::getCloudOffset(int index)
{
	return m_mapOffsets[m_vpClouds[index]];
}

SonarPointCloud* CloudCollection::getCloud(int index)
{
	return m_vpClouds.at(index);
}

float CloudCollection::getMinDepthTPU()
{
	return m_fMinDepthTPU;
}
float CloudCollection::getMaxDepthTPU()
{
	return m_fMaxDepthTPU;
}
float CloudCollection::getMinPositionalTPU()
{
	return m_fMinPositionalTPU;
}
float CloudCollection::getMaxPositionalTPU()
{
	return m_fMaxPositionalTPU;
}

void CloudCollection::updateClouds()
{
	for (auto const &cloud : m_vpClouds)
		cloud->update();
}

void CloudCollection::resetMarksInAllClouds()
{
	for (auto const &cloud : m_vpClouds)
		cloud->resetAllMarks();
}