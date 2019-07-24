#include "CosmoGrid.h"
#include "Renderer.h"
#include <iostream>

using namespace std::chrono_literals;

CosmoGrid::CosmoGrid(const char * dataDir)
	: Dataset(dataDir)
	, m_fXMin(0.f)
	, m_fXMax(1.f)
	, m_nXPoints(400)
	, m_fYMin(0.f)
	, m_fYMax(1.f)
	, m_nYPoints(400)
	, m_fZMin(0.f)
	, m_fZMax(1.f)
	, m_nZPoints(400)
{
	std::string filename = std::string(dataDir) + "/vectors_400.bov";

	FILE *inputFile;
	printf("Opening binary file as CosmoGrid: %s\n", filename.c_str());
	
	inputFile = fopen(filename.c_str(), "rb");

	if (inputFile == NULL)
	{
		printf("Unable to open binary input file!");
		return;
	}

	setName(filename.c_str());

	checkNewPosition(glm::dvec3(m_fXMin, m_fYMin, m_fZMin));
	checkNewPosition(glm::dvec3(m_fXMax, m_fYMax, m_fZMax));

	init();

	float tempU, tempV, tempW;
	for (int x = 0; x < m_nXPoints; x++)
	{
		for (int y = 0; y < m_nYPoints; y++)
		{
			for (int z = 0; z < m_nZPoints; z++)
			{
				fread(&tempU, sizeof(float), 1, inputFile);
				fread(&tempV, sizeof(float), 1, inputFile);
				fread(&tempW, sizeof(float), 1, inputFile);
				setPointUVWValue(x, y, z, tempU, tempV, tempW);
				glm::vec3 flowNorm = glm::normalize(glm::vec3(tempU, tempV, tempW));
			}//end for z
		}//end for y
	}//end for x

	fclose(inputFile);

	printf("Imported CosmoGrid from binary file %s\n", filename.c_str());


	m_fAvgVelocity /= m_nXYZPoints;

	std::cout << "Velocity:" << std::endl;
	std::cout << "\tmin=" << m_fMinVelocity << std::endl;
	std::cout << "\tmax=" << m_fMaxVelocity << std::endl;
	std::cout << "\trng=" << m_fMaxVelocity - m_fMinVelocity << std::endl;
	std::cout << "\tavg=" << m_fAvgVelocity << std::endl;

	m_bLoaded = true;
}

CosmoGrid::~CosmoGrid()
{
	delete[] m_arrfUValues;
	delete[] m_arrfVValues;
	delete[] m_arrfWValues;
	delete[] m_arrfVelocityValues;
}

void CosmoGrid::init()
{
	m_nGridSize2d = m_nXPoints * m_nYPoints;
	m_nGridSize3d = m_nXPoints * m_nYPoints * m_nZPoints;
	m_nXYPoints = m_nXPoints * m_nYPoints;
	m_nXYZPoints = m_nXPoints * m_nYPoints * m_nZPoints;
	m_fXRange = glm::abs(m_fXMax - m_fXMin);
	m_fYRange = glm::abs(m_fYMax - m_fYMin);
	m_fZRange = glm::abs(m_fZMax - m_fZMin);

	m_fXCellSize = m_fXRange / static_cast<float>(m_nXPoints - 1);
	m_fYCellSize = m_fYRange / static_cast<float>(m_nYPoints - 1);
	m_fZCellSize = m_fZRange / static_cast<float>(m_nZPoints - 1);

	m_fMinVelocity = std::numeric_limits<float>::max();
	m_fMaxVelocity = std::numeric_limits<float>::min();
	m_fAvgVelocity = 0.f;
	

	m_arrfUValues = new float[m_nGridSize3d];
	m_arrfVValues = new float[m_nGridSize3d];
	m_arrfWValues = new float[m_nGridSize3d];
	m_arrfVelocityValues = new float[m_nGridSize3d];
}

void CosmoGrid::setPointUVWValue(int x, int y, int z, float u, float v, float w)
{
	int index3d = gridIndex(x, y, z);

	m_arrfUValues[index3d] = u;
	m_arrfVValues[index3d] = v;
	m_arrfWValues[index3d] = w;
	m_arrfVelocityValues[index3d] = sqrt(u*u + v*v + w*w);

	m_fAvgVelocity += m_arrfVelocityValues[index3d];

	if (m_arrfVelocityValues[index3d] < m_fMinVelocity)
		m_fMinVelocity = m_arrfVelocityValues[index3d];
	
	if (m_arrfVelocityValues[index3d] > m_fMaxVelocity)
		m_fMaxVelocity = m_arrfVelocityValues[index3d];	
}

glm::vec3 CosmoGrid::getUVWat(glm::vec3 pos)
{
	if (!contains(pos))
		return glm::vec3();	
	
	return glm::vec3(
		trilinear(&m_arrfUValues, pos),
		trilinear(&m_arrfVValues, pos),
		trilinear(&m_arrfWValues, pos)
	);
}

float CosmoGrid::getVelocityAt(glm::vec3 pos)
{
	if (!contains(pos))
		return 0.f;
	
	return trilinear(&m_arrfVelocityValues, pos);
}

float CosmoGrid::getMinVelocity()
{
	return m_fMinVelocity;
}

float CosmoGrid::getMaxVelocity()
{
	return m_fMaxVelocity;
}

float CosmoGrid::getAvgVelocity()
{
	return m_fAvgVelocity;
}

bool CosmoGrid::contains(glm::vec3 pos)
{
	if (pos.x < m_fXMin || pos.x > m_fXMax ||
		pos.y < m_fYMin || pos.y > m_fYMax ||
		pos.z < m_fZMin || pos.z > m_fZMax)
		return false;
	
	return true;
}

glm::vec3 CosmoGrid::rk4(glm::vec3 pos, float delta)
{
	if (!contains(pos))
		return pos;

	glm::vec3 k1 = getUVWat(pos);

	glm::vec3 y1 = pos + k1 * delta * 0.5f;

	glm::vec3 k2 = getUVWat(y1);

	glm::vec3 y2 = pos + k2 * delta * 0.5f;

	glm::vec3 k3 = getUVWat(y2);

	glm::vec3 y3 = pos + k3 * delta;

	glm::vec3 k4 = getUVWat(y3);

	glm::vec3 newPos = pos + delta * (k1 + 2.f * k2 + 2.f * k3 + k4) / 6.f;

	return newPos;
}


float CosmoGrid::getXMin()
{
	return m_fXMin;
}

float CosmoGrid::getXMax()
{
	return m_fXMax;
}

float CosmoGrid::getYMin()
{
	return m_fYMin;
}

float CosmoGrid::getYMax()
{
	return m_fYMax;
}

float CosmoGrid::getZMin()
{
	return m_fZMin;
}

float CosmoGrid::getZMax()
{
	return m_fZMax;
}


float CosmoGrid::getXCellSize()
{
	return m_fXCellSize;
}

float CosmoGrid::getYCellSize()
{
	return m_fYCellSize;
}

float CosmoGrid::getZCellSize()
{
	return m_fZCellSize;
}


char* CosmoGrid::getName()
{
	return m_strName;
}

void CosmoGrid::setName(const char* name)
{
	strcpy(m_strName, name);
}


int CosmoGrid::gridIndex(int xInd, int yInd, int zInd)
{
	return zInd * m_nXYPoints + yInd * m_nXPoints + xInd;
}

float CosmoGrid::trilinear(float ** arr, glm::vec3 pos)
{
	if (!contains(pos))
		return 0.f;

	// Normalized to the grid indices.
	float xn = (pos.x - m_fXMin) / m_fXCellSize;
	float yn = (pos.y - m_fYMin) / m_fYCellSize;
	float zn = (pos.z - m_fZMin) / m_fZCellSize;

	// Indices of grid cell containing point
	unsigned xi = static_cast<int>(xn);
	unsigned yi = static_cast<int>(yn);
	unsigned zi = static_cast<int>(zn);

	// Deltas
	float xt = xn - floor(xn);
	float yt = yn - floor(yn);
	float zt = zn - floor(zn);

	return (1.f - xt) * (1.f - yt) * (1.f - zt) * (*arr)[gridIndex(xi, yi, zi)] +
		xt * (1.f - yt) * (1.f - zt) * (*arr)[gridIndex(xi + 1u, yi, zi)] +
		(1.f - xt) * yt * (1.f - zt) * (*arr)[gridIndex(xi, yi + 1u, zi)] +
		xt * yt * (1.f - zt) * (*arr)[gridIndex(xi + 1u, yi + 1u, zi)] +
		(1.f - xt) * (1.f - yt) * zt * (*arr)[gridIndex(xi, yi, zi + 1u)] +
		xt * (1.f - yt) * zt * (*arr)[gridIndex(xi + 1u, yi, zi + 1u)] +
		(1.f - xt) * yt * zt * (*arr)[gridIndex(xi, yi + 1u, zi + 1u)] +
		xt * yt * zt * (*arr)[gridIndex(xi + 1u, yi + 1u, zi + 1u)];
}
