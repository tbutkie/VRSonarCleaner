#include "FlowGrid.h"

#include "DebugDrawer.h"

FlowGrid::FlowGrid(float minX, float maxX, int cellsX, float minY, float maxY, int cellsY, int cellsZ, int timesteps)
{
	printf("new grid: %f-%f,%f-%f (%d, %d)x%d\n", minX, maxX, minY, maxY, cellsX, cellsY, cellsZ);
	numTimesteps = timesteps;
	xCells = cellsX;
	yCells = cellsY;
	zCells = cellsZ;
	xMin = minX;
	xMax = maxX;
	yMin = minY;
	yMax = maxY;
	init();
}

FlowGrid::FlowGrid(char* filename)
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

	fread(&xMin, sizeof(float), 1, inputFile);
	fread(&xMax, sizeof(float), 1, inputFile);
	fread(&xCells, sizeof(int), 1, inputFile);
	fread(&yMin, sizeof(float), 1, inputFile);
	fread(&yMax, sizeof(float), 1, inputFile);
	fread(&yCells, sizeof(int), 1, inputFile);
	fread(&zMin, sizeof(float), 1, inputFile);
	fread(&zMax, sizeof(float), 1, inputFile);
	fread(&zCells, sizeof(int), 1, inputFile);
	fread(&numTimesteps, sizeof(int), 1, inputFile);

	init();

	float tempDepth;
	for (int i = 0; i < zCells; i++)
	{
		fread(&tempDepth, sizeof(float), 1, inputFile);
		setDepthValue(i, tempDepth);
	}

	float tempTime;
	for (int i = 0; i < numTimesteps; i++)
	{
		fread(&tempTime, sizeof(float), 1, inputFile);
		setTimeValue(i, tempTime);
	}

	int index4d;
	int tempIsWater;
	float tempU, tempV, tempW;
	for (int x = 0; x<xCells; x++)
	{
		for (int y = 0; y<yCells; y++)
		{
			for (int z = 0; z<zCells; z++)
			{
				for (int t = 0; t<numTimesteps; t++)
				{
					index4d = (t*xyzCells) + (z*xyCells) + (y*xCells) + x;
					fread(&tempIsWater, sizeof(int), 1, inputFile);
					fread(&tempU, sizeof(float), 1, inputFile);
					fread(&tempV, sizeof(float), 1, inputFile);
					fread(&tempW, sizeof(float), 1, inputFile);
					setIsWaterValue(x, y, z, t, tempIsWater);
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
	activeTimestep = -1;
	times = new float[numTimesteps];
	gridSize2d = xCells * yCells;
	gridSize3d = xCells * yCells * zCells;
	gridSize4d = xCells * yCells * zCells * numTimesteps;
	xyCells = xCells*yCells;
	xyzCells = xCells*yCells*zCells;
	xCellsFloat = (float)xCells;
	yCellsFloat = (float)yCells;
	zCellsFloat = (float)zCells;
	xRange = xMax - xMin;
	yRange = yMax - yMin;
	m_vDepthValues.resize(zCells);
	depthsSet = false;

	xCellSize = xRange / xCellsFloat;
	yCellSize = yRange / yCellsFloat;

	maxVelocity = 0;



	//allocate storage arrays
	isWater = new bool[gridSize4d];
	//bathyDepth2d = new float[gridSize2d];

	uValues = new float[gridSize4d];
	vValues = new float[gridSize4d];
	wValues = new float[gridSize4d];
	velocityValues = new float[gridSize4d];
	//tValues = new float[gridSize4d];
	//sValues = new float[gridSize4d];

	lastTimeRequested = -1;
	lastTimeRequested = true;

	enableIllustrativeParticles = true;
	numIllustrativeParticles = 1000;
	illustrativeParticleTrailTime = 500;
	illustrativeParticleLifetime = 2500;
	illustrativeParticleSize = 1;

	colorIllustrativeParticles[0] = 0.25;
	colorIllustrativeParticles[1] = 0.95;
	colorIllustrativeParticles[2] = 1.0;
	illustrativeParticleVelocityScale = .01;//0.000001;
}

void FlowGrid::deleteSelf()
{
	delete[] times;
	delete[] isWater;
	//delete[] bathyDepth2d;
	delete[] uValues;
	delete[] vValues;
	delete[] wValues;
	delete[] velocityValues;
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
	times[timeIndex] = timeValue;
	if (timeValue < minTime)
		minTime = timeValue;
	if (timeValue > maxTime)
		maxTime = timeValue;
}

void FlowGrid::setCellValue(int x, int y, int z, int timestep, float u, float v)
{
	int index4d = (timestep*xyzCells) + (z*xyCells) + (y*xCells) + x;
	uValues[index4d] = u;
	vValues[index4d] = v;
	velocityValues[index4d] = sqrt(u*u + v*v);
	
	if (velocityValues[index4d] > maxVelocity)
		maxVelocity = velocityValues[index4d];
}

void FlowGrid::setCellValue(int x, int y, int z, int timestep, float u, float v, float w)
{
	int index4d = (timestep*xyzCells) + (z*xyCells) + (y*xCells) + x;
	uValues[index4d] = u;
	vValues[index4d] = v;
	wValues[index4d] = w;
	velocityValues[index4d] = sqrt(u*u + v*v + w*w);
	
	if (velocityValues[index4d] > maxVelocity)
		maxVelocity = velocityValues[index4d];
}

void FlowGrid::setIsWaterValue(int x, int y, int z, int t, bool isCellWater)
{
	isWater[(t*xyzCells) + (z*xyCells) + (y*xCells) + x] = isCellWater;
}

bool FlowGrid::getIsWaterAt(float lonX, float latY, float depth, float time)
{
	int x = (int)floor(((lonX-xMin)/xCellSize)+0.5);
	int y = (int)floor(((latY-yMin)/yCellSize)+0.5);
	int z = 0;
	for (int i=0;i<zCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			z = i;
	}

	int t = 0;
	for (int i=0;i<numTimesteps;i++)
	{
		if (time >= times[i])
		{
			t = i;
		}
	}
	
	return isWater[(t*xyzCells) + (z*xyCells) + (y*xCells) + x];
}

bool FlowGrid::getUVat(float lonX, float latY, float depth, float time, float *u, float *v)
{
	//first we check time requested to see if its the same as last time
	if (time != lastTimeRequested)
	{
		//new time, have to find time values anew
		lastTimeRequested = time;
		
		//find if on timestep exactly, or is between what times
		int above = -1;
		int below = -1;
		lastTimeOnTimestep = false;
		for (int i=0;i<numTimesteps;i++)
		{
			if (time > times[i])
			{
				below = i;
			}
			else if (time == times[i])
			{
				lastTimeOnTimestep = true;
				lastTime1 = i;
				lastTimeFactor1 = 1;
				lastTime2 = i;
				lastTimeFactor2 = 0;
				above = i;
				below = i;
				break;
			}
			else if (time < times[i])
			{
				above = i;
				break;
			}
		}
		if (!lastTimeOnTimestep) //if didn't match a timestep exactly
		{
			//if was below min time
			if (below == -1)
			{
				//set to min time
				lastTimeOnTimestep = true;
				lastTime1 = 0;
				lastTimeFactor1 = 1;
				lastTime2 = 0;
				lastTimeFactor2 = 0;
			}
			//if was above max time
			else if (above == -1)
			{
				//set to max time
				lastTimeOnTimestep = true;
				lastTime1 = numTimesteps-1;
				lastTimeFactor1 = 1;
				lastTime2 = numTimesteps-1;
				lastTimeFactor2 = 0;
			}
			else //is in between two timesteps
			{
				//set both times and factor for each
				float range = times[above] - times[below];
				float fromBelow = time - times[below];
				float factor = fromBelow / range;
				lastTime1 = below;
				lastTimeFactor1 = 1-factor;
				lastTime2 = above;
				lastTimeFactor2 = factor;
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
	for (int i=0;i<zCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-xMin)/xCellSize)+0.5);
	int y = (int)floor(((latY-yMin)/yCellSize)+0.5);
	//int x = (int)floor( (((lonX - xMin)/xRange)*xCellsFloat) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > xCells-1 || y > yCells-1 || z > zCells-1)
		return false;
	//xyz index
	int index3d = ((z*xyCells)+(y*xCells)+(x));
			
	//check if in water
	if (!isWater[(lastTime1*xyzCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
		//if single timestep
		if (lastTimeOnTimestep)
		{
			*u = uValues[(lastTime1*xyzCells) + index3d];
			*v = vValues[(lastTime1*xyzCells) + index3d];
			//printf("on timestep\n");
			//printf ("U: %f, V: %f\n", uValues[(lastTime1*size3d) + index3d], vValues[(lastTime1*size3d) + index3d]);
		}
		else //between timesteps
		{ 
			//printf("betwen timesteps\n");
			*u = (uValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);
			*v = (vValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);
			//printf ("U: %f, V: %f\n", (uValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*size3d) + index3d]*lastTimeFactor2), (vValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*size3d) + index3d]*lastTimeFactor2));
		}
		return true;
	}

}//end getUVat()


bool FlowGrid::getUVWat(float lonX, float latY, float depth, float time, float *u, float *v, float *w)
{
	//first we check time requested to see if its the same as last time
	if (time != lastTimeRequested)
	{
		//new time, have to find time values anew
		lastTimeRequested = time;
		
		//find if on timestep exactly, or is between what times
		int above = -1;
		int below = -1;
		lastTimeOnTimestep = false;
		for (int i=0;i<numTimesteps;i++)
		{
			if (time > times[i])
			{
				below = i;
			}
			else if (time == times[i])
			{
				lastTimeOnTimestep = true;
				lastTime1 = i;
				lastTimeFactor1 = 1;
				lastTime2 = i;
				lastTimeFactor2 = 0;
				above = i;
				below = i;
				break;
			}
			else if (time < times[i])
			{
				above = i;
				break;
			}
		}
		if (!lastTimeOnTimestep) //if didn't match a timestep exactly
		{
			//if was below min time
			if (below == -1)
			{
				//set to min time
				lastTimeOnTimestep = true;
				lastTime1 = 0;
				lastTimeFactor1 = 1;
				lastTime2 = 0;
				lastTimeFactor2 = 0;
			}
			//if was above max time
			else if (above == -1)
			{
				//set to max time
				lastTimeOnTimestep = true;
				lastTime1 = numTimesteps-1;
				lastTimeFactor1 = 1;
				lastTime2 = numTimesteps-1;
				lastTimeFactor2 = 0;
			}
			else //is in between two timesteps
			{
				//set both times and factor for each
				float range = times[above] - times[below];
				float fromBelow = time - times[below];
				float factor = fromBelow / range;
				lastTime1 = below;
				lastTimeFactor1 = 1-factor;
				lastTime2 = above;
				lastTimeFactor2 = factor;
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
	for (int i=0;i<zCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-xMin)/xCellSize)+0.5);
	int y = (int)floor(((latY-yMin)/yCellSize)+0.5);
	//int x = (int)floor( (((lonX - xMin)/xRange)*xCellsFloat) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > xCells-1 || y > yCells-1 || z > zCells-1)
		return false;
	//xyz index
	int index3d = ((z*xyCells)+(y*xCells)+(x));
			
	//check if in water
	if (!isWater[(lastTime1*xyzCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
		//if single timestep
		if (lastTimeOnTimestep)
		{
			*u = uValues[(lastTime1*xyzCells) + index3d];
			*v = vValues[(lastTime1*xyzCells) + index3d];
			*w = wValues[(lastTime1*xyzCells) + index3d];
			//printf("on timestep\n");
			//printf ("U: %f, V: %f\n", uValues[(lastTime1*size3d) + index3d], vValues[(lastTime1*size3d) + index3d]);
		}
		else //between timesteps
		{ 
			//printf("betwen timesteps\n");
			*u = (uValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);
			*v = (vValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);
			*w = (wValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (wValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);
			//printf ("U: %f, V: %f\n", (uValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (uValues[(lastTime2*size3d) + index3d]*lastTimeFactor2), (vValues[(lastTime1*size3d) + index3d]*lastTimeFactor1) + (vValues[(lastTime2*size3d) + index3d]*lastTimeFactor2));
		}
		return true;
	}

}//end getUVWat()



bool FlowGrid::getVelocityAt(float lonX, float latY, float depth, float time, float *velocity)
{
	//first we check time requested to see if its the same as last time
	if (time != lastTimeRequested)
	{
		//new time, have to find time values anew
		lastTimeRequested = time;
		
		//find if on timestep exactly, or is between what times
		int above = -1;
		int below = -1;
		lastTimeOnTimestep = false;
		for (int i=0;i<numTimesteps;i++)
		{
			if (time > times[i])
			{
				below = i;
			}
			else if (time == times[i])
			{
				lastTimeOnTimestep = true;
				lastTime1 = i;
				lastTimeFactor1 = 1;
				lastTime2 = i;
				lastTimeFactor2 = 0;
				above = i;
				below = i;
				break;
			}
			else if (time < times[i])
			{
				above = i;
				break;
			}
		}
		if (!lastTimeOnTimestep) //if didn't match a timestep exactly
		{
			//if was below min time
			if (below == -1)
			{
				//set to min time
				lastTimeOnTimestep = true;
				lastTime1 = 0;
				lastTimeFactor1 = 1;
				lastTime2 = 0;
				lastTimeFactor2 = 0;
			}
			//if was above max time
			else if (above == -1)
			{
				//set to max time
				lastTimeOnTimestep = true;
				lastTime1 = numTimesteps-1;
				lastTimeFactor1 = 1;
				lastTime2 = numTimesteps-1;
				lastTimeFactor2 = 0;
			}
			else //is in between two timesteps
			{
				//set both times and factor for each
				float range = times[above] - times[below];
				float fromBelow = time - times[below];
				float factor = fromBelow / range;
				lastTime1 = below;
				lastTimeFactor1 = 1-factor;
				lastTime2 = above;
				lastTimeFactor2 = factor;
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
	for (int i=0;i<zCells;i++)
	{
		if (depth >= m_vDepthValues[i])
			deepestLevelAbove = i;
	}
	//get index of requested location
	int x = (int)floor(((lonX-xMin)/xCellSize)+0.5);
	int y = (int)floor(((latY-yMin)/yCellSize)+0.5);
	//int x = (int)floor( (((lonX - xMin)/xRange)*xCellsFloat) + 0.5);
	//int y = (int)floor( (((latY - yMin)/yRange)*yCellsFloat) + 0.5);
	int z = deepestLevelAbove;
		
	//printf("UVAT: x: %d of %d  y: %d of %d  z: %d of %d\n", x, xCells, y, yCells, z, zCells);

	//check if in bounds of the dataset
	if (x < 0 || y < 0 || z < 0 || x > xCells-1 || y > yCells-1 || z > zCells-1)
		return false;
	//xyz index
	int index3d = ((z*xyCells)+(y*xCells)+(x));
			
	//check if in water
	if (!isWater[(lastTime1*xyzCells) + index3d])
	{
		//printf("not in water\n");
		return false; //if not in water, return false
	}
	else
	{
		//if single timestep
		if (lastTimeOnTimestep)
		{
			*velocity = velocityValues[(lastTime1*xyzCells) + index3d];
		}
		else //between timesteps
		{ 
			*velocity = (velocityValues[(lastTime1*xyzCells) + index3d]*lastTimeFactor1) + (velocityValues[(lastTime2*xyzCells) + index3d]*lastTimeFactor2);	
		}
		return true;
	}

}//end getVelocityAt()


void FlowGrid::drawBBox()
{
	float BBox[6];
	float visualOffset = 0.1;

	//HAD TO SWAP ZY again for VR coord system
	glm::vec3 bbMin(scaler->getScaledLonX(xMin) - visualOffset, scaler->getScaledDepth(m_vDepthValues.front()) + visualOffset, scaler->getScaledLatY(yMin) - visualOffset);
	glm::vec3 bbMax(scaler->getScaledLonX(xMax) + visualOffset, scaler->getScaledDepth(m_vDepthValues.back()) - visualOffset, scaler->getScaledLatY(yMax) + visualOffset);

	//DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawBox(bbMin, bbMax, glm::vec4(1.f, 0.f, 0.f, 1.f));
}//end drawBBox()

void FlowGrid::setCoordinateScaler(CoordinateScaler *Scaler)
{
	scaler = Scaler;
	scaler->submitOriginCandidate(xMin, yMin);
}
float FlowGrid::getScaledXMin()
{
	return scaler->getScaledLonX(xMin);
}

float FlowGrid::getScaledXMax()
{
	return scaler->getScaledLonX(xMax);
}

float FlowGrid::getScaledYMin()
{
	return scaler->getScaledLatY(yMin);
}

float FlowGrid::getScaledYMax()
{
	return scaler->getScaledLatY(yMax);
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
	if (x < xMin)
		return false;
	else if (x > xMax)
		return false;
	else if (y < yMin)
		return false;
	else if (y > yMax)
		return false;
	else
		return true;
}

bool FlowGrid::contains(float x, float y, float z)
{
	if (x < xMin)
		return false;
	else if (x > xMax)
		return false;
	else if (y < yMin)
		return false;
	else if (y > yMax)
		return false;
	else if (z < 0)
		return false;
	else if (z > m_vDepthValues.back())
		return false;
	else
		return true;
}


int FlowGrid::getNumXYCells()
{
	return xyCells;
}

void FlowGrid::getXYZofCell(int cellIndex, float *lonX, float *latY, float *depth)
{
	int x = cellIndex%xCells;
	int y = floor((float)cellIndex/(float)xCells);
	
	*lonX = xMin + (x * xCellSize);
	*latY = yMin + (y * yCellSize);

	//*depth = bathyDepth2d[(y*xCells) + x];

	int z = 0;
	for (int i=0;i<zCells;i++)
	{
		if (*depth >= m_vDepthValues[i])
			z = i;
	}
	*depth = z;

	//printf("Cell %d of %d is %f, %f, %f\n", cellIndex, gridSize2d, *lonX, *latY, *depth);
}


float FlowGrid::getXMin()
{
	return xMin;
}

float FlowGrid::getXMax()
{
	return xMax;
}

float FlowGrid::getYMin()
{
	return yMin;
}

float FlowGrid::getYMax()
{
	return yMax;
}

float FlowGrid::getXCellSize()
{
	return xCellSize;
}

float FlowGrid::getYCellSize()
{
	return yCellSize;
}

bool FlowGrid::getCellBounds(float xmin, float xmax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax)
{
	//find min x cell
	int minXCell;
	if (xmin < xMin) //beyond and contains edge
	{
		minXCell = 0;
	}
	else if (xmin > xMax) //not in bounds, should not happen
	{
		printf("ERROR 15151515 in getCellBounds\n");
		return false;
	}
	else //within
	{
		minXCell = 0;
		float xHere;
		for (int i=0;i<xCells-1;i++)
		{
			xHere = xMin + (xCellSize*i);
			if (xHere < xmin)
				minXCell = i;
			else
				break;
		}
	}


	//find max x cell
	int maxXCell;
	if (xmax > xMax) //beyond and contains edge
	{
		maxXCell = xCells-1;
	}
	else if (xmax < xMin) //not in bounds, should not happen
	{
		printf("ERROR 377257 in getCellBounds\n");
		return false;
	}
	else //within
	{
		maxXCell = xCells-1;
		float xHere;
		for (int i=xCells-1;i>-1;i--)
		{
			xHere = xMin + xCellSize + (xCellSize*i); //extra xcellsize because want far edge
			if (xHere > xmax)
				maxXCell = i;
			else
				break;
		}
	}

	//find min y cell
	int minYCell;
	if (ymin < yMin) //beyond and contains edge
	{
		minYCell = 0;
	}
	else if (ymin > yMax) //not in bounds, should not happen
	{
		printf("ERROR 254572 in getCellBounds\n");
		return false;
	}
	else //within
	{
		minYCell = 0;
		float yHere;
		for (int i=0;i<yCells-1;i++)
		{
			yHere = yMin + (yCellSize*i);
			if (yHere < xmin)
				minYCell = i;
			else
				break;
		}
	}


	//find max y cell
	int maxYCell;
	if (ymax > yMax) //beyond and contains edge
	{
		maxYCell = yCells-1;
	}
	else if (ymax < yMin) //not in bounds, should not happen
	{
		printf("ERROR 546854683 in getCellBounds\n");
		return false;
	}
	else //within
	{
		maxYCell = yCells-1;
		float yHere;
		for (int i=yCells-1;i>-1;i--)
		{
			yHere = yMin + yCellSize + (yCellSize*i); //extra xcellsize because want far edge
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

	*xcellmin = min(minXCell, maxXCell);
	*xcellmax = max(minXCell, maxXCell);

	//*xcells = max(minXCell, maxXCell) - min(minXCell, maxXCell) + 1;

	*ycellmin = min(minYCell, maxYCell);
	*ycellmax = max(minYCell, maxYCell);
	//*ycells = max(minYCell, maxYCell) - min(minYCell, maxYCell) + 1;

	return true;
}

float FlowGrid::getTimeAtTimestep(int timestep)
{
	return times[timestep];
}

int FlowGrid::getNumTimeCells()
{
	return numTimesteps;
}

char* FlowGrid::getName()
{
	return name;
}

void FlowGrid::setName(char* Name)
{
	strcpy(name, Name);
}

