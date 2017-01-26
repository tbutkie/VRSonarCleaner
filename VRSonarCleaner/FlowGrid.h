#ifndef __FlowGrid_h__
#define __FlowGrid_h__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm> 
#include <GL/glew.h>
#include "CoordinateScaler.h"

using namespace std;

extern unsigned int textureIDs[100];

class FlowGrid
{
	public:

		FlowGrid(float minX, float maxX, int cellsX, float minY, float maxY, int cellsY, int cellsZ, int timesteps);
		FlowGrid(char* filename);
		virtual ~FlowGrid();

		void init();

		void deleteSelf();

		void setDepthValue(int depthIndex, float depth);

		void setBathyDepthValue(int x, int y, float depth);

		void setTimeValue(int timeIndex, float timeValue);		
		
		void setCellValue(int x, int y, int z, int timestep, float u, float v);
		//void setCellValue(int x, int y, int z, int timestep, float u, float v, float t, float s);

		void setIsWaterValue(int x, int y, int z, int t, bool isCellWater);
		
		bool getUVat(float lonX, float latY, float depth, float time, float *u, float *v);
		bool getVelocityAt(float lonX, float latY, float depth, float time, float *velocity);
		//bool getUVTSat(float lonX, float latY, float depth, float time, float *u, float *v, float *t, float *s);
		bool getIsWaterAt(float lonX, float latY, float depth, float time);
		//float getBathyDepthAt(float lonX, float latY);
		//void getInfoOnWaterCellsAt(float lonX, float latY, int *numCells, );


		void drawBBox();

		//void setScaleDepthMinMax(float min, float max);

		bool contains(float x, float y);
		bool contains(float x, float y, float z);

		
		//dynamic terrain mod:
		int getNumXYCells();
		void getXYZofCell(int cellIndex, float *lonX, float *latY, float *depth);

		float getMinDepth();
		float getMaxDepth();

		void setCoordinateScaler(CoordinateScaler *Scaler);
		float getScaledXMin();
		float getScaledXMax();
		float getScaledYMin();
		float getScaledYMax();
		float getScaledMinDepth();
		float getScaledMaxDepth();

		CoordinateScaler *scaler;

		float xMin, xMax, xRange, xCellsFloat, xCellSize;
		int xCells;
		float yMin, yMax, yRange, yCellsFloat, yCellSize;
		int yCells;
		
		void exportSurfaceLayerToImage(char* filename);
		
		float zCellsFloat;
		int zCells;
		float *depthValues;

		int gridSize2d, gridSize3d, gridSize4d;
		int xyzCells, xyCells;

		
		float lastTimeRequested;
		bool lastTimeOnTimestep;
		int lastTime1, lastTime2;
		float lastTimeFactor1, lastTimeFactor2;
	
		float maxVelocity;

		float* uValues;
		float* vValues;
		float* velocityValues;
		//float* tValues;
		//float* sValues;
		float* times;
		int numTimesteps;
		
		int activeTimestep;

		bool *isWater;
				
		//float *bathyDepth2d;
		//float minBathyDepth;
		//float maxBathyDepth;
		bool depthsSet;


		//particle sys stuff:
		bool enableIllustrativeParticles;
		int numIllustrativeParticles;
		float colorIllustrativeParticles[3];
		float illustrativeParticleVelocityScale;

		float illustrativeParticleLifetime;
		float illustrativeParticleTrailTime;
		float illustrativeParticleSize;

		//bbox access:
		float getXMin();
		float getXMax();
		float getYMin();
		float getYMax();

		//direct cell access (used for ROIs)
		bool getCellBounds(float xmin, float xmax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax);
		float getXCellSize();
		float getYCellSize();
		float getDepthAtCell(int xcell, int ycell, float time);
		float getTimeAtTimestep(int timestep);
		int getNumTimeCells();
		
		char* getName();
		void setName(char* Name);
		char name[512];

		float currentTime;
		float minTime;
		float maxTime;


	private:

		////terrain draw mod:
		int terrainVBOID;
		GLuint terrainQuadsPositionsVBO;
		GLuint terrainQuadsColorsVBO;
		GLuint terrainQuadsTextCoordsVBO;

		bool* terrainTileNorth;
		bool* terrainTileSouth;
		bool* terrainTileEast;
		bool* terrainTileWest;
		int numTerrainQuads;
		bool terrainHeightsExtracted;

		
};

#endif
