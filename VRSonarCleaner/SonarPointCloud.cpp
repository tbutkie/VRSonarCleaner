#include "SonarPointCloud.h"

#include "GLSLpreamble.h"
#include <numeric>
#include <limits>

SonarPointCloud::SonarPointCloud(ColorScaler * const colorScaler, std::string fileName)
	: Dataset(false)
	, m_pColorScaler(colorScaler)
	, m_iPreviewReductionFactor(20)
	, m_bPointsAllocated(false)
	, refreshNeeded(true)
	, previewRefreshNeeded(true)
	, colorMode(1) //0=predefined 1=scaled
	, colorScale(2)
	, colorScope(1) //0=global 1=dynamic
	, m_fMinPositionalTPU(std::numeric_limits<float>::max())
	, m_fMaxPositionalTPU(std::numeric_limits<float>::min())
	, m_fMinDepthTPU(std::numeric_limits<float>::max())
	, m_fMaxDepthTPU(std::numeric_limits<float>::min())
{
	loadFromSonarTxt(fileName);
}

SonarPointCloud::~SonarPointCloud()
{	
}

void SonarPointCloud::initPoints(int numPointsToAllocate)
{
	m_nPoints = numPointsToAllocate;
	if (m_bPointsAllocated)
	{
		m_vvec3RawPointsPositions.clear();
		m_vvec3AdjustedPointsPositions.clear();
		m_vvec4PointsColors.clear();
		m_vuiPointsMarks.clear();
		m_vfPointsDepthTPU.clear();
		m_vfPointsPositionTPU.clear();
		m_vuiIndicesFull.clear();
	}

	m_vvec3RawPointsPositions.resize(m_nPoints);
	m_vvec3AdjustedPointsPositions.resize(m_nPoints);
	m_vvec4PointsColors.resize(m_nPoints);
	m_vuiPointsMarks.resize(m_nPoints);
	m_vfPointsDepthTPU.resize(m_nPoints);
	m_vfPointsPositionTPU.resize(m_nPoints);
	m_vuiIndicesFull.resize(m_nPoints);

	m_bPointsAllocated = true;
}

void SonarPointCloud::setPoint(int index, double lonX, double latY, double depth)
{
	glm::dvec3 pt(lonX, latY, depth);
	m_vvec3RawPointsPositions[index] = pt;
	
	m_vvec4PointsColors[index] = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);

	m_vfPointsDepthTPU[index] = 0.f;
	m_vfPointsPositionTPU[index] = 0.f;
	m_vuiPointsMarks[index] = 0u;
	
	checkNewRawPosition(pt);
}


void SonarPointCloud::setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU)
{
	glm::dvec3 pt(lonX, latY, depth); 
	m_vvec3RawPointsPositions[index] = pt;

	float r, g, b;
	m_pColorScaler->getBiValueScaledColor(depthTPU, positionTPU, &r, &g, &b);
	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);

	m_vfPointsDepthTPU[index] = depthTPU;
	m_vfPointsPositionTPU[index] = positionTPU;

	m_vuiPointsMarks[index] = 0u;

	checkNewRawPosition(pt);

	if (depthTPU < m_fMinDepthTPU)
		m_fMinDepthTPU = depthTPU;
	if (depthTPU > m_fMaxDepthTPU)
		m_fMaxDepthTPU = depthTPU;

	if (positionTPU < m_fMinPositionalTPU)
		m_fMinPositionalTPU = positionTPU;
	if (positionTPU > m_fMaxPositionalTPU)
		m_fMaxPositionalTPU = positionTPU;
}


void SonarPointCloud::setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b)
{
	m_vvec3RawPointsPositions[index] = glm::vec3(lonX, latY, depth);

	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);

	m_vfPointsDepthTPU[index] = 0.f;
	m_vfPointsPositionTPU[index] = 0.f;
	m_vuiPointsMarks[index] = 0u;
}


bool SonarPointCloud::loadFromSonarTxt(std::string filename)
{
	
	printf("Loading Point Cloud from %s\n", filename);

	m_strName = std::string(filename);
		
	FILE *file;
	file = fopen(filename.c_str(), "r");
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
		unsigned int numPointsInFile = 0u;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
			numPointsInFile++;

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
		GLuint index = 0u;
		double averageDepth =  0.0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index++, x, y, depth, depthTPU, positionTPU);
			averageDepth += depth;
		}
		averageDepth /= m_nPoints;

		std::iota(m_vuiIndicesFull.begin(), m_vuiIndicesFull.end(), 0u);

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", getRawXMin(), getRawXMax());
		printf("Y Min: %f Max: %f\n", getRawYMin(), getRawYMax());
		printf("Depth Min: %f Max: %f\n", getRawZMin(), getRawZMax());
		printf("Depth Avg: %f\n", averageDepth);

		fclose(file);
		
		adjustPoints();

		setRefreshNeeded();

		createAndLoadBuffers();
	}

	return true;
}

bool SonarPointCloud::generateFakeCloud(float xSize, float ySize, float zSize, int numPoints)
{
	m_strName = "fakeCloud";
	
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

	setRefreshNeeded();

	return true;
}

void SonarPointCloud::update()
{
	if (refreshNeeded)
	{
		// Sub buffer data for colors...
		glNamedBufferSubData(m_glVBO, m_vvec3AdjustedPointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), &m_vvec4PointsColors[0]);

		refreshNeeded = false;
	}
}

GLuint SonarPointCloud::getVAO()
{
	return m_glVAO;
}

GLsizei SonarPointCloud::getPointCount()
{
	return m_nPoints;
}

GLuint SonarPointCloud::getPreviewVAO()
{
	return m_glPreviewVAO;
}

GLsizei SonarPointCloud::getPreviewPointCount()
{
	return m_nPoints / m_iPreviewReductionFactor;
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
	return m_vvec3AdjustedPointsPositions;
}

float SonarPointCloud::getMinDepthTPU()
{
	return m_fMinDepthTPU;
}
float SonarPointCloud::getMaxDepthTPU()
{
	return m_fMaxDepthTPU;
}
float SonarPointCloud::getMinPositionalTPU()
{
	return m_fMinPositionalTPU;
}
float SonarPointCloud::getMaxPositionalTPU()
{
	return m_fMaxPositionalTPU;
}

std::string SonarPointCloud::getName()
{
	return m_strName;
}

void SonarPointCloud::setName(std::string name)
{
	m_strName = name;
}

void SonarPointCloud::adjustPoints()
{
	glm::dvec3 adjustment = getDataCenteringAdjustments();

	for (int i = 0; i < m_nPoints; ++i)
		m_vvec3AdjustedPointsPositions[i] = m_vvec3RawPointsPositions[i] + adjustment;		
}

void SonarPointCloud::createAndLoadBuffers()
{
	// Create data buffer, allocate storage, and upload initial data
	glCreateBuffers(1, &m_glVBO);
	glNamedBufferStorage(m_glVBO, m_nPoints * sizeof(glm::vec3) + m_nPoints * sizeof(glm::vec4), NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(m_glVBO, 0, m_nPoints * sizeof(glm::vec3), &m_vvec3AdjustedPointsPositions[0]);
	glNamedBufferSubData(m_glVBO, m_nPoints * sizeof(glm::vec3), m_nPoints * sizeof(glm::vec4), &m_vvec4PointsColors[0]);

	// Create index buffer, allocate storage, and upload initial data
	glCreateBuffers(1, &m_glEBO);
	glNamedBufferStorage(m_glEBO, m_nPoints * sizeof(GLuint), &m_vuiIndicesFull[0], GL_DYNAMIC_STORAGE_BIT);

	// Create main VAO
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(m_nPoints * sizeof(glm::vec3)));
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
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * m_iPreviewReductionFactor, (GLvoid*)(m_nPoints * sizeof(glm::vec3)));
	glBindVertexArray(0);
}

void SonarPointCloud::markPoint(int index, int code)
{
	m_vuiPointsMarks[index] = code;			
	
	if (code == 1)
	{
		m_vvec4PointsColors[index] = glm::vec4(0.f);
	}
	else
	{
		float r, g, b;
		m_pColorScaler->getBiValueScaledColor(m_vfPointsDepthTPU[index], m_vfPointsPositionTPU[index], &r, &g, &b);

		if (m_vuiPointsMarks[index] == 2)
		{
			r = 1.0;
			g = 0.0;
			b = 0.0;
		}
		if (m_vuiPointsMarks[index] == 3)
		{
			r = 0.0;
			g = 1.0;
			b = 0.0;
		}
		if (m_vuiPointsMarks[index] == 4)
		{
			r = 0.0;
			g = 0.0;
			b = 1.0;
		}

		if (m_vuiPointsMarks[index] >= 100)
		{
			r = (1.f / r) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
			g = (1.f / g) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
			b = (1.f / b) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
		}
		m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);
	}

	setRefreshNeeded();
}

void SonarPointCloud::resetAllMarks()
{
	for (int i = 0; i < m_nPoints; i++)
		markPoint(i, 0);	
}

int SonarPointCloud::getPointMark(int index)
{
	return m_vuiPointsMarks[index];
}