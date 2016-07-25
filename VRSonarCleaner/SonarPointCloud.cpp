#include "SonarPointCloud.h"

SonarPointCloud::SonarPointCloud()
{
	markedForDeletion = false;
	buffersGenerated = false;

	firstMinMaxSet = false;

	xMin = 0;
	xMax = 0;
	xRange = xMax - xMin;
	yMin = 0;
	yMax = 0;
	yRange = yMax - yMin;
	
	minDepth = 123456789;
	maxDepth = -123456789;

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
		pointsAllocated = false;
	}

	if (buffersGenerated)
	{
		glDeleteBuffers(1, &pointsPositionsVBO);
		glDeleteBuffers(1, &pointsColorsVBO);
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
		delete[] pointsDepthTPU;
		delete[] pointsPositionTPU;
	}
	pointsPositions = new double[numPoints*3];
	pointsColors = new float[numPoints*3];
	pointsDepthTPU = new float[numPoints];
	pointsPositionTPU = new float[numPoints];;
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

		colorScalerTPU->submitBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU);

		setRefreshNeeded();

		printf("Trimmed Min/Maxes:\n");
		printf("TrimXMin: %f TrimYMin: %f\n", actualRemovedXmin, actualRemovedYmin);
		printf("X Min: %f Max: %f\n", xMin, xMax);
		printf("Y Min: %f Max: %f\n", yMin, yMax);
		printf("ZDepth Min: %f Max: %f\n", minDepth, maxDepth);

	}

	return true;
}

void SonarPointCloud::buildPointsVBO()
{
	
	printf("Rebuilding PointCloud VBOs\n");

	//printGlErrorCode();

	//printf("Time: %f, TMin: %d, TMax: %d, TFact: %0.2f\n", time, tmin, tmax, timeFactor);
	
	if (!glewInited)
	{
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			printf("Glew already initialized?!\n");
			printf("GlewInit error: %s\n", glewGetErrorString(err));
			//return;
		}
		glewInited = true;
	}


	
	//build VBO out of mesh
	//printf("checking before gen buffers\n");
	if (!buffersGenerated)
	{
		//printf("PID of dynamicbathy process: %d\n", GetCurrentProcessId());
		glGenBuffers(1, &pointsPositionsVBO);
		glGenBuffers(1, &pointsColorsVBO);
		
		buffersGenerated = true;
	}
	else
	{
		//printf("deleting buffers\n");
		glDeleteBuffers(1, &pointsPositionsVBO);
		glDeleteBuffers(1, &pointsColorsVBO);
		
		glGenBuffers(1, &pointsPositionsVBO);
		glGenBuffers(1, &pointsColorsVBO);

		buffersGenerated = true;
	}

	numPointsInVBO = numPoints;

	printf("building vbo with %d points\n", numPointsInVBO);

	GLsizeiptr positionsSize = numPointsInVBO * 3  * sizeof(GLfloat);
	GLsizeiptr colorsSize    = numPointsInVBO * 3 * sizeof(GLfloat);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, pointsPositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, pointsColorsVBO);
	glBufferData(GL_ARRAY_BUFFER, colorsSize, NULL, GL_STATIC_DRAW);
	GLfloat* colors = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	double x,y,z;
	float r,g,b;
	
	int index = 0;
	
	//if (colorScope == 1)
		//projSettings->resetMinMaxForColorScale(minDepth, maxDepth);

	//projSettings->setColorScale(colorScale);

	for (int i=0;i<numPoints;i++)
	{
		x = pointsPositions[i * 3];//projSettings->getScaledLonX(pointsPositions[i*3]);
		y = pointsPositions[(i * 3) + 1];//projSettings->getScaledLatY(pointsPositions[(i*3)+1]);
		z = pointsPositions[(i * 3) + 2];//projSettings->getScaledDepth(pointsPositions[(i*3)+2]);

		colorScalerTPU->getBiValueScaledColor(pointsDepthTPU[i], pointsPositionTPU[i], &r, &g, &b);
		//r = 1;// colorScalerTPU->getScaledColor(//1;// pointsColors[i * 3];
		//g = 1;// pointsColors[(i * 3) + 1];
		//b = 1;// pointsColors[(i * 3) + 2];
		
		positions[(index*3)]   = (float)x;
		positions[(index*3)+1] = (float)z;  ///SWAP Y and Z for OpenGL coordinate system (y-up)
		positions[(index*3)+2] = (float)y;

		colors[(index*3)]   = r;
		colors[(index*3)+1] = g;
		colors[(index*3)+2] = b;

		index++;

	}//end for each pt

	glBindBuffer(GL_ARRAY_BUFFER, pointsPositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, pointsColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(3, GL_FLOAT, 0, NULL);
//printf("at END: ");
	//printGlErrorCode();

	printf("Build VBO with %d points\n", numPointsInVBO);

	refreshNeeded = false;

}

void SonarPointCloud::buildPreviewVBO()
{

	printf("Rebuilding PointCloud VBOs\n");

	//printGlErrorCode();

	//printf("Time: %f, TMin: %d, TMax: %d, TFact: %0.2f\n", time, tmin, tmax, timeFactor);

	if (!glewInited)
	{
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			printf("Glew already initialized?!\n");
			printf("GlewInit error: %s\n", glewGetErrorString(err));
			//return;
		}
		glewInited = true;
	}



	//build VBO out of mesh
	//printf("checking before gen buffers\n");
	if (!buffersGenerated)
	{
		//printf("PID of dynamicbathy process: %d\n", GetCurrentProcessId());
		glGenBuffers(1, &previewPointsPositionsVBO);
		glGenBuffers(1, &previewPointsColorsVBO);

		buffersGenerated = true;
	}
	else
	{
		//printf("deleting buffers\n");
		glDeleteBuffers(1, &previewPointsPositionsVBO);
		glDeleteBuffers(1, &previewPointsColorsVBO);

		glGenBuffers(1, &previewPointsPositionsVBO);
		glGenBuffers(1, &previewPointsColorsVBO);

		buffersGenerated = true;
	}

	int reductionFactor = 20;

	previewNumPointsInVBO = (int)floor((double)numPoints/ (double)reductionFactor);

	printf("building preview vbo with %d points\n", previewNumPointsInVBO);

	GLsizeiptr positionsSize = previewNumPointsInVBO * 3 * sizeof(GLfloat);
	GLsizeiptr colorsSize = previewNumPointsInVBO * 3 * sizeof(GLfloat);


	glBindBuffer(GL_ARRAY_BUFFER, previewPointsPositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, previewPointsColorsVBO);
	glBufferData(GL_ARRAY_BUFFER, colorsSize, NULL, GL_STATIC_DRAW);
	GLfloat* colors = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	double x, y, z;
	float r, g, b;

	int index = 0;

	//if (colorScope == 1)
	//projSettings->resetMinMaxForColorScale(minDepth, maxDepth);

	//projSettings->setColorScale(colorScale);

	for (int i = 0; i<numPoints; i+= reductionFactor)
	{
		x = pointsPositions[i * 3];//projSettings->getScaledLonX(pointsPositions[i*3]);
		y = pointsPositions[(i * 3) + 1];//projSettings->getScaledLatY(pointsPositions[(i*3)+1]);
		z = pointsPositions[(i * 3) + 2];//projSettings->getScaledDepth(pointsPositions[(i*3)+2]);

		colorScalerTPU->getBiValueScaledColor(pointsDepthTPU[i], pointsPositionTPU[i], &r, &g, &b);
		//r = 1;// colorScalerTPU->getScaledColor(//1;// pointsColors[i * 3];
		//g = 1;// pointsColors[(i * 3) + 1];
		//b = 1;// pointsColors[(i * 3) + 2];

		positions[(index * 3)] = (float)x;
		positions[(index * 3) + 1] = (float)z;  ///SWAP Y and Z for OpenGL coordinate system (y-up)
		positions[(index * 3) + 2] = (float)y;

		colors[(index * 3)] = r;
		colors[(index * 3) + 1] = g;
		colors[(index * 3) + 2] = b;

		index++;

	}//end for each pt

	glBindBuffer(GL_ARRAY_BUFFER, previewPointsPositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, previewPointsColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(3, GL_FLOAT, 0, NULL);
	//printf("at END: ");
	//printGlErrorCode();

	printf("Build Preview VBO with %d points\n", previewNumPointsInVBO);

	previewRefreshNeeded = false;

}

void SonarPointCloud::draw()
{
	if (numPoints > 0) //if we even have points
	{
		if (!buffersGenerated || numPointsInVBO < 1 || refreshNeeded)
		{
			buildPointsVBO();
		}
		drawPointsVBO();
	}
}

void SonarPointCloud::drawPreview()
{
	if (numPoints > 0) //if we even have points
	{
		if (!previewBuffersGenerated || previewNumPointsInVBO < 1 || previewRefreshNeeded)
		{
			buildPreviewVBO();
		}
		drawPreviewVBO();
	}
}

void SonarPointCloud::drawPointsVBO()
{
	//printf("PointCloud-drawPointsVBO()\n");

	//printf("MINMAX X %f, %f Y %f, %f Z %f %f\n", xMin, xMax, yMin, yMax, minDepth, maxDepth);

	if (!buffersGenerated)
		return;
	if (numPointsInVBO < 1)
		return;

	/*
	float ptSizes[2];
	float ptQuadratic[3];
	ptQuadratic[0] = 0.00000001;//0.0000001;//a = -0.01 this is the "falloff" speed that decreases the point size as distance grows
	ptQuadratic[1] = 0.0001;//0.001;//b = 0 center parabola on x (dist) = 0
	ptQuadratic[2] = 0.00001;//0.0001;//c = 4 (this is the minimum size, when dist = 0)
		
	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, ptSizes);
    //glEnable( GL_POINT_SPRITE_ARB );
    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, ptSizes[0]);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, ptSizes[1]);
    glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, ptQuadratic);
    //glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
	*/

	glPointSize(1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glColor4f(1,1,1,1);
	//glPointSize(3);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, pointsPositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, pointsColorsVBO);
	glColorPointer(3, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_POINTS, 0, numPointsInVBO);
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );	

	glDisable(GL_POINT_SPRITE_ARB);
	
	//printf("DrawDone\n");

}//end drawWaterMeshVBO()

void SonarPointCloud::drawPreviewVBO()
{
	if (!previewBuffersGenerated)
		return;
	if (previewNumPointsInVBO < 1)
		return;

	/*
	float ptSizes[2];
	float ptQuadratic[3];
	ptQuadratic[0] = 0.00000001;//0.0000001;//a = -0.01 this is the "falloff" speed that decreases the point size as distance grows
	ptQuadratic[1] = 0.0001;//0.001;//b = 0 center parabola on x (dist) = 0
	ptQuadratic[2] = 0.00001;//0.0001;//c = 4 (this is the minimum size, when dist = 0)

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, ptSizes);
	//glEnable( GL_POINT_SPRITE_ARB );
	glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, ptSizes[0]);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, ptSizes[1]);
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, ptQuadratic);
	//glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
	*/

	glPointSize(1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glColor4f(1, 1, 1, 1);
	//glPointSize(3);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, previewPointsPositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, previewPointsColorsVBO);
	glColorPointer(3, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_POINTS, 0, previewNumPointsInVBO);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_POINT_SPRITE_ARB);

	//printf("DrawDone\n");

}//end drawWaterMeshVBO()

void SonarPointCloud::refreshPointsVBO()
{
	//printf("Refreshing Dynamic Bathy VBOs\n");
	buildPointsVBO();
	refreshNeeded = false;
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

std::vector<Vector3> SonarPointCloud::getPointPositions()
{
	std::vector<Vector3> ret;
	for (int i = 0; i < numPoints * 3; i += 3)
	{
		Vector3 thisVert = { // SWAP Y AND Z
			  (float)pointsPositions[i + 0]
			, (float)pointsPositions[i + 2]
			, (float)pointsPositions[i + 1]
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
	//printf("In Holodeck Draw()\n");
	glColor3f(0.75, 0.0, 0.0);
	glLineWidth(1.0);

	float smallSize = xRange / 40;
	float bigSize = xRange / 20;
	float centerX = (xMin + xMax) / 2;
	float centerY = (minDepth + maxDepth) / 2;  //swap z & y
	float centerZ = (yMin + yMax) / 2;

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(centerX - smallSize, centerY, centerZ);
	glVertex3f(centerX + bigSize, centerY, centerZ);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(centerX, centerY - smallSize, centerZ);
	glVertex3f(centerX, centerY + bigSize, centerZ);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(centerX, centerY, centerZ - smallSize);
	glVertex3f(centerX, centerY, centerZ + bigSize);
	glEnd();
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