#ifndef __SonarPointCloud_h__
#define __SonarPointCloud_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "ColorScaler.h"

#include "../thirdparty/OpenNS_1.6.0/include/bag.h"

#include <shared/glm/glm.hpp>

class SonarPointCloud
{
	public:
		SonarPointCloud(ColorScaler * const colorScaler);
		~SonarPointCloud();

		void markForDeletion();
		bool shouldBeDeleted();
		bool markedForDeletion;
		
		bool getRefreshNeeded();
		void setRefreshNeeded();
		void update();

		GLuint getVAO();
		GLsizei getPointCount();
		GLuint getPreviewVAO();
		GLsizei getPreviewPointCount();

		//methods:
		void deleteSelf();
	
		void initPoints(int numPoints);
		void setPoint(int index, double lonX, double latY, double depth);
		void setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU);
		void setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b);

		bool loadFromSonarTxt(char* filename);

		bool generateFakeCloud(float xSize, float ySize, float zSize, int numPoints);

		int colorScale;

		void setColorScale(int scale);
		int getColorScale();
		void setColorMode(int mode);  //2=static color
		int getColorMode();
		void setColorScope(int mode);
		int getColorScope();
		
		void markPoint(int index, int code);
		void resetAllMarks();

		//cleaning
		std::vector<glm::vec3> getPointPositions();

		int getPointMark(int index);

		//bounds access:
		double getXMin();
		double getXMax();
		double getYMin();
		double getYMax();
		double getMinDepth();
		double getMaxDepth();
		double getActualRemovedXMin();
		double getActualRemovedYMin();
		double getMinDepthTPU();
		double getMaxDepthTPU();
		double getMinPositionalTPU();
		double getMaxPositionalTPU();

		void useNewActualRemovedMinValues(double newRemovedXmin, double newRemovedYmin);

		char* getName();
		void setName(char* Name);

	private:
		//variables
		char name[512];
		double xMin, xMax, xRange;
		double yMin, yMax, yRange;
		double minDepth, maxDepth, rangeDepth;
		double actualRemovedXmin, actualRemovedYmin; //stores the actual x and y min of the original data, we subtract them to keep scaling easier for opengl
		float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;

		std::vector<glm::vec3> m_vvec3PointsPositions;
		std::vector<glm::vec4> m_vvec4PointsColors;
		std::vector<GLushort> m_vusIndicesFull;
		int *pointsMarks;
		float *pointsDepthTPU;
		float *pointsPositionTPU;
		int numPoints;
		bool pointsAllocated;
		bool firstMinMaxSet;

		int colorMode;
		int colorScope;

		ColorScaler *m_pColorScaler;
		
		int m_iPreviewReductionFactor;

		//preview
		bool refreshNeeded;
		bool previewRefreshNeeded;

		//OpenGL
		GLuint m_glVAO, m_glVBO, m_glEBO;
		GLuint m_glPreviewVAO;		

		void createAndLoadBuffers();
};

#endif
