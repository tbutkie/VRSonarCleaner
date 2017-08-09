#ifndef __FlowGrid_h__
#define __FlowGrid_h__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm> 
#include <vector>
#include <GL/glew.h>
#include <shared/glm/glm.hpp>
#include "CoordinateScaler.h"

class FlowGrid
{
	public:
		//FlowGrid(float minX, float maxX, int cellsX, float minY, float maxY, int cellsY, int cellsZ, int timesteps);
		FlowGrid(char* filename, bool useZInsteadOfDepth);
		virtual ~FlowGrid();

		void init();

		void deleteSelf();

		void setDepthValue(int depthIndex, float depth);

		void setTimeValue(int timeIndex, float timeValue);		
		
		void setCellValue(int x, int y, int z, int timestep, float u, float v);
		void setCellValue(int x, int y, int z, int timestep, float u, float v, float w);
		//void setCellValue(int x, int y, int z, int timestep, float u, float v, float t, float s);

		void setIsWaterValue(int x, int y, int z, int t, bool isCellWater);
		
		bool getUVat(float lonX, float latY, float depth, float time, float *u, float *v);
		bool getUVWat(float lonX, float latY, float depth, float time, float *u, float *v, float *w);
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

		//bbox access:
		float getXMin();
		float getXMax();
		float getYMin();
		float getYMax();

		//direct cell access (used for ROIs)
		bool getCellBounds(float xmin, float xmax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax);
		float getXCellSize();
		float getYCellSize();
		float getTimeAtTimestep(int timestep);
		int getNumTimeCells();

		char* getName();
		void setName(char* name);
		
public:
		CoordinateScaler *scaler;

		float m_fXMin, m_fXMax, m_fXRange, m_fXCells, m_fXCellSize;
		int m_nXCells;
		float m_fYMin, m_fYMax, m_fYRange, m_fYCells, m_fYCellSize;
		int m_nYCells;
		
		float m_fZCells;
		int m_nZCells;
		std::vector<float> m_vDepthValues;

		int m_nGridSize2d, m_nGridSize3d, m_nGridSize4d;
		int m_nXYZCells, m_nXYCells;

		
		float m_fLastTimeRequested;
		bool m_bLastTimeOnTimestep;
		int m_iLastTime1, m_iLastTime2;
		float m_fLastTimeFactor1, m_fLastTimeFactor2;
	
		float m_fMaxVelocity;

		float* m_arrfUValues;
		float* m_arrfVValues;
		float* m_arrfWValues;
		float* m_arrfVelocityValues;
		//float* tValues;
		//float* sValues;
		float* m_arrfTimes;
		int m_nTimesteps;
		
		int m_nActiveTimestep;

		bool* m_arrbIsWater;
				
		//float *bathyDepth2d;
		//float minBathyDepth;
		//float maxBathyDepth;
		bool m_bDepthsSet;
		bool m_bUsesZInsteadOfDepth;

		//particle sys stuff:
		bool m_bIllustrativeParticlesEnabled;
		int m_nIllustrativeParticles;
		glm::vec3 m_vec3IllustrativeParticlesColor;

		float m_fIllustrativeParticleVelocityScale;
		float m_fIllustrativeParticleLifetime;
		float m_fIllustrativeParticleTrailTime;
		float m_fIllustrativeParticleSize;


		char m_strName[512];

		float m_fCurrentTime;
		float m_fMinTime;
		float m_fMaxTime;
};

#endif
