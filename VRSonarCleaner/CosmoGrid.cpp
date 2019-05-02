#include "CosmoGrid.h"

using namespace std::chrono_literals;

CosmoGrid::CosmoGrid(const char * filename)
	: Dataset(filename)
	, m_fMinTime(-1.f)
	, m_fMaxTime(-1.f)
	, m_fXMin(0.f)
	, m_fXMax(1.f)
	, m_nXCells(400)
	, m_fYMin(0.f)
	, m_fYMax(1.f)
	, m_nYCells(400)
	, m_fZMin(0.f)
	, m_fZMax(1.f)
	, m_nZCells(400)
	, m_nTimesteps(1)
{
	FILE *inputFile;
	printf("Opening binary file as CosmoGrid: %s\n", filename);

	inputFile = fopen(filename, "rb");

	if (inputFile == NULL)
	{
		printf("Unable to open binary input file!");
		return;
	}

	setName(filename);

	checkNewPosition(glm::dvec3(m_fXMin, m_fYMin, m_fZMin));
	checkNewPosition(glm::dvec3(m_fXMax, m_fYMax, m_fZMax));

	init();

	for (int i = 0; i < m_nTimesteps; i++)
	{
		setTimeValue(i, 1.f);
	}

	int index4d;
	float tempU, tempV, tempW;
	for (int x = 0; x < m_nXCells; x++)
	{
		for (int y = 0; y < m_nYCells; y++)
		{
			for (int z = 0; z < m_nZCells; z++)
			{
				for (int t = 0; t < m_nTimesteps; t++)
				{
					index4d = (t*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
					fread(&tempU, sizeof(float), 1, inputFile);
					fread(&tempV, sizeof(float), 1, inputFile);
					fread(&tempW, sizeof(float), 1, inputFile);
					setCellValue(x, y, z, t, tempU, tempV, tempW);
				}//end for z
			}//end for z
		}//end for y
	}//end for x

	fclose(inputFile);

	m_bLoaded = true;

	printf("Imported CosmoGrid from binary file %s\n", filename);
}

CosmoGrid::~CosmoGrid()
{
	delete[] m_arrfTimes;
	delete[] m_arrfUValues;
	delete[] m_arrfVValues;
	delete[] m_arrfWValues;
	delete[] m_arrfVelocityValues;
}

void CosmoGrid::init()
{
	m_nActiveTimestep = -1;
	m_arrfTimes = new float[m_nTimesteps];
	m_nGridSize2d = m_nXCells * m_nYCells;
	m_nGridSize3d = m_nXCells * m_nYCells * m_nZCells;
	m_nGridSize4d = m_nXCells * m_nYCells * m_nZCells * m_nTimesteps;
	m_nXYCells = m_nXCells * m_nYCells;
	m_nXYZCells = m_nXCells * m_nYCells * m_nZCells;
	m_fXCells = (float)m_nXCells;
	m_fYCells = (float)m_nYCells;
	m_fZCells = (float)m_nZCells;
	m_fXRange = glm::abs(m_fXMax - m_fXMin);
	m_fYRange = glm::abs(m_fYMax - m_fYMin);
	m_fZRange = glm::abs(m_fZMax - m_fZMin);

	m_fXCellSize = m_fXRange / m_fXCells;
	m_fYCellSize = m_fYRange / m_fYCells;
	m_fZCellSize = m_fZRange / m_fZCells;
	
	m_fMaxVelocity = 0;

	m_arrfUValues = new float[m_nGridSize4d];
	m_arrfVValues = new float[m_nGridSize4d];
	m_arrfWValues = new float[m_nGridSize4d];
	m_arrfVelocityValues = new float[m_nGridSize4d];

	m_fLastTimeRequested = -1.f;
}


void CosmoGrid::setTimeValue(int timeIndex, float timeValue)
{
	m_arrfTimes[timeIndex] = timeValue;
	if (timeValue < m_fMinTime || m_fMinTime == -1.f)
		m_fMinTime = timeValue;
	if (timeValue > m_fMaxTime || m_fMaxTime == -1.f)
		m_fMaxTime = timeValue;
}

void CosmoGrid::setCellValue(int x, int y, int z, int timestep, float u, float v, float w)
{
	int index4d = (timestep*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
	m_arrfUValues[index4d] = u;
	m_arrfVValues[index4d] = v;
	m_arrfWValues[index4d] = w;
	m_arrfVelocityValues[index4d] = sqrt(u*u + v*v + w*w);
	
	if (m_arrfVelocityValues[index4d] > m_fMaxVelocity)
		m_fMaxVelocity = m_arrfVelocityValues[index4d];
}

bool CosmoGrid::getUVWat(float x, float y, float z, float time, float *u, float *v, float *w)
{
	if (x < m_fXMin || x > m_fXMax ||
		y < m_fYMin || y > m_fYMax ||
		z < m_fZMin || z > m_fZMax)
		return false;

	//first we check time requested to see if its the same as last time
	if (time != m_fLastTimeRequested)
	{
		//new time, have to find time values anew
		m_fLastTimeRequested = time;
		
		//find if on timestep exactly, or is between what times
		int above = -1;
		int below = -1;
		m_bLastTimeOnTimestep = false;
		for (int i=0;i<m_nTimesteps;i++)
		{
			if (time > m_arrfTimes[i])
			{
				below = i;
			}
			else if (time == m_arrfTimes[i])
			{
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = i;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = i;
				m_fLastTimeFactor2 = 0;
				above = i;
				below = i;
				break;
			}
			else if (time < m_arrfTimes[i])
			{
				above = i;
				break;
			}
		}
		if (!m_bLastTimeOnTimestep) //if didn't match a timestep exactly
		{
			//if was below min time
			if (below == -1)
			{
				//set to min time
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = 0;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = 0;
				m_fLastTimeFactor2 = 0;
			}
			//if was above max time
			else if (above == -1)
			{
				//set to max time
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = m_nTimesteps-1;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = m_nTimesteps-1;
				m_fLastTimeFactor2 = 0;
			}
			else //is in between two timesteps
			{
				//set both times and factor for each
				float range = m_arrfTimes[above] - m_arrfTimes[below];
				float fromBelow = time - m_arrfTimes[below];
				float factor = fromBelow / range;
				m_iLastTime1 = below;
				m_fLastTimeFactor1 = 1-factor;
				m_iLastTime2 = above;
				m_fLastTimeFactor2 = factor;
			}
		}//end if !lastTimeOnTimestep

	}//if time not same as last time
	//now we have time(s) and factor(s)

	//get index of requested location
	int xInd = (int)floor(((x - m_fXMin) / m_fXCellSize));
	int yInd = (int)floor(((y - m_fYMin) / m_fYCellSize));
	int zInd = (int)floor(((z - m_fZMin) / m_fZCellSize));

	//check if in bounds of the dataset
	if (xInd < 0 || yInd < 0 || zInd < 0 || xInd > m_nXCells-1 || yInd > m_nYCells-1 || zInd > m_nZCells-1)
		return false;

	//xyz index
	int index3d = ((zInd*m_nXYCells)+(yInd*m_nXCells)+(xInd));

	//if single timestep
	if (m_bLastTimeOnTimestep)
	{
		*u = m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d];
		*v = m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d];
		*w = m_arrfWValues[(m_iLastTime1*m_nXYZCells) + index3d];
	}
	else //between timesteps
	{ 
		//printf("betwen timesteps\n");
		*u = (m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfUValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
		*v = (m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfVValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
		*w = (m_arrfWValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfWValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
	}

	return true;
}//end getUVWat()



bool CosmoGrid::getVelocityAt(float x, float y, float z, float time, float *velocity)
{
	//first we check time requested to see if its the same as last time
	if (time != m_fLastTimeRequested)
	{
		//new time, have to find time values anew
		m_fLastTimeRequested = time;
		
		//find if on timestep exactly, or is between what times
		int above = -1;
		int below = -1;
		m_bLastTimeOnTimestep = false;
		for (int i=0;i<m_nTimesteps;i++)
		{
			if (time > m_arrfTimes[i])
			{
				below = i;
			}
			else if (time == m_arrfTimes[i])
			{
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = i;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = i;
				m_fLastTimeFactor2 = 0;
				above = i;
				below = i;
				break;
			}
			else if (time < m_arrfTimes[i])
			{
				above = i;
				break;
			}
		}
		if (!m_bLastTimeOnTimestep) //if didn't match a timestep exactly
		{
			//if was below min time
			if (below == -1)
			{
				//set to min time
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = 0;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = 0;
				m_fLastTimeFactor2 = 0;
			}
			//if was above max time
			else if (above == -1)
			{
				//set to max time
				m_bLastTimeOnTimestep = true;
				m_iLastTime1 = m_nTimesteps-1;
				m_fLastTimeFactor1 = 1;
				m_iLastTime2 = m_nTimesteps-1;
				m_fLastTimeFactor2 = 0;
			}
			else //is in between two timesteps
			{
				//set both times and factor for each
				float range = m_arrfTimes[above] - m_arrfTimes[below];
				float fromBelow = time - m_arrfTimes[below];
				float factor = fromBelow / range;
				m_iLastTime1 = below;
				m_fLastTimeFactor1 = 1-factor;
				m_iLastTime2 = above;
				m_fLastTimeFactor2 = factor;
			}
		}//end if !lastTimeOnTimestep

	}//if time not same as last time
	
	//now we have time(s) and factor(s)

	//get index of requested location
	int xInd = (int)floor(((x - m_fXMin) / m_fXCellSize) + 0.5);
	int yInd = (int)floor(((y - m_fYMin) / m_fYCellSize) + 0.5);
	int zInd = (int)floor(((z - m_fZMin) / m_fZCellSize) + 0.5);
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (xInd < 0 || yInd < 0 || zInd < 0 || xInd > m_nXCells -1 || yInd > m_nYCells-1 || zInd > m_nZCells-1)
		return false;
	//xyz index
	int index3d = zInd * m_nXYCells + yInd * m_nXCells + xInd;

	//if single timestep
	if (m_bLastTimeOnTimestep)
	{
		*velocity = m_arrfVelocityValues[(m_iLastTime1*m_nXYZCells) + index3d];
	}
	else //between timesteps
	{ 
		*velocity = (m_arrfVelocityValues[(m_iLastTime1*m_nXYZCells) + index3d]* m_fLastTimeFactor1) + (m_arrfVelocityValues[(m_iLastTime2*m_nXYZCells) + index3d]* m_fLastTimeFactor2);
	}

	return true;
}//end getVelocityAt()

bool CosmoGrid::contains(float x, float y, float z)
{
	if (x < m_fXMin)
		return false;
	else if (x > m_fXMax)
		return false;
	else if (y < m_fYMin)
		return false;
	else if (y > m_fYMax)
		return false;
	else if (z < m_fZMin)
		return false;
	else if (z > m_fZMax)
		return false;
	else
		return true;
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

float CosmoGrid::getTimeAtTimestep(int timestep)
{
	return m_arrfTimes[timestep];
}

int CosmoGrid::getNumTimeCells()
{
	return m_nTimesteps;
}

char* CosmoGrid::getName()
{
	return m_strName;
}

void CosmoGrid::setName(const char* name)
{
	strcpy(m_strName, name);
}

