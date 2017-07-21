#include "FlowGrid.h"

#include "DebugDrawer.h"

FlowGrid::FlowGrid(char* filename, bool hasZRange)
	: m_fMinTime(-1.f)
	, m_fMaxTime(-1.f)
{
	FILE *inputFile;
	printf("opening: %s\n", filename);
	inputFile = fopen(filename, "rb");
	if (inputFile == NULL)
	{
		printf("Unable to open flowgrid input file!");
		return;
	}

	float zMin, zMax; //temp for now

	fread(&m_fXMin, sizeof(float), 1, inputFile);
	fread(&m_fXMax, sizeof(float), 1, inputFile);
	fread(&m_nXCells, sizeof(int), 1, inputFile);
	fread(&m_fYMin, sizeof(float), 1, inputFile);
	fread(&m_fYMax, sizeof(float), 1, inputFile);
	fread(&m_nYCells, sizeof(int), 1, inputFile);
	if (hasZRange)
	{
		fread(&zMin, sizeof(float), 1, inputFile);
		fread(&zMax, sizeof(float), 1, inputFile);
	}
	else
	{
		zMin = 1.f;
		zMax = 0.f;
	}
	fread(&m_nZCells, sizeof(int), 1, inputFile);
	fread(&m_nTimesteps, sizeof(int), 1, inputFile);

	init();

	float tempDepth;
	for (int i = 0; i < m_nZCells; i++)
	{
		fread(&tempDepth, sizeof(float), 1, inputFile);
		setDepthValue(i, tempDepth);
	}

	float tempTime;
	for (int i = 0; i < m_nTimesteps; i++)
	{
		fread(&tempTime, sizeof(float), 1, inputFile);
		setTimeValue(i, tempTime);
	}

	int index4d;
	int tempIsWater;
	float tempU, tempV, tempW;
	for (int x = 0; x<m_nXCells; x++)
	{
		for (int y = 0; y<m_nYCells; y++)
		{
			for (int z = 0; z<m_nZCells; z++)
			{
				for (int t = 0; t<m_nTimesteps; t++)
				{
					index4d = (t*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
					fread(&tempIsWater, sizeof(int), 1, inputFile);
					fread(&tempU, sizeof(float), 1, inputFile);
					fread(&tempV, sizeof(float), 1, inputFile);
					if (hasZRange)
						fread(&tempW, sizeof(float), 1, inputFile);
					else
						tempW = 0.f;
					setIsWaterValue(x, y, z, t, tempIsWater == 0 ? false : true);
					setCellValue(x, y, z, t, tempU, tempV, tempW);
				}//end for z
			}//end for z
		}//end for y
	}//end for x

	fclose(inputFile);

	printf("Imported FlowGrid from %s\n", filename);

}//end file loading constructor


FlowGrid::~FlowGrid()
{

}

void FlowGrid::init()
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
	m_fXRange = m_fXMax - m_fXMin;
	m_fYRange = m_fYMax - m_fYMin;
	m_vDepthValues.resize(m_nZCells);
	m_bDepthsSet = false;

	m_fXCellSize = m_fXRange / m_fXCells;
	m_fYCellSize = m_fYRange / m_fYCells;

	m_fMaxVelocity = 0;

	//allocate storage arrays
	m_arrbIsWater = new bool[m_nGridSize4d];
	//bathyDepth2d = new float[gridSize2d];

	m_arrfUValues = new float[m_nGridSize4d];
	m_arrfVValues = new float[m_nGridSize4d];
	m_arrfWValues = new float[m_nGridSize4d];
	m_arrfVelocityValues = new float[m_nGridSize4d];
	//tValues = new float[m_nGridSize4d];
	//sValues = new float[m_nGridSize4d];

	m_fLastTimeRequested = -1.f;

	m_bIllustrativeParticlesEnabled = true;
	m_nIllustrativeParticles = 20000;
	m_fIllustrativeParticleTrailTime = 500;
	m_fIllustrativeParticleLifetime = 2500;
	m_fIllustrativeParticleSize = 1;

	m_vec3IllustrativeParticlesColor = glm::vec3(0.25f, 0.95f, 1.f);
	m_fIllustrativeParticleVelocityScale = 0.33f;//0.000001;
}

void FlowGrid::deleteSelf()
{
	delete[] m_arrfTimes;
	delete[] m_arrbIsWater;
	//delete[] bathyDepth2d;
	delete[] m_arrfUValues;
	delete[] m_arrfVValues;
	delete[] m_arrfWValues;
	delete[] m_arrfVelocityValues;
	//delete[] tValues;
	//delete[] sValues;

}

void FlowGrid::setDepthValue(int depthIndex, float depth)
{
	m_vDepthValues[depthIndex] = depth;
}

float FlowGrid::getMinDepth()
{
	return m_vDepthValues.front();
}

float FlowGrid::getMaxDepth()
{
	return m_vDepthValues.back();
}


void FlowGrid::setTimeValue(int timeIndex, float timeValue)
{
	m_arrfTimes[timeIndex] = timeValue;
	if (timeValue < m_fMinTime || m_fMinTime == -1.f)
		m_fMinTime = timeValue;
	if (timeValue > m_fMaxTime || m_fMaxTime == -1.f)
		m_fMaxTime = timeValue;
}

void FlowGrid::setCellValue(int x, int y, int z, int timestep, float u, float v)
{
	int index4d = (timestep*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
	m_arrfUValues[index4d] = u;
	m_arrfVValues[index4d] = v;
	m_arrfVelocityValues[index4d] = sqrt(u*u + v*v);
	
	if (m_arrfVelocityValues[index4d] > m_fMaxVelocity)
		m_fMaxVelocity = m_arrfVelocityValues[index4d];
}

void FlowGrid::setCellValue(int x, int y, int z, int timestep, float u, float v, float w)
{
	int index4d = (timestep*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
	m_arrfUValues[index4d] = u;
	m_arrfVValues[index4d] = v;
	m_arrfWValues[index4d] = w;
	m_arrfVelocityValues[index4d] = sqrt(u*u + v*v + w*w);
	
	if (m_arrfVelocityValues[index4d] > m_fMaxVelocity)
		m_fMaxVelocity = m_arrfVelocityValues[index4d];
}

void FlowGrid::setIsWaterValue(int x, int y, int z, int t, bool isCellWater)
{
	m_arrbIsWater[(t*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x] = isCellWater;
}

bool FlowGrid::getIsWaterAt(float lonX, float latY, float depth, float time)
{
	int x = (int)floor(((lonX-m_fXMin)/m_fXCellSize)+0.5);
	int y = (int)floor(((latY- m_fYMin)/ m_fYCellSize)+0.5);
	int z = 0;
	for (int i=0;i<m_nZCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			z = i;
	}

	int t = 0;
	for (int i=0;i<m_nTimesteps;i++)
	{
		if (time >= m_arrfTimes[i])
		{
			t = i;
		}
	}
	
	return m_arrbIsWater[(t*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x];
}

bool FlowGrid::getUVat(float lonX, float latY, float depth, float time, float *u, float *v)
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

	//find closest depth level? HACK: just use deepest one not below requested depth for now, maybe fix later if more accuracy needed
	int deepestLevelAbove = 0;
	if (depth < 0 || depth > m_vDepthValues.back()+500)  //HACK: MAGIC NUMBER (500) FIX LATER?
	{
		return false;
	}
	for (int i=0;i<m_nZCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-m_fXMin)/m_fXCellSize)+0.5);
	int y = (int)floor(((latY- m_fYMin)/ m_fYCellSize)+0.5);
	//int x = (int)floor( (((lonX - m_fXMin)/m_fXRange)*m_fXCells) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > m_nXCells-1 || y > m_nYCells-1 || z > m_nZCells-1)
		return false;
	//xyz index
	int index3d = ((z*m_nXYCells)+(y*m_nXCells)+(x));
			
	//check if in water
	if (!m_arrbIsWater[(m_iLastTime1*m_nXYZCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
		//if single timestep
		if (m_bLastTimeOnTimestep)
		{
			*u = m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d];
			*v = m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d];
			//printf("on timestep\n");
			//printf ("U: %f, V: %f\n", uValues[(lastTime1*size3d) + index3d], vValues[(lastTime1*size3d) + index3d]);
		}
		else //between timesteps
		{ 
			//printf("betwen timesteps\n");
			*u = (m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfUValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
			*v = (m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfVValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
			//printf ("U: %f, V: %f\n", (uValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*size3d) + index3d]*lastTimeFactor2), (vValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*size3d) + index3d]*lastTimeFactor2));
		}
		return true;
	}

}//end getUVat()


bool FlowGrid::getUVWat(float lonX, float latY, float depth, float time, float *u, float *v, float *w)
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

	//find closest depth level? HACK: just use deepest one not below requested depth for now, maybe fix later if more accuracy needed
	int deepestLevelAbove = 0;
	if (depth < 0 || depth > m_vDepthValues.back()+500)  //HACK: MAGIC NUMBER (500) FIX LATER?
	{
		return false;
	}
	for (int i=0;i<m_nZCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-m_fXMin)/m_fXCellSize)+0.5);
	int y = (int)floor(((latY- m_fYMin)/ m_fYCellSize)+0.5);
	//int x = (int)floor( (((lonX - m_fXMin)/m_fXRange)*m_fXCells) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > m_nXCells-1 || y > m_nYCells-1 || z > m_nZCells-1)
		return false;
	//xyz index
	int index3d = ((z*m_nXYCells)+(y*m_nXCells)+(x));
			
	//check if in water
	if (!m_arrbIsWater[(m_iLastTime1*m_nXYZCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
		//if single timestep
		if (m_bLastTimeOnTimestep)
		{
			*u = m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d];
			*v = m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d];
			*w = m_arrfWValues[(m_iLastTime1*m_nXYZCells) + index3d];
			//printf("on timestep\n");
			//printf ("U: %f, V: %f\n", uValues[(lastTime1*size3d) + index3d], vValues[(lastTime1*size3d) + index3d]);
		}
		else //between timesteps
		{ 
			//printf("betwen timesteps\n");
			*u = (m_arrfUValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfUValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
			*v = (m_arrfVValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfVValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
			*w = (m_arrfWValues[(m_iLastTime1*m_nXYZCells) + index3d]*m_fLastTimeFactor1) + (m_arrfWValues[(m_iLastTime2*m_nXYZCells) + index3d]*m_fLastTimeFactor2);
			//printf ("U: %f, V: %f\n", (uValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*size3d) + index3d]*lastTimeFactor2), (vValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*size3d) + index3d]*lastTimeFactor2));
		}
		return true;
	}

}//end getUVWat()



bool FlowGrid::getVelocityAt(float lonX, float latY, float depth, float time, float *velocity)
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

	//find closest depth level? HACK: just use deepest one not below requested depth for now, maybe fix later if more accuracy needed
	int deepestLevelAbove = 0;
	if (depth < 0 || depth > m_vDepthValues.back()+500)  //HACK: MAGIC NUMBER (500) FIX LATER?
	{
		return false;
	}
	for (int i=0;i<m_nZCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-m_fXMin)/m_fXCellSize)+0.5);
	int y = (int)floor(((latY- m_fYMin)/ m_fYCellSize)+0.5);
	//int x = (int)floor( (((lonX - m_fXMin)/m_fXRange)*m_fXCells) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > m_nXCells -1 || y > m_nYCells-1 || z > m_nZCells-1)
		return false;
	//xyz index
	int index3d = ((z*m_nXYCells)+(y*m_nXCells)+(x));
			
	//check if in water
	if (!m_arrbIsWater[(m_iLastTime1*m_nXYZCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
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
	}

}//end getVelocityAt()


void FlowGrid::drawBBox()
{
	float BBox[6];
	float visualOffset = 0.f;

	//HAD TO SWAP ZY again for VR coord system
	glm::vec3 bbMin(scaler->getScaledLonX(m_fXMin) - visualOffset, scaler->getScaledDepth(m_vDepthValues.front()) + visualOffset, scaler->getScaledLatY(m_fYMin) - visualOffset);
	glm::vec3 bbMax(scaler->getScaledLonX(m_fXMax) + visualOffset, scaler->getScaledDepth(m_vDepthValues.back()) - visualOffset, scaler->getScaledLatY(m_fYMax) + visualOffset);

	//DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawBox(bbMin, bbMax, glm::vec4(1.f, 0.f, 0.f, 1.f));
}//end drawBBox()

void FlowGrid::setCoordinateScaler(CoordinateScaler *Scaler)
{
	scaler = Scaler;
	scaler->submitOriginCandidate(m_fXMin, m_fYMin);
}
float FlowGrid::getScaledXMin()
{
	return scaler->getScaledLonX(m_fXMin);
}

float FlowGrid::getScaledXMax()
{
	return scaler->getScaledLonX(m_fXMax);
}

float FlowGrid::getScaledYMin()
{
	return scaler->getScaledLatY(m_fYMin);
}

float FlowGrid::getScaledYMax()
{
	return scaler->getScaledLatY(m_fYMax);
}

float FlowGrid::getScaledMinDepth()
{
	return scaler->getScaledDepth(getMinDepth());
}

float FlowGrid::getScaledMaxDepth()
{
	return scaler->getScaledDepth(getMaxDepth());
}



bool FlowGrid::contains(float x, float y)
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

bool FlowGrid::contains(float x, float y, float z)
{
	if (x < m_fXMin)
		return false;
	else if (x > m_fXMax)
		return false;
	else if (y < m_fYMin)
		return false;
	else if (y > m_fYMax)
		return false;
	else if (z < m_vDepthValues.front())
		return false;
	else if (z > m_vDepthValues.back())
		return false;
	else
		return true;
}


int FlowGrid::getNumXYCells()
{
	return m_nXYCells;
}

void FlowGrid::getXYZofCell(int cellIndex, float *lonX, float *latY, float *depth)
{
	int x = cellIndex%m_nXCells;
	int y = floor((float)cellIndex/(float)m_nXCells);
	
	*lonX = m_fXMin + (x * m_fXCellSize);
	*latY = m_fYMin + (y * m_fYCellSize);

	//*depth = bathyDepth2d[(y*xCells) + x];

	int z = 0;
	for (int i=0;i<m_nZCells;i++)
	{
		if (*depth >= m_vDepthValues[i])
			z = i;
	}
	*depth = z;

	//printf("Cell %d of %d is %f, %f, %f\n", cellIndex, gridSize2d, *lonX, *latY, *depth);
}


float FlowGrid::getXMin()
{
	return m_fXMin;
}

float FlowGrid::getXMax()
{
	return m_fXMax;
}

float FlowGrid::getYMin()
{
	return m_fYMin;
}

float FlowGrid::getYMax()
{
	return m_fYMax;
}

float FlowGrid::getXCellSize()
{
	return m_fXCellSize;
}

float FlowGrid::getYCellSize()
{
	return m_fYCellSize;
}

bool FlowGrid::getCellBounds(float m_fXMin, float m_fXMax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax)
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

float FlowGrid::getTimeAtTimestep(int timestep)
{
	return m_arrfTimes[timestep];
}

int FlowGrid::getNumTimeCells()
{
	return m_nTimesteps;
}

char* FlowGrid::getName()
{
	return m_strName;
}

void FlowGrid::setName(char* name)
{
	strcpy(m_strName, name);
}

