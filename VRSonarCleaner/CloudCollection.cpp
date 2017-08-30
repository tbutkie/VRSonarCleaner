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
	for (int i = 0; i < m_vpClouds.size(); i++)
	{
		glm::dvec3 minPos = m_vpClouds.at(i)->getRawMinBounds();
		glm::dvec3 maxPos = m_vpClouds.at(i)->getRawMaxBounds();

		checkNewRawPosition(minPos);
		checkNewRawPosition(maxPos);

		if (m_vpClouds.at(i)->getMinDepthTPU() < m_fMinDepthTPU)
			m_fMinDepthTPU = m_vpClouds.at(i)->getMinDepthTPU();
		if (m_vpClouds.at(i)->getMaxDepthTPU() > m_fMaxDepthTPU)
			m_fMaxDepthTPU = m_vpClouds.at(i)->getMaxDepthTPU();

		if (m_vpClouds.at(i)->getMinPositionalTPU() < m_fMinPositionalTPU)
			m_fMinPositionalTPU = m_vpClouds.at(i)->getMinPositionalTPU();
		if (m_vpClouds.at(i)->getMaxPositionalTPU() > m_fMaxPositionalTPU)
			m_fMaxPositionalTPU = m_vpClouds.at(i)->getMaxPositionalTPU();
	}

	// Assign offsets to each cloud to position it within the collection bounds


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