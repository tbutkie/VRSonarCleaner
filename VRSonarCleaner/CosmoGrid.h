#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm> 
#include <vector>
#include <chrono>
#include <GL/glew.h>
#include <glm.hpp>
#include "Dataset.h"

class CosmoGrid : public Dataset
{
	public:
		CosmoGrid(const char* filename);
		virtual ~CosmoGrid();

		void init();

		void deleteSelf();

		void setTimeValue(int timeIndex, float timeValue);		
		
		void setCellValue(int x, int y, int z, int timestep, float u, float v);
		void setCellValue(int x, int y, int z, int timestep, float u, float v, float w);
		
		bool getUVWat(float lonX, float latY, float depth, float time, float *u, float *v, float *w);
		bool getVelocityAt(float lonX, float latY, float depth, float time, float *velocity);

		bool contains(float x, float y);
		bool contains(float x, float y, float z);
		
		//dynamic terrain mod:
		int getNumXYCells();
		void getXYZofCell(int cellIndex, float *lonX, float *latY, float *depth);

		//bbox access:
		float getXMin();
		float getXMax();
		float getYMin();
		float getYMax();
		float getZMin();
		float getZMax();

		//direct cell access (used for ROIs)
		bool getCellBounds(float xmin, float xmax, float ymin, float ymax, int *xcellmin, int *xcellmax, int *ycellmin, int *ycellmax);
		float getXCellSize();
		float getYCellSize();
		float getZCellSize();
		float getTimeAtTimestep(int timestep);
		int getNumTimeCells();

		char* getName();
		void setName(const char* name);
		
public:
		float m_fXMin, m_fXMax, m_fXRange, m_fXCells, m_fXCellSize;
		int m_nXCells;
		float m_fYMin, m_fYMax, m_fYRange, m_fYCells, m_fYCellSize;
		int m_nYCells;		
		float m_fZMin, m_fZMax, m_fZRange, m_fZCells, m_fZCellSize;
		int m_nZCells;

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

		//particle sys stuff:
		bool m_bIllustrativeParticlesEnabled;
		int m_nIllustrativeParticles;
		glm::vec3 m_vec3IllustrativeParticlesColor;

		float m_fIllustrativeParticleVelocityScale;
		std::chrono::milliseconds m_fIllustrativeParticleLifetime;
		std::chrono::milliseconds m_fIllustrativeParticleTrailTime;
		float m_fIllustrativeParticleSize;


		char m_strName[512];

		float m_fMinTime;
		float m_fMaxTime;
};
