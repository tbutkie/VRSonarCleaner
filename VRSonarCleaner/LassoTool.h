#ifndef LASSOTOOL_H
#define LASSOTOOL_H

#include <vector>

#include <glm.hpp>

#include "BehaviorBase.h"
#include "Renderer.h"

class LassoTool : public InitializableBehavior
{
public:
	LassoTool();
	~LassoTool();

	void init();
	void update();
	void draw();

	void start(int mx, int my);
	void move(int mx, int my);
	void end();

	bool readyToCheck();

	std::vector<glm::vec3> getPoints();

	bool checkPoint(glm::vec2 testPt);

	void reset();

private:
	void precalc();

	std::vector<glm::vec3> m_vvec3LassoPoints;
	std::vector<unsigned short> m_vusLassoIndices;
	std::vector<glm::vec4> m_vvec4Colors;

	bool m_bLassoActive, m_bShowBBox, m_bShowConnector, m_bPrecalcsDone;
	glm::vec2 m_vec2MinBB, m_vec2MaxBB;

	// variables for precomputations to speed up Point-in-Poly test
	std::vector<float> m_vfConstants, m_vfMultiplicands;

	unsigned int m_glVAO, m_glVBO, m_glEBO;
};

#endif
