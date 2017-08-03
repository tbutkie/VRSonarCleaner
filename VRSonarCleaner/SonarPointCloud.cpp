#include "SonarPointCloud.h"

#include "DebugDrawer.h"

SonarPointCloud::SonarPointCloud(ColorScaler * const colorScaler)
	: m_pColorScaler(colorScaler)
	, m_iPreviewReductionFactor(20)
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

	refreshNeeded = true;
	previewRefreshNeeded = true;

	glGenBuffers(1, &m_glVBO);

	glGenVertexArrays(1, &m_glVAO);
	glGenBuffers(1, &m_glEBO);

	glBindVertexArray(this->m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
	glBindVertexArray(0);


	glGenVertexArrays(1, &m_glPreviewVAO);
	glGenBuffers(1, &m_glPreviewEBO);
	
	glBindVertexArray(this->m_glPreviewVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glPreviewEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
	glBindVertexArray(0);
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
		m_vvec3PointsPositions.clear();
		m_vvec4PointsColors.clear();
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
		m_vvec3PointsPositions.clear();
		m_vvec4PointsColors.clear();
		delete[] pointsMarks;
		delete[] pointsDepthTPU;
		delete[] pointsPositionTPU;
	}
	m_vvec3PointsPositions.resize(numPoints);
	m_vvec4PointsColors.resize(numPoints);
	pointsDepthTPU = new float[numPoints];
	pointsPositionTPU = new float[numPoints];;
	pointsMarks = new int[numPoints];
	pointsAllocated = true;

	// Buffer orphaning
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
		glBufferData(GL_ARRAY_BUFFER, numPoints * sizeof(glm::vec3) + numPoints * sizeof(glm::vec4), 0, GL_STREAM_DRAW);

	glBindVertexArray(this->m_glVAO);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(numPoints * sizeof(glm::vec3)));
	glBindVertexArray(0);

	glBindVertexArray(this->m_glPreviewVAO);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(numPoints * sizeof(glm::vec3)));
	glBindVertexArray(0);
}

void SonarPointCloud::setPoint(int index, double lonX, double latY, double depth)
{
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, depth);
	
	m_vvec4PointsColors[index] = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);

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
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, depth); 

	float r, g, b;
	m_pColorScaler->getBiValueScaledColor(depthTPU, positionTPU, &r, &g, &b);
	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);

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
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, depth);

	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);

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
		GLushort *inds = new GLushort[numPoints];
		GLushort *previewInds = new GLushort[numPoints / m_iPreviewReductionFactor];
		double averageDepth =  0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &z, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index, x, y, z, depthTPU, positionTPU);
			inds[index] = index;
			if (index % m_iPreviewReductionFactor == 0)
			{
				previewInds[index / m_iPreviewReductionFactor] = index;
			}
			index++;
			averageDepth += z;
		}
		averageDepth /= numPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", xMin, xMax);
		printf("Y Min: %f Max: %f\n", yMin, yMax);
		printf("ZDepth Min: %f Max: %f\n", minDepth, maxDepth);
		printf("ZDepth Avg: %f\n", averageDepth);

		fclose(file);

		//scaling hack
		for (int i = 0; i < numPoints; i++)
		{
			m_vvec3PointsPositions[i].x -= xMin;
			m_vvec3PointsPositions[i].y -= yMin;
			m_vvec3PointsPositions[i].z = -m_vvec3PointsPositions[i].z;
		}
		actualRemovedXmin = xMin;
		actualRemovedYmin = yMin;
		xMin = xMax = m_vvec3PointsPositions.front().x;
		yMin = yMax = m_vvec3PointsPositions.front().y;
		minDepth = maxDepth = m_vvec3PointsPositions.front().y;

		for (int i = 0; i < numPoints; i++)
		{
			if (m_vvec3PointsPositions[i].x < xMin)
				xMin = m_vvec3PointsPositions[i].x;
			if (m_vvec3PointsPositions[i].x > xMax)
				xMax = m_vvec3PointsPositions[i].x;

			if (m_vvec3PointsPositions[i].y < yMin)
				yMin = m_vvec3PointsPositions[i].y;
			if (m_vvec3PointsPositions[i].y > yMax)
				yMax = m_vvec3PointsPositions[i].y;

			if (m_vvec3PointsPositions[i].z < minDepth)
				minDepth = m_vvec3PointsPositions[i].z;
			if (m_vvec3PointsPositions[i].z > maxDepth)
				maxDepth = m_vvec3PointsPositions[i].z;
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

		glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
			// Sub buffer data for points and colors...
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_vvec3PointsPositions.size() * sizeof(glm::vec3), &m_vvec3PointsPositions[0]);
			glBufferSubData(GL_ARRAY_BUFFER, m_vvec3PointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), &m_vvec4PointsColors[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numPoints * sizeof(unsigned short), 0, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numPoints * sizeof(unsigned short), &inds[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glPreviewEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (numPoints / m_iPreviewReductionFactor) * sizeof(unsigned short), 0, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (numPoints / m_iPreviewReductionFactor) * sizeof(unsigned short), &previewInds[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
		m_vvec3PointsPositions[i].x -= xMin;
		m_vvec3PointsPositions[i].y -= yMin;
		m_vvec3PointsPositions[i].z = -m_vvec3PointsPositions[i].z;
	}
	actualRemovedXmin = xMin;
	actualRemovedYmin = yMin;
	xMin = xMax = m_vvec3PointsPositions.front().x;
	yMin = yMax = m_vvec3PointsPositions.front().y;
	minDepth = maxDepth = m_vvec3PointsPositions.front().z;

	for (int i = 0; i<numPoints; i++)
	{
		if (m_vvec3PointsPositions[i].x < xMin)
			xMin = m_vvec3PointsPositions[i].x;
		if (m_vvec3PointsPositions[i].x > xMax)
			xMax = m_vvec3PointsPositions[i].x;

		if (m_vvec3PointsPositions[i].y < yMin)
			yMin = m_vvec3PointsPositions[i].y;
		if (m_vvec3PointsPositions[i].y > yMax)
			yMax = m_vvec3PointsPositions[i].y;

		if (m_vvec3PointsPositions[i].z < minDepth)
			minDepth = m_vvec3PointsPositions[i].z;
		if (m_vvec3PointsPositions[i].z > maxDepth)
			maxDepth = m_vvec3PointsPositions[i].z;
	}

	xRange = xMax - xMin;
	yRange = yMax - yMin;
	rangeDepth = maxDepth - minDepth;

	setRefreshNeeded();

	return true;
}

void SonarPointCloud::update()
{
	if (refreshNeeded)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
			// Sub buffer data for colors...
			glBufferSubData(GL_ARRAY_BUFFER, m_vvec3PointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), &m_vvec4PointsColors[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		refreshNeeded = false;
	}
}

GLuint SonarPointCloud::getVAO()
{
	return m_glVAO;
}

GLsizei SonarPointCloud::getPointCount()
{
	return numPoints;
}

GLuint SonarPointCloud::getPreviewVAO()
{
	return m_glPreviewVAO;
}

GLsizei SonarPointCloud::getPreviewPointCount()
{
	return numPoints / m_iPreviewReductionFactor;
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
	return m_vvec3PointsPositions;
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
		m_vvec3PointsPositions[i].x += adjustmentX;
		m_vvec3PointsPositions[i].y += adjustmentY;
	}

	setRefreshNeeded();
}

void SonarPointCloud::markPoint(int index, int code)
{
	pointsMarks[index] = code;			
	
	if (code == 1)
	{
		m_vvec4PointsColors[index] = glm::vec4(0.f);
	}
	else
	{
		float r, g, b;
		m_pColorScaler->getBiValueScaledColor(pointsDepthTPU[index], pointsPositionTPU[index], &r, &g, &b);

		if (pointsMarks[index] == 2)
		{
			r = 1.0;
			g = 0.0;
			b = 0.0;
		}
		if (pointsMarks[index] == 3)
		{
			r = 0.0;
			g = 1.0;
			b = 0.0;
		}
		if (pointsMarks[index] == 4)
		{
			r = 0.0;
			g = 0.0;
			b = 1.0;
		}

		if (pointsMarks[index] >= 100)
		{
			r = (1.f / r) * (static_cast<float>(pointsMarks[index]) - 100.f) / 100.f;
			g = (1.f / g) * (static_cast<float>(pointsMarks[index]) - 100.f) / 100.f;
			b = (1.f / b) * (static_cast<float>(pointsMarks[index]) - 100.f) / 100.f;
		}
		m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);
	}

	setRefreshNeeded();
}

void SonarPointCloud::resetAllMarks()
{
	for (int i = 0; i < numPoints; i++)
		markPoint(i, 0);	
}

int SonarPointCloud::getPointMark(int index)
{
	return pointsMarks[index];
}