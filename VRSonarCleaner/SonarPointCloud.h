#ifndef __SonarPointCloud_h__
#define __SonarPointCloud_h__

#include <windows.h>
#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "Vec3.h"
#include "Vec3Double.h"
#include "ColorScaler.h"


extern ColorScaler *colorScalerTPU;

class SonarPointCloud
{
	public:
		SonarPointCloud();
		~SonarPointCloud();

		void markForDeletion();
		bool shouldBeDeleted();
		bool markedForDeletion;
		
		bool getRefreshNeeded();
		void setRefreshNeeded();
		bool refreshNeeded;

		//methods:
		void deleteSelf();
	
		void initPoints(int numPoints);
		void setPoint(int index, double lonX, double latY, double depth);
		void setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU);
		void setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b);


		bool loadFromASCII(char* filename, int linesToSkip);
		bool loadFromSonarTxt(char* filename);

		float getMinValidTime();
		float getMaxValidTime();
		void setValidTimes(bool infiniteMin, float minTime, bool infiniteMax, float maxTime);

		//VBOs
		void buildPointsVBO();
		void drawPointsVBO();
		void refreshPointsVBO();

		void draw();

		void drawAxes();

		int colorScale;

		void setColorScale(int scale);
		int getColorScale();
		void setColorMode(int mode);  //2=static color
		int getColorMode();
		void setColorScope(int mode);
		int getColorScope();
		
		//bbox access:
		double getXMin();
		double getXMax();
		double getYMin();
		double getYMax();
		double getMinDepth();
		double getMaxDepth();

		char* getName();
		void setName(char* Name);
		char name[512];

	private:
		//variables
		double xMin, xMax, xRange;
		double yMin, yMax, yRange;
		double minDepth, maxDepth, rangeDepth;
		double actualRemovedXmin, actualRemovedYmin; //stores the actual x and y min of the original data, we subtract them to keep scaling easier for opengl
		float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;

		double *pointsPositions;
		float *pointsColors;
		float *pointsDepthTPU;
		float *pointsPositionTPU;
		int numPoints;
		bool pointsAllocated;
		bool firstMinMaxSet;

		int colorMode;
		int colorScope;
		
		//VBOs
		bool glewInited;
		bool buffersGenerated;
		
		GLuint pointsPositionsVBO;
		GLuint pointsColorsVBO;
		int numPointsInVBO;
};

#endif
