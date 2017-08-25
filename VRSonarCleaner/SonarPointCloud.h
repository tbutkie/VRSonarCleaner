#ifndef __SonarPointCloud_h__
#define __SonarPointCloud_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include "Dataset.h"
#include "ColorScaler.h"

#include "../thirdparty/OpenNS_1.6.0/include/bag.h"

#include <shared/glm/glm.hpp>

class SonarPointCloud : public Dataset
{
	public:
		SonarPointCloud(ColorScaler * const colorScaler);
		~SonarPointCloud();
		
		bool getRefreshNeeded();
		void setRefreshNeeded();
		void update();

		GLuint getVAO();
		GLsizei getPointCount();
		GLuint getPreviewVAO();
		GLsizei getPreviewPointCount();

		//methods:

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
		double getMinDepthTPU();
		double getMaxDepthTPU();
		double getMinPositionalTPU();
		double getMaxPositionalTPU();
		
		std::string getName();
		void setName(std::string n);

	private:
		//variables
		std::string m_strName;
		float minDepthTPU, maxDepthTPU, minPositionalTPU, maxPositionalTPU;

		std::vector<glm::dvec3> m_vvec3RawPointsPositions;
		std::vector<glm::vec3> m_vvec3AdjustedPointsPositions;
		std::vector<glm::vec4> m_vvec4PointsColors;
		std::vector<GLuint> m_vuiIndicesFull;
		std::vector<GLuint> m_vuiPointsMarks;
		std::vector<float> m_vfPointsDepthTPU;
		std::vector<float> m_vfPointsPositionTPU;
		int m_nPoints;
		bool m_bPointsAllocated;
		bool m_bFirstMinMaxSet;

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

		void adjustPoints();
		void createAndLoadBuffers();
};

#endif
