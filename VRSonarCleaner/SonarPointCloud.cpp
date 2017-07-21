#include "SonarPointCloud.h"

#include "DebugDrawer.h"

SonarPointCloud::SonarPointCloud()
{
	markedForDeletion = false;

	firstMinMaxSet = false;

	xMin = 0;
	xMax = 0;
	xRange = xMax - xMin;
	yMin = 0;
	yMax = 0;
	yRange = yMax - yMin;
	
	minDepth = 123456789;
	maxDepth = -123456789;

	minDepthTPU = 0;
	maxDepthTPU = 0;
	minPositionalTPU = 0;
	maxPositionalTPU = 0;

	colorMode = 1; //0=predefined 1=scaled
	colorScale = 2;
	colorScope = 1; //0=global 1=dynamic

	pointsAllocated = false;

	glewInited = false;

	refreshNeeded = true;
	previewRefreshNeeded = true;
}

SonarPointCloud::~SonarPointCloud()
{
	deleteSelf();
}

void SonarPointCloud::markForDeletion()
{
	markedForDeletion = true;
}

bool SonarPointCloud::shouldBeDeleted()
{
	return markedForDeletion;
}

void SonarPointCloud::deleteSelf()
{
	printf("PointCloud cleaning itself up...\n");
	
	if (pointsAllocated)
	{
		delete[] pointsPositions;
		delete[] pointsColors;
		delete[] pointsMarks;
		delete[] pointsDepthTPU;
		delete[] pointsPositionTPU;
		pointsAllocated = false;
	}
	printf("PointCloud done cleaning itself up!\n");
}

void SonarPointCloud::initPoints(int numPointsToAllocate)
{
	numPoints = numPointsToAllocate;
	if (pointsAllocated)
	{
		delete[] pointsPositions;
		delete[] pointsColors;
		delete[] pointsMarks;
		delete[] pointsDepthTPU;
		delete[] pointsPositionTPU;
	}
	pointsPositions = new double[numPoints*3];
	pointsColors = new float[numPoints*3];
	pointsDepthTPU = new float[numPoints];
	pointsPositionTPU = new float[numPoints];;
	pointsMarks = new int[numPoints];
	pointsAllocated = true;
}

void SonarPointCloud::setPoint(int index, double lonX, double latY, double depth)
{
	pointsPositions[index*3]	 = lonX;
	pointsPositions[(index*3)+1] = latY;
	pointsPositions[(index*3)+2] = depth;
	
	pointsColors[index*3]	 = 0.75;
	pointsColors[(index*3)+1] = 0.75;
	pointsColors[(index*3)+2] = 0.75;

	pointsDepthTPU[index] = 0.0;
	pointsPositionTPU[index] = 0.0;
	pointsMarks[index] = 0;
	
	if (firstMinMaxSet)
	{
		if (lonX < xMin)
			xMin = lonX;
		if (lonX > xMax)
			xMax = lonX;

		if (latY < yMin)
			yMin = latY;
		if (latY > yMax)
			yMax = latY;
		
		if (depth < minDepth)
			minDepth = depth;
		if (depth > maxDepth)
			maxDepth = depth;
	}
	else
	{
		xMin = lonX;
		xMax = lonX;
		yMin = latY;
		yMax = latY;
		minDepth = depth;
		maxDepth = depth;
		firstMinMaxSet = true;
	}

}


void SonarPointCloud::setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU)
{
	pointsPositions[index * 3] = lonX;
	pointsPositions[(index * 3) + 1] = latY;
	pointsPositions[(index * 3) + 2] = depth; 

	pointsColors[index * 3] = 0.75;
	pointsColors[(index * 3) + 1] = 0.75;
	pointsColors[(index * 3) + 2] = 0.75;

	pointsDepthTPU[index] = depthTPU;
	pointsPositionTPU[index] = positionTPU;

	pointsMarks[index] = 0;

	if (firstMinMaxSet)
	{
		if (lonX < xMin)
			xMin = lonX;
		if (lonX > xMax)
			xMax = lonX;

		if (latY < yMin)
			yMin = latY;
		if (latY > yMax)
			yMax = latY;

		if (depth < minDepth)
			minDepth = depth;
		if (depth > maxDepth)
			maxDepth = depth;

		if (depthTPU < minDepthTPU)
			minDepthTPU = depthTPU;
		if (depthTPU > maxDepthTPU)
			maxDepthTPU = depthTPU;

		if (positionTPU < minPositionalTPU)
			minPositionalTPU = positionTPU;
		if (positionTPU > maxPositionalTPU)
			maxPositionalTPU = positionTPU;
	}
	else
	{
		xMin = lonX;
		xMax = lonX;
		yMin = latY;
		yMax = latY;
		minDepth = depth;
		maxDepth = depth;
		minDepthTPU = depthTPU;
		maxDepthTPU = depthTPU;
		minPositionalTPU = positionTPU;
		maxPositionalTPU = positionTPU;
		firstMinMaxSet = true;
	}

}


void SonarPointCloud::setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b)
{
	pointsPositions[index*3]	 = lonX;
	pointsPositions[(index*3)+1] = latY;
	pointsPositions[(index*3)+2] = depth;
	
	pointsColors[index*3]	 = r;
	pointsColors[(index*3)+1] = g;
	pointsColors[(index*3)+2] = b;

	pointsDepthTPU[index] = 0.0;
	pointsPositionTPU[index] = 0.0;
	pointsMarks[index] = 0;
}


bool SonarPointCloud::loadFromSonarTxt(char* filename)
{
	
	printf("Loading Point Cloud from %s\n", filename);

	strcpy(name, filename);

		
	FILE *file;
	file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("ERROR reading file in loadFromASCII!\n");
	}
	else
	{
		//count points
		//skip the first linesToSkip lines
		int skipped = 0;
		int tries = 0;
		char tempChar;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}
		printf("Skipped %d characters\n", skipped);

		//now count lines of points
		double x, y, z;
		int profnum, beamnum;
		float depthTPU, positionTPU, alongAngle, acrossAngle;
		int numPointsInFile = 0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &z, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			numPointsInFile++;
		}
		initPoints(numPointsInFile);
		printf("found %d lines of points\n", numPointsInFile);

		//rewind
		rewind(file);
		//skip the first linesToSkip lines
		skipped = 0;
		tries = 0;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}

		//now load lines of points
		int index = 0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &z, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index, x, y, z, depthTPU, positionTPU);
			index++;
		}

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", xMin, xMax);
		printf("Y Min: %f Max: %f\n", yMin, yMax);
		printf("ZDepth Min: %f Max: %f\n", minDepth, maxDepth);
		double averageDepth =  0;
		for (int i = 0; i<numPoints; i++)
		{
			averageDepth += pointsPositions[(i * 3) + 2];
		}
		averageDepth /= numPoints;
		printf("ZDepth Avg: %f\n", averageDepth);



		fclose(file);

		//scaling hack
		for (int i = 0; i<numPoints; i++)
		{
			pointsPositions[i * 3] = (pointsPositions[i * 3] - xMin);
			pointsPositions[(i * 3) + 1] = (pointsPositions[(i * 3) + 1] - yMin);
			pointsPositions[(i * 3) + 2] = -pointsPositions[(i * 3) + 2];
		}
		actualRemovedXmin = xMin;
		actualRemovedYmin = yMin;
		xMin = pointsPositions[0];
		xMax = pointsPositions[0];
		yMin = pointsPositions[1];
		yMax = pointsPositions[1];
		minDepth = pointsPositions[2];
		maxDepth = pointsPositions[2];

		for (int i = 0; i<numPoints; i++)
		{
			if (pointsPositions[i * 3] < xMin)
				xMin = pointsPositions[i * 3];
			if (pointsPositions[i * 3] > xMax)
				xMax = pointsPositions[i * 3];

			if (pointsPositions[(i * 3) + 1] < yMin)
				yMin = pointsPositions[(i * 3) + 1];
			if (pointsPositions[(i * 3) + 1] > yMax)
				yMax = pointsPositions[(i * 3) + 1];

			if (pointsPositions[(i * 3) + 2] < minDepth)
				minDepth = pointsPositions[(i * 3) + 2];
			if (pointsPositions[(i * 3) + 2] > maxDepth)
				maxDepth = pointsPositions[(i * 3) + 2];
		}

		xRange = xMax - xMin;
		yRange = yMax - yMin;
		rangeDepth = maxDepth - minDepth;

		setRefreshNeeded();

		printf("Trimmed Min/Maxes:\n");
		printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
		printf("X Min: %f Max: %f\n", xMin, xMax);
		printf("Y Min: %f Max: %f\n", yMin, yMax);
		printf("ZDepth Min: %f Max: %f\n", minDepth, maxDepth);

	}

	return true;
}

bool SonarPointCloud::generateFakeCloud(float xSize, float ySize, float zSize, int numPoints)
{
	strcpy(name, "fakeCloud");
	
	initPoints(numPoints);

	int index = 0;
	float randX, randY, randZ;
	srand(149124);
	for (int i = 0; i < numPoints; i++)
	{
		if (i == 0)
		{
			randX = 0;
			randY = 0;
			randZ = 0;
		}
		else if (i == 1)
		{
			randX = xSize;
			randY = ySize;
			randZ = zSize;
		}
		else if (i < numPoints*0.40)
		{
			randZ = zSize*(((float)(rand() % 1000)) / 1000);
			randX = xSize*((((float)(rand() % 100)) + 450) / 1000);
			randY = ySize*(((float)(rand() % 1000)) / 1000);
		}
		else
		{
			randZ = zSize*((((float)(rand() % 100)) + 450) / 1000);
			randX = xSize*(((float)(rand() % 1000)) / 1000);
			randY = ySize*(((float)(rand() % 1000)) / 1000);
		}
		setUncertaintyPoint(i, randX, randY, randZ, 0.0, 0.0);
	}

	//scaling hack
	for (int i = 0; i<numPoints; i++)
	{
		pointsPositions[i * 3] = (pointsPositions[i * 3] - xMin);
		pointsPositions[(i * 3) + 1] = (pointsPositions[(i * 3) + 1] - yMin);
		pointsPositions[(i * 3) + 2] = -pointsPositions[(i * 3) + 2];
	}
	actualRemovedXmin = xMin;
	actualRemovedYmin = yMin;
	xMin = pointsPositions[0];
	xMax = pointsPositions[0];
	yMin = pointsPositions[1];
	yMax = pointsPositions[1];
	minDepth = pointsPositions[2];
	maxDepth = pointsPositions[2];

	for (int i = 0; i<numPoints; i++)
	{
		if (pointsPositions[i * 3] < xMin)
			xMin = pointsPositions[i * 3];
		if (pointsPositions[i * 3] > xMax)
			xMax = pointsPositions[i * 3];

		if (pointsPositions[(i * 3) + 1] < yMin)
			yMin = pointsPositions[(i * 3) + 1];
		if (pointsPositions[(i * 3) + 1] > yMax)
			yMax = pointsPositions[(i * 3) + 1];

		if (pointsPositions[(i * 3) + 2] < minDepth)
			minDepth = pointsPositions[(i * 3) + 2];
		if (pointsPositions[(i * 3) + 2] > maxDepth)
			maxDepth = pointsPositions[(i * 3) + 2];
	}

	xRange = xMax - xMin;
	yRange = yMax - yMin;
	rangeDepth = maxDepth - minDepth;

	setRefreshNeeded();

	return true;
}

void SonarPointCloud::draw(ColorScaler * const colorScaler)
{
	if (numPoints > 0) //if we even have points
	{
		double x, y, z;
		float r, g, b;

		for (int i = 0; i<numPoints; i++)
		{
			if (pointsMarks[i] != 1)
			{
				x = pointsPositions[i * 3];//projSettings->getScaledLonX(pointsPositions[i*3]);
				y = pointsPositions[(i * 3) + 1];//projSettings->getScaledLatY(pointsPositions[(i*3)+1]);
				z = pointsPositions[(i * 3) + 2];//projSettings->getScaledDepth(pointsPositions[(i*3)+2]);

				colorScaler->getBiValueScaledColor(pointsDepthTPU[i], pointsPositionTPU[i], &r, &g, &b);

				if (pointsMarks[i] == 2)
				{
					r = 1.0;
					g = 0.0;
					b = 0.0;
				}
				if (pointsMarks[i] == 3)
				{
					r = 0.0;
					g = 1.0;
					b = 0.0;
				}
				if (pointsMarks[i] == 4)
				{
					r = 0.0;
					g = 0.0;
					b = 1.0;
				}

				if (pointsMarks[i] >= 100)
				{
					r = (1.f / r) * (static_cast<float>(pointsMarks[i]) - 100.f) / 100.f;
					g = (1.f / g) * (static_cast<float>(pointsMarks[i]) - 100.f) / 100.f;
					b = (1.f / b) * (static_cast<float>(pointsMarks[i]) - 100.f) / 100.f;
				}

				glm::vec3 pt(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

				DebugDrawer::getInstance().drawPoint(pt, glm::vec4(r, g, b, 1.f));
			}//end if not marked
		}//end for each pt
	}
}

void SonarPointCloud::drawPreview(ColorScaler * const colorScaler)
{
	if (numPoints > 0) //if we even have points
	{
		int reductionFactor = 20;

		double x, y, z;
		float r, g, b;

		for (int i = 0; i<numPoints; i += reductionFactor)
		{
			if (pointsMarks[i] != 1)
			{
				x = pointsPositions[i * 3];
				y = pointsPositions[(i * 3) + 1];
				z = pointsPositions[(i * 3) + 2];

				colorScaler->getBiValueScaledColor(pointsDepthTPU[i], pointsPositionTPU[i], &r, &g, &b);

				glm::vec3 pt(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

				DebugDrawer::getInstance().drawPoint(pt, glm::vec4(r, g, b, 1.f));
			}//end if point not marked
		}//end for each pt
	}
}

bool SonarPointCloud::getRefreshNeeded()
{
	return refreshNeeded;
}

void SonarPointCloud::setRefreshNeeded()
{
	refreshNeeded = true;
	previewRefreshNeeded = true;
}


void SonarPointCloud::setColorScale(int scale)
{
	colorScale = scale;
}

int SonarPointCloud::getColorScale()
{
	return colorScale;
}


void SonarPointCloud::setColorMode(int mode)
{
	colorMode = mode;
}

int SonarPointCloud::getColorMode()
{
	return colorMode;
}

void SonarPointCloud::setColorScope(int scope)
{
	colorScope = scope;
}

int SonarPointCloud::getColorScope()
{
	return colorScope;
}

std::vector<glm::vec3> SonarPointCloud::getPointPositions()
{
	std::vector<glm::vec3> ret;
	for (int i = 0; i < numPoints * 3; i += 3)
	{
		glm::vec3 thisVert = {
			  (float)pointsPositions[i + 0]
			, (float)pointsPositions[i + 1]
			, (float)pointsPositions[i + 2]
		};

		ret.push_back(thisVert);
	}
	return ret;
}

double SonarPointCloud::getXMin()
{
	return xMin;
}

double SonarPointCloud::getXMax()
{
	return xMax;
}

double SonarPointCloud::getYMin()
{
	return yMin;
}

double SonarPointCloud::getYMax()
{
	return yMax;
}

double SonarPointCloud::getMinDepth()
{
	return minDepth;
}

double SonarPointCloud::getMaxDepth()
{
	return maxDepth;
}

double SonarPointCloud::getMinDepthTPU()
{
	return minDepthTPU;
}
double SonarPointCloud::getMaxDepthTPU()
{
	return maxDepthTPU;
}
double SonarPointCloud::getMinPositionalTPU()
{
	return minPositionalTPU;
}
double SonarPointCloud::getMaxPositionalTPU()
{
	return maxPositionalTPU;
}

char* SonarPointCloud::getName()
{
	return name;
}

void SonarPointCloud::setName(char* Name)
{
	strcpy(name, Name);
}

double SonarPointCloud::getActualRemovedXMin()
{
	return actualRemovedXmin;
}

double SonarPointCloud::getActualRemovedYMin()
{
	return actualRemovedYmin;
}


void SonarPointCloud::drawAxes()
{
	DebugDrawer::getInstance().drawTransform(0.1f);
}

void SonarPointCloud::useNewActualRemovedMinValues(double newRemovedXmin, double newRemovedYmin)
{
	double adjustmentX = actualRemovedXmin - newRemovedXmin;
	double adjustmentY = actualRemovedYmin - newRemovedYmin;

	printf("adjusting cloud by %f, %f\n", adjustmentX, adjustmentY);

	actualRemovedXmin = newRemovedXmin;
	actualRemovedYmin = newRemovedYmin;

	//adjust bounds
	xMin += adjustmentX;
	xMax += adjustmentX; 
	yMin += adjustmentY;
	yMax += adjustmentY;

	//shouldn't have to do this, but just in case of precision errors
	xRange = xMax - xMin;
	yRange = yMax - yMin;

	//adjust point positions
	for (int i = 0; i < numPoints; i++)
	{
		pointsPositions[i * 3] += adjustmentX;
		pointsPositions[(i * 3) + 1] += adjustmentY;
	}

	setRefreshNeeded();
}

void SonarPointCloud::markPoint(int index, int code)
{
	pointsMarks[index] = code;
}

void SonarPointCloud::resetAllMarks()
{
	for (int i = 0; i < numPoints; i++)
	{
		pointsMarks[i] = 0;
	}
	setRefreshNeeded();
}

int SonarPointCloud::getPointMark(int index)
{
	return pointsMarks[index];
}