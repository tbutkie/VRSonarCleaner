#include "CloudCollection.h"

CloudCollection::CloudCollection(ColorScaler *colorScaler)
	: Dataset(false)
	, m_pColorScaler(colorScaler)
{
}

CloudCollection::~CloudCollection()
{
	clearAllClouds();
	clouds.clear();
}

void CloudCollection::loadCloud(char* filename)
{
	clouds.push_back(new SonarPointCloud(m_pColorScaler));
	clouds.back()->loadFromSonarTxt(filename);
}

void CloudCollection::generateFakeTestCloud(float sizeX, float sizeY, float sizeZ, int numPoints)
{
	SonarPointCloud* cloud;
	cloud = new SonarPointCloud(m_pColorScaler);
	cloud->generateFakeCloud(sizeX, sizeY, sizeZ, numPoints);
	clouds.push_back(cloud);
}

void CloudCollection::calculateCloudBoundsAndAlign()
{
	//find absolute minimum actual removed min values (the min bounds of the whole dataset)
	//for (int i = 0; i < clouds.size(); i++)
	//{
	//	if (i == 0)//if first cloud use its values
	//	{
	//		actualRemovedXmin = clouds.at(i)->getActualRemovedXMin();
	//		actualRemovedYmin = clouds.at(i)->getActualRemovedYMin();
	//	}
	//	else
	//	{
	//		if (clouds.at(i)->getActualRemovedXMin() < actualRemovedXmin)
	//			actualRemovedXmin = clouds.at(i)->getActualRemovedXMin();
	//		if (clouds.at(i)->getActualRemovedYMin() < actualRemovedYmin)
	//			actualRemovedYmin = clouds.at(i)->getActualRemovedYMin();
	//	}
	//}//end for
	//
	//printf("Lowest Trimmed Min/Maxes:\n");
	//printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
	//
	////now we have the actual min bounds, refactor the others with those as their new removed mins
	//for (auto &cloud : clouds)
	//	cloud->useNewActualRemovedMinValues(actualRemovedXmin, actualRemovedYmin);

	for (int i = 0; i < clouds.size(); i++)
	{
		if (i == 0)//for the first cloud, just use its bounds
		{
			m_dvec3MinBounds = clouds.at(i)->getMinBounds();
			m_dvec3MaxBounds = clouds.at(i)->getMaxBounds();
			maxDepthTPU = clouds.at(i)->getMaxDepthTPU();
			minPositionalTPU = clouds.at(i)->getMinPositionalTPU();
			maxPositionalTPU = clouds.at(i)->getMaxPositionalTPU();
		}
		else //for each additonal cloud being added
		{
			//first check the min removed values and update other clouds as needed
			if (clouds.at(i)->getXMin() < m_dvec3MinBounds.x)
				m_dvec3MinBounds.x = clouds.at(i)->getXMin();
			if (clouds.at(i)->getXMax() > m_dvec3MaxBounds.x)
				m_dvec3MaxBounds.x = clouds.at(i)->getXMax();

			if (clouds.at(i)->getYMin() < m_dvec3MinBounds.y)
				m_dvec3MinBounds.y = clouds.at(i)->getYMin();
			if (clouds.at(i)->getYMax() > m_dvec3MaxBounds.y)
				m_dvec3MaxBounds.y = clouds.at(i)->getYMax();

			if (clouds.at(i)->getZMin() < m_dvec3MinBounds.z)
				m_dvec3MinBounds.z = clouds.at(i)->getZMin();
			if (clouds.at(i)->getZMax() > m_dvec3MaxBounds.z)
				m_dvec3MaxBounds.z = clouds.at(i)->getZMax();

			if (clouds.at(i)->getMinDepthTPU() < minDepthTPU)
				minDepthTPU = clouds.at(i)->getMinDepthTPU();
			if (clouds.at(i)->getMaxDepthTPU() > maxDepthTPU)
				maxDepthTPU = clouds.at(i)->getMaxDepthTPU();

			if (clouds.at(i)->getMinPositionalTPU() < minPositionalTPU)
				minPositionalTPU = clouds.at(i)->getMinPositionalTPU();
			if (clouds.at(i)->getMaxPositionalTPU() > maxPositionalTPU)
				maxPositionalTPU = clouds.at(i)->getMaxPositionalTPU();
			
		}//end else additonal cloud being added
	}//end for i

	printf("Final Aligned Min/Maxes:\n");
	printf("X Min: %f Max: %f\n", m_dvec3MinBounds.x, m_dvec3MaxBounds.x);
	printf("Y Min: %f Max: %f\n", m_dvec3MinBounds.y, m_dvec3MaxBounds.y);
	printf("Z Min: %f Max: %f\n", -m_dvec3MaxBounds.z, -m_dvec3MinBounds.z);
	printf("Depth Min: %f Max: %f\n", m_dvec3MinBounds.z, m_dvec3MaxBounds.z);

	m_dvec3Dimensions = m_dvec3MaxBounds - m_dvec3MinBounds;
}//end calculateCloudBoundsAndAlign()

void CloudCollection::clearAllClouds()
{
	for (auto &cloud : clouds)
		delete cloud;

	clouds.clear();
}

int CloudCollection::getNumClouds()
{
	return clouds.size();
}

SonarPointCloud* CloudCollection::getCloud(int index)
{
	return clouds.at(index);
}

double CloudCollection::getMinDepthTPU()
{
	return minDepthTPU;
}
double CloudCollection::getMaxDepthTPU()
{
	return maxDepthTPU;
}
double CloudCollection::getMinPositionalTPU()
{
	return minPositionalTPU;
}
double CloudCollection::getMaxPositionalTPU()
{
	return maxPositionalTPU;
}

//double CloudCollection::getActualRemovedXMin()
//{
//	return actualRemovedXmin;
//}
//
//double CloudCollection::getActualRemovedYMin()
//{
//	return actualRemovedYmin;
//}

void CloudCollection::updateClouds()
{
	for (auto const &cloud : clouds)
		cloud->update();
}

void CloudCollection::resetMarksInAllClouds()
{
	for (int i = 0; i < clouds.size(); i++)
		clouds.at(i)->resetAllMarks();
}