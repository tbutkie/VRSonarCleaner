#ifndef __SonarPointCloud_h__
#define __SonarPointCloud_h__

#include <vector>
#include <GL/glew.h>
#include <stdio.h>
#include <math.h>
#include <functional>
#include <future>
#include "Dataset.h"
#include "ColorScaler.h"

#include <bag.h>

#include <glm.hpp>

class SonarPointCloud : public Dataset
{
public:
	enum SONAR_FILETYPE {
		CARIS,
		XYZF,
		QIMERA
	};

	public:
		SonarPointCloud(ColorScaler * const colorScaler, std::string fileName, SONAR_FILETYPE filetype);
		~SonarPointCloud();

		bool ready();
		
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
		glm::dvec3 getRawPointPosition(unsigned int index);
		int getPointMark(unsigned int index);
		float getPointDepthTPU(unsigned int index);
		float getPointPositionTPU(unsigned int index);

		float getMinDepthTPU();
		float getMaxDepthTPU();
		float getMinPositionalTPU();
		float getMaxPositionalTPU();

		static bool s_funcDepthTPUMinCompare(SonarPointCloud* const &lhs, SonarPointCloud* const &rhs);
		static bool s_funcDepthTPUMaxCompare(SonarPointCloud* const &lhs, SonarPointCloud* const &rhs);

		static bool s_funcPosTPUMinCompare(SonarPointCloud* const &lhs, SonarPointCloud* const &rhs);
		static bool s_funcPosTPUMaxCompare(SonarPointCloud* const &lhs, SonarPointCloud* const &rhs);

	private:
		std::future<bool> m_Future;

		//variables
		float m_fMinDepthTPU, m_fMaxDepthTPU, m_fMinPositionalTPU, m_fMaxPositionalTPU;

		std::vector<glm::dvec3> m_vdvec3RawPointsPositions;
		std::vector<glm::vec3> m_vvec3AdjustedPointsPositions;
		std::vector<glm::vec4> m_vvec4PointsColors;
		std::vector<GLuint> m_vuiPointsMarks;
		std::vector<float> m_vfPointsDepthTPU;
		std::vector<float> m_vfPointsPositionTPU;
		unsigned int m_nPoints;
		bool m_bPointsAllocated;

		GLuint m_glInstancedSpriteVBO;

		int colorMode;
		int colorScope;

		ColorScaler *m_pColorScaler;
		
		int m_iPreviewReductionFactor;

		//preview
		bool refreshNeeded;
		bool previewRefreshNeeded;

		//OpenGL
		GLuint m_glVAO, m_glPointsVBO;
		GLuint m_glPreviewVAO;		

		bool loadCARISTxt();
		bool loadQimeraTxt();
		bool loadStudyCSV();

		glm::vec3 getDefaultPointColor(unsigned int index);
		void adjustPoints();
		void createAndLoadBuffers();
};

#endif
