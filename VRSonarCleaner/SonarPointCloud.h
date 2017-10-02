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
		SonarPointCloud(ColorScaler * const colorScaler, std::string fileName, bool studyfile = false);
		~SonarPointCloud();
		
		bool getRefreshNeeded();
		void setRefreshNeeded();
		void update();

		GLuint getVAO();
		unsigned int getPointCount();
		GLuint getPreviewVAO();
		unsigned int getPreviewPointCount();

		//methods:

		void initPoints(int numPoints);
		void setPoint(int index, double lonX, double latY, double depth);
		void setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU);
		void setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b);

		bool loadFromSonarTxt();

		bool loadStudyData();

		int colorScale;

		void setColorScale(int scale);
		int getColorScale();
		void setColorMode(int mode);  //2=static color
		int getColorMode();
		void setColorScope(int mode);
		int getColorScope();
		
		void markPoint(unsigned int index, int code);
		void resetAllMarks();

		glm::vec3 getAdjustedPointPosition(unsigned int index);
		glm::vec3 getRawPointPosition(unsigned int index);
		int getPointMark(unsigned int index);
		float getPointDepthTPU(unsigned int index);
		float getPointPositionTPU(unsigned int index);

		float getMinDepthTPU();
		float getMaxDepthTPU();
		float getMinPositionalTPU();
		float getMaxPositionalTPU();

	private:
		//variables
		float m_fMinDepthTPU, m_fMaxDepthTPU, m_fMinPositionalTPU, m_fMaxPositionalTPU;

		std::vector<glm::dvec3> m_vvec3RawPointsPositions;
		std::vector<glm::vec3> m_vvec3AdjustedPointsPositions;
		std::vector<glm::vec4> m_vvec4PointsColors;
		std::vector<GLuint> m_vuiIndicesFull;
		std::vector<GLuint> m_vuiPointsMarks;
		std::vector<float> m_vfPointsDepthTPU;
		std::vector<float> m_vfPointsPositionTPU;
		unsigned int m_nPoints;
		bool m_bPointsAllocated;

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

		glm::vec3 getDefaultPointColor(unsigned int index);
		void adjustPoints();
		void createAndLoadBuffers();
};

#endif
