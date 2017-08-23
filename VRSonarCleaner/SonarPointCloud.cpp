#include "SonarPointCloud.h"

#include "DebugDrawer.h"

SonarPointCloud::SonarPointCloud(ColorScaler * const colorScaler)
	: m_pColorScaler(colorScaler)
	, m_iPreviewReductionFactor(20)
{
	markedForDeletion = false;

	firstMinMaxSet = false;

	m_dvec3MinBounds = glm::dvec3(0., 0., 123456789.);
	m_dvec3MaxBounds = glm::dvec3(0., 0., -123456789.);
	m_dvec3BoundsRange = m_dvec3MaxBounds - m_dvec3MinBounds;

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
}

void SonarPointCloud::setPoint(int index, double lonX, double latY, double depth)
{
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, -depth);
	
	m_vvec4PointsColors[index] = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);

	pointsDepthTPU[index] = 0.0;
	pointsPositionTPU[index] = 0.0;
	pointsMarks[index] = 0;
	
	if (firstMinMaxSet)
	{
		if (lonX < m_dvec3MinBounds.x)
			m_dvec3MinBounds.x = lonX;
		if (lonX > m_dvec3MaxBounds.x)
			m_dvec3MaxBounds.x = lonX;

		if (latY < m_dvec3MinBounds.y)
			m_dvec3MinBounds.y = latY;
		if (latY > m_dvec3MaxBounds.y)
			m_dvec3MaxBounds.y = latY;
		
		if (depth < m_dvec3MinBounds.z)
			m_dvec3MinBounds.z = depth;
		if (depth > m_dvec3MaxBounds.z)
			m_dvec3MaxBounds.z = depth;
	}
	else
	{
		m_dvec3MinBounds = m_dvec3MaxBounds = glm::dvec3(lonX, latY, depth);
		firstMinMaxSet = true;
	}

}


void SonarPointCloud::setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU)
{
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, -depth); 

	float r, g, b;
	m_pColorScaler->getBiValueScaledColor(depthTPU, positionTPU, &r, &g, &b);
	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);

	pointsDepthTPU[index] = depthTPU;
	pointsPositionTPU[index] = positionTPU;

	pointsMarks[index] = 0;

	if (firstMinMaxSet)
	{
		if (lonX < m_dvec3MinBounds.x)
			m_dvec3MinBounds.x = lonX;
		if (lonX > m_dvec3MaxBounds.x)
			m_dvec3MaxBounds.x = lonX;

		if (latY < m_dvec3MinBounds.y)
			m_dvec3MinBounds.y = latY;
		if (latY > m_dvec3MaxBounds.y)
			m_dvec3MaxBounds.y = latY;

		if (depth < m_dvec3MinBounds.z)
			m_dvec3MinBounds.z = depth;
		if (depth > m_dvec3MaxBounds.z)
			m_dvec3MaxBounds.z = depth;

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
		m_dvec3MinBounds = m_dvec3MaxBounds = glm::dvec3(lonX, latY, depth);
		minDepthTPU = maxDepthTPU = depthTPU;
		minPositionalTPU = maxPositionalTPU = positionTPU;
		firstMinMaxSet = true;
	}

}


void SonarPointCloud::setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b)
{
	m_vvec3PointsPositions[index] = glm::vec3(lonX, latY, -depth);

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
		printf("ERROR reading file in %s\n", __FUNCTION__);
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
		double x, y, depth;
		int profnum, beamnum;
		float depthTPU, positionTPU, alongAngle, acrossAngle;
		int numPointsInFile = 0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
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
		GLushort index = 0;
		m_vusIndicesFull = std::vector<GLushort>(numPoints);
		double averageDepth =  0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index, x, y, depth, depthTPU, positionTPU);
			m_vusIndicesFull[index] = index++;
			averageDepth += depth;
		}
		averageDepth /= numPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", m_dvec3MinBounds.x, m_dvec3MaxBounds.x);
		printf("Y Min: %f Max: %f\n", m_dvec3MinBounds.y, m_dvec3MaxBounds.y);
		printf("Z Min: %f Max: %f\n", -m_dvec3MaxBounds.z, -m_dvec3MinBounds.z);
		printf("Depth Min: %f Max: %f\n", m_dvec3MinBounds.z, m_dvec3MaxBounds.z);
		printf("Depth Avg: %f\n", averageDepth);

		fclose(file);

		//scaling hack
		//for (int i = 0; i < numPoints; i++)
		//{
		//	m_vvec3PointsPositions[i].x -= m_dvec3MinBounds.x;
		//	m_vvec3PointsPositions[i].y -= m_dvec3MinBounds.y;
		//	m_vvec3PointsPositions[i].z -= -m_dvec3MinBounds.z;
		//}
		//actualRemovedXmin = m_dvec3MinBounds.x;
		//actualRemovedYmin = m_dvec3MinBounds.y;
		//m_dvec3MaxBounds -= m_dvec3MinBounds;
		//m_dvec3MinBounds = glm::dvec3(0.);

		m_dvec3BoundsRange = m_dvec3MaxBounds - m_dvec3MinBounds;

		setRefreshNeeded();

		//printf("Trimmed Min/Maxes:\n");
		//printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
		//printf("X Min: %f Max: %f\n", m_dvec3MinBounds.x, m_dvec3MaxBounds.x);
		//printf("Y Min: %f Max: %f\n", m_dvec3MinBounds.y, m_dvec3MaxBounds.y);
		//printf("Z Min: %f Max: %f\n", -m_dvec3MaxBounds.z, -m_dvec3MinBounds.z);
		//printf("Depth Min: %f Max: %f\n", m_dvec3MinBounds.z, m_dvec3MaxBounds.z);

		createAndLoadBuffers();
	}

	return true;
}

bool SonarPointCloud::generateFakeCloud(float xSize, float ySize, float zSize, int numPoints)
{
	strcpy(name, "fakeCloud");
	
	initPoints(numPoints);

	int index = 0;
	float randX, randY, randDepth;
	srand(149124);
	for (int i = 0; i < numPoints; i++)
	{
		if (i == 0)
		{
			randX = 0;
			randY = 0;
			randDepth = 0;
		}
		else if (i == 1)
		{
			randX = xSize;
			randY = ySize;
			randDepth = zSize;
		}
		else if (i < numPoints*0.40)
		{
			randX = xSize*((((float)(rand() % 100)) + 450) / 1000);
			randY = ySize*(((float)(rand() % 1000)) / 1000);
			randDepth = zSize*(((float)(rand() % 1000)) / 1000);
		}
		else
		{
			randX = xSize*(((float)(rand() % 1000)) / 1000);
			randY = ySize*(((float)(rand() % 1000)) / 1000);
			randDepth = zSize*((((float)(rand() % 100)) + 450) / 1000);
		}
		setUncertaintyPoint(i, randX, randY, randDepth, 0.0, 0.0);
	}

	//scaling hack
	//for (auto &pt : m_vvec3PointsPositions)
	//	pt -= glm::dvec3(m_dvec3MinBounds.x, m_dvec3MinBounds.y, -m_dvec3MinBounds.z);
	//
	//actualRemovedXmin = m_dvec3MinBounds.x;
	//actualRemovedYmin = m_dvec3MinBounds.y;
	//m_dvec3MinBounds = m_dvec3MaxBounds = m_vvec3PointsPositions.front();
	//m_dvec3MinBounds.z = -m_dvec3MinBounds.z;
	//m_dvec3MaxBounds.z *= -m_dvec3MaxBounds.z;

	//for (auto const & pos : m_vvec3PointsPositions)
	//{
	//	if (pos.x < m_dvec3MinBounds.x)
	//		m_dvec3MinBounds.x = pos.x;
	//	if (pos.x > m_dvec3MaxBounds.x)
	//		m_dvec3MaxBounds.x = pos.x;
	//
	//	if (pos.y < m_dvec3MinBounds.y)
	//		m_dvec3MinBounds.y = pos.y;
	//	if (pos.y > m_dvec3MaxBounds.y)
	//		m_dvec3MaxBounds.y = pos.y;
	//
	//	if (-pos.z < m_dvec3MinBounds.z)
	//		m_dvec3MinBounds.z = -pos.z;
	//	if (-pos.z > m_dvec3MaxBounds.z)
	//		m_dvec3MaxBounds.z = -pos.z;
	//}

	m_dvec3BoundsRange = m_dvec3MaxBounds - m_dvec3MinBounds;

	setRefreshNeeded();

	return true;
}

void SonarPointCloud::update()
{
	if (refreshNeeded)
	{
		// Sub buffer data for colors...
		glNamedBufferSubData(m_glVBO, m_vvec3PointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), &m_vvec4PointsColors[0]);

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
	return m_dvec3MinBounds.x;
}

double SonarPointCloud::getXMax()
{
	return m_dvec3MaxBounds.x;
}

double SonarPointCloud::getYMin()
{
	return m_dvec3MinBounds.y;
}

double SonarPointCloud::getYMax()
{
	return m_dvec3MaxBounds.y;
}

double SonarPointCloud::getMinDepth()
{
	return m_dvec3MinBounds.z;
}

double SonarPointCloud::getMaxDepth()
{
	return m_dvec3MaxBounds.z;
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

void SonarPointCloud::createAndLoadBuffers()
{
	// Create data buffer, allocate storage, and upload initial data
	glCreateBuffers(1, &m_glVBO);
	glNamedBufferStorage(m_glVBO, numPoints * sizeof(glm::vec3) + numPoints * sizeof(glm::vec4), NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(m_glVBO, 0, m_vvec3PointsPositions.size() * sizeof(glm::vec3), &m_vvec3PointsPositions[0]);
	glNamedBufferSubData(m_glVBO, m_vvec3PointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), &m_vvec4PointsColors[0]);

	// Create index buffer, allocate storage, and upload initial data
	glCreateBuffers(1, &m_glEBO);
	glNamedBufferStorage(m_glEBO, numPoints * sizeof(unsigned short), &m_vusIndicesFull[0], GL_DYNAMIC_STORAGE_BIT);

	// Create main VAO
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(this->m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(numPoints * sizeof(glm::vec3)));
	glBindVertexArray(0);

	// Create preview VAO
	glGenVertexArrays(1, &m_glPreviewVAO);
	glBindVertexArray(m_glPreviewVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * m_iPreviewReductionFactor, (GLvoid*)0);
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * m_iPreviewReductionFactor, (GLvoid*)(numPoints * sizeof(glm::vec3)));
	glBindVertexArray(0);
}

/*
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
	glm::dvec3 adjustment(actualRemovedXmin - newRemovedXmin, actualRemovedYmin - newRemovedYmin, 0.);

	printf("adjusting cloud by %f, %f\n", adjustment.x, adjustment.y);

	actualRemovedXmin = newRemovedXmin;
	actualRemovedYmin = newRemovedYmin;

	//adjust bounds
	m_dvec3MinBounds += adjustment;
	m_dvec3MaxBounds += adjustment;

	//shouldn't have to do this, but just in case of precision errors
	m_dvec3BoundsRange = m_dvec3MaxBounds - m_dvec3MinBounds;

	//adjust point positions
	for (auto &point : m_vvec3PointsPositions)
		point += adjustment;

	setRefreshNeeded();
}
*/
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