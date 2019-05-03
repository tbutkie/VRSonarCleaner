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
		
		glm::vec3 getUVWat(glm::vec3 pos);
		float getVelocityAt(glm::vec3 pos);

		bool contains(glm::vec3 pos);

		//bbox access:
		float getXMin();
		float getXMax();
		float getYMin();
		float getYMax();
		float getZMin();
		float getZMax();

		//direct cell access (used for ROIs)
		float getXCellSize();
		float getYCellSize();
		float getZCellSize();

		char* getName();
		void setName(const char* name);

		glm::vec3 rk4(glm::vec3 pos, float delta);
		
public:
		void setPointUVWValue(int x, int y, int z, float u, float v, float w);
		int gridIndex(int xInd, int yInd, int zInd);
		float trilinear(float** arr, glm::vec3 pos);

		float m_fXMin, m_fXMax, m_fXRange, m_fXCellSize;
		int m_nXPoints;
		float m_fYMin, m_fYMax, m_fYRange, m_fYCellSize;
		int m_nYPoints;		
		float m_fZMin, m_fZMax, m_fZRange, m_fZCellSize;
		int m_nZPoints;

		int m_nGridSize2d, m_nGridSize3d;
		int m_nXYZPoints, m_nXYPoints;
	
		float m_fMinVelocity, m_fMaxVelocity;

		float* m_arrfUValues;
		float* m_arrfVValues;
		float* m_arrfWValues;
		float* m_arrfVelocityValues;
		float* m_arrDensityValues;
		float* m_arrH2IIDensityValues;
		float* m_arrTemperatureValues;

		char m_strName[512];
};
