#include "CloudCollection.h"

CloudCollection::CloudCollection()
{
	clouds = new std::vector <SonarPointCloud*>;
}

CloudCollection::~CloudCollection()
{
	clearAllClouds();
	delete clouds;
}

void CloudCollection::loadCloud(char* filename)
{
	SonarPointCloud* cloud;
	cloud = new SonarPointCloud();
	cloud->loadFromSonarTxt(filename);
	clouds->push_back(cloud);
}

void CloudCollection::calculateCloudBoundsAndAlign()
{
	//find absolute minimum actual removed min values (the min bounds of the whole dataset)
	for (int i = 0; i < clouds->size(); i++)
	{
		if (i == 0)//if first cloud use its values
		{
			actualRemovedXmin = clouds->at(i)->getActualRemovedXMin();
			actualRemovedYmin = clouds->at(i)->getActualRemovedYMin();
		}
		else
		{
			if (clouds->at(i)->getActualRemovedXMin() < actualRemovedXmin)
				actualRemovedXmin = clouds->at(i)->getActualRemovedXMin();
			if (clouds->at(i)->getActualRemovedYMin() < actualRemovedYmin)
				actualRemovedYmin = clouds->at(i)->getActualRemovedYMin();
		}
	}//end for

	printf("Lowest Trimmed Min/Maxes:\n");
	printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
	
	//now we have the actual min bounds, refactor the others with those as their new removed mins
	if (clouds->size() > 1)
	{
		for (int i = 0; i < clouds->size(); i++)
		{
			clouds->at(i)->useNewActualRemovedMinValues(actualRemovedXmin, actualRemovedYmin);
		}
	}//end if

	for (int i = 0; i < clouds->size(); i++)
	{
		if (i == 0)//for the first cloud, just use its bounds
		{
			xMin = clouds->at(i)->getXMin();
			xMax = clouds->at(i)->getXMax();
			yMin = clouds->at(i)->getYMin();
			yMax = clouds->at(i)->getYMax();
			minDepth = clouds->at(i)->getMinDepth();
			maxDepth = clouds->at(i)->getMaxDepth();
			minDepthTPU = clouds->at(i)->getMinDepthTPU();
			maxDepthTPU = clouds->at(i)->getMaxDepthTPU();
			minPositionalTPU = clouds->at(i)->getMinPositionalTPU();
			maxPositionalTPU = clouds->at(i)->getMaxPositionalTPU();
		}
		else //for each additonal cloud being added
		{
			//first check the min removed values and update other clouds as needed
			if (clouds->at(i)->getXMin() < xMin)
				xMin = clouds->at(i)->getXMin();
			if (clouds->at(i)->getXMax() > xMax)
				xMax = clouds->at(i)->getXMax();

			if (clouds->at(i)->getYMin() < yMin)
				yMin = clouds->at(i)->getYMin();
			if (clouds->at(i)->getYMax() > yMax)
				yMax = clouds->at(i)->getYMax();

			if (clouds->at(i)->getMinDepth() < minDepth)
				minDepth = clouds->at(i)->getMinDepth();
			if (clouds->at(i)->getMaxDepth() > maxDepth)
				maxDepth = clouds->at(i)->getMaxDepth();

			if (clouds->at(i)->getMinDepthTPU() < minDepthTPU)
				minDepthTPU = clouds->at(i)->getMinDepthTPU();
			if (clouds->at(i)->getMaxDepthTPU() > maxDepthTPU)
				maxDepthTPU = clouds->at(i)->getMaxDepthTPU();

			if (clouds->at(i)->getMinPositionalTPU() < minPositionalTPU)
				minPositionalTPU = clouds->at(i)->getMinPositionalTPU();
			if (clouds->at(i)->getMaxPositionalTPU() > maxPositionalTPU)
				maxPositionalTPU = clouds->at(i)->getMaxPositionalTPU();
			
		}//end else additonal cloud being added
	}//end for i

	printf("Final Aligned Min/Maxes:\n");
	printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
	printf("X Min: %f Max: %f\n", xMin, xMax);
	printf("Y Min: %f Max: %f\n", yMin, yMax);
	printf("ZDepth Min: %f Max: %f\n", minDepth, maxDepth);


	xRange = xMax - xMin;
	yRange = yMax - yMin;
	rangeDepth = maxDepth - minDepth;
	colorScalerTPU->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU);

}//end calculateCloudBoundsAndAlign()

void CloudCollection::clearAllClouds()
{
	for (int i = 0; i < clouds->size(); i++)
	{
		clouds->at(i)->deleteSelf();
		delete clouds->at(i);
	}
	clouds->clear();
}

int CloudCollection::getNumClouds()
{
	return clouds->size();
}

SonarPointCloud* CloudCollection::getCloud(int index)
{
	return clouds->at(index);
}

double CloudCollection::getXMin()
{
	return xMin;
}

double CloudCollection::getXMax()
{
	return xMax;
}

double CloudCollection::getYMin()
{
	return yMin;
}

double CloudCollection::getYMax()
{
	return yMax;
}

double CloudCollection::getMinDepth()
{
	return minDepth;
}

double CloudCollection::getMaxDepth()
{
	return maxDepth;
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
double CloudCollection::getActualRemovedXMin()
{
	return actualRemovedXmin;
}

double CloudCollection::getActualRemovedYMin()
{
	return actualRemovedYmin;
}

void CloudCollection::drawCloud(int index) 
{
	clouds->at(index)->draw();
}

void CloudCollection::drawAllClouds()
{
	for (int i=0;i<clouds->size();i++)
		clouds->at(i)->draw();
}