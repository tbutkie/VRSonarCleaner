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

	m_bIllustrativeParticlesEnabled = true;
	m_nIllustrativeParticles = 10000;
	m_fIllustrativeParticleTrailTime = 500ms;
	m_fIllustrativeParticleLifetime = 2500ms;
	m_fIllustrativeParticleSize = 1.f;

	m_vec3IllustrativeParticlesColor = glm::vec3(0.25f, 0.95f, 1.f);
	m_fIllustrativeParticleVelocityScale = 0.001f;//0.000001;
}

void CosmoGrid::deleteSelf()
{
	delete[] m_arrfTimes;
	delete[] m_arrfUValues;
	delete[] m_arrfVValues;
	delete[] m_arrfWValues;
	delete[] m_arrfVelocityValues;

}


void CosmoGrid::setTimeValue(int timeIndex, float timeValue)
{
	m_arrfTimes[timeIndex] = timeValue;
	if (timeValue < m_fMinTime || m_fMinTime == -1.f)
		m_fMinTime = timeValue;
	if (timeValue > m_fMaxTime || m_fMaxTime == -1.f)
		m_fMaxTime = timeValue;
}

void CosmoGrid::setCellValue(int x, int y, int z, int timestep, float u, float v)
{
	int index4d = (timestep*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
	m_arrfUValues[index4d] = u;
	m_arrfVValues[index4d] = v;
	m_arrfVelocityValues[index4d] = sqrt(u*u + v*v);
	
	if (m_arrfVelocityValues[index4d] > m_fMaxVelocity)
		m_fMaxVelocity = m_arrfVelocityValues[index4d];
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



bool CosmoGrid::getVelocityAt(float lonX, float latY, float depth, float time, float *velocity)
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
	int x = (int)floor(((lonX-m_fXMin)/m_fXCellSize)+0.5);
	int y = (int)floor(((latY- m_fYMin)/ m_fYCellSize)+0.5);
	int z = (int)floor(((depth - m_fZMin) / m_fZCellSize) + 0.5);
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > m_nXCells -1 || y > m_nYCells-1 || z > m_nZCells-1)
		return false;
	//xyz index
	int index3d = ((z*m_nXYCells)+(y*m_nXCells)+(x));

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

bool CosmoGrid::contains(float x, float y)
{
	if (x < m_fXMin)
		return false;
	else if (x > m_fXMax)
		return false;
	else if (y < m_fYMin)
		return false;
	else if (y > m_fYMax)
		return false;
	else
		return true;
}

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


int CosmoGrid::getNumXYCells()
{
	return m_nXYCells;
}

void CosmoGrid::getXYZofCell(int cellIndex, float *lonX, float *latY, float *depth)
{
	int x = cellIndex % m_nXCells;
	int y = cellIndex / m_nXCells;
	int z = cellIndex / m_nZCells;
	
	*lonX = m_fXMin + (x * m_fXCellSize);
	*latY = m_fYMin + (y * m_fYCellSize);
	*depth = m_fZMin + (z * m_fZCellSize);

	//printf("Cell %d of %d is %f, %f, %f\n", cellIndex, gridSize2d, *lonX, *latY, *depth);
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

bool CosmoGrid::getCellBounds(float m_fXMin, float m_fXMax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax)
{
	//find min x cell
	int minXCell;
	if (m_fXMin < m_fXMin) //beyond and contains edge
	{
		minXCell = 0;
	}
	else if (m_fXMin > m_fXMax) //not in bounds, should not happen
	{
		printf("ERROR 15151515 in getCellBounds\n");
		return false;
	}
	else //within
	{
		minXCell = 0;
		float xHere;
		for (int i=0;i<m_nXCells -1;i++)
		{
			xHere = m_fXMin + (m_fXCellSize*i);
			if (xHere < m_fXMin)
				minXCell = i;
			else
				break;
		}
	}


	//find max x cell
	int maxXCell;
	if (m_fXMax > m_fXMax) //beyond and contains edge
	{
		maxXCell = m_nXCells -1;
	}
	else if (m_fXMax < m_fXMin) //not in bounds, should not happen
	{
		printf("ERROR 377257 in getCellBounds\n");
		return false;
	}
	else //within
	{
		maxXCell = m_nXCells -1;
		float xHere;
		for (int i= m_nXCells -1;i>-1;i--)
		{
			xHere = m_fXMin + m_fXCellSize + (m_fXCellSize*i); //extra m_fXCellSize because want far edge
			if (xHere > m_fXMax)
				maxXCell = i;
			else
				break;
		}
	}

	//find min y cell
	int minYCell;
	if (ymin < m_fYMin) //beyond and contains edge
	{
		minYCell = 0;
	}
	else if (ymin > m_fYMax) //not in bounds, should not happen
	{
		printf("ERROR 254572 in getCellBounds\n");
		return false;
	}
	else //within
	{
		minYCell = 0;
		float yHere;
		for (int i=0;i<m_nYCells-1;i++)
		{
			yHere = m_fYMin + (m_fYCellSize*i);
			if (yHere < m_fXMin)
				minYCell = i;
			else
				break;
		}
	}


	//find max y cell
	int maxYCell;
	if (ymax > m_fYMax) //beyond and contains edge
	{
		maxYCell = m_nYCells-1;
	}
	else if (ymax < m_fYMin) //not in bounds, should not happen
	{
		printf("ERROR 546854683 in getCellBounds\n");
		return false;
	}
	else //within
	{
		maxYCell = m_nYCells-1;
		float yHere;
		for (int i=m_nYCells-1;i>-1;i--)
		{
			yHere = m_fYMin + m_fYCellSize + (m_fYCellSize*i); //extra m_fXCellSize because want far edge
			if (yHere > ymax)
				maxYCell = i;
			else
				break;
		}
	}

	//*xcellmin = minXCell;
	//*xcells = maxXCell - minXCell;

	//*ycellmin = minYCell;
	//*ycells = maxYCell - minYCell;

	*xcellmin = std::min(minXCell, maxXCell);
	*xcellmax = std::max(minXCell, maxXCell);

	//*xcells = max(minXCell, maxXCell) - min(minXCell, maxXCell) + 1;

	*ycellmin = std::min(minYCell, maxYCell);
	*ycellmax = std::max(minYCell, maxYCell);
	//*ycells = max(minYCell, maxYCell) - min(minYCell, maxYCell) + 1;

	return true;
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
