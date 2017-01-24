#ifndef LASSOTOOL_H
#define LASSOTOOL_H

#include <vector>

#include <shared/glm/glm.hpp>

class LassoTool
{
public:
	LassoTool();
	~LassoTool();

	void start(int mx, int my);
	void move(int mx, int my);
	void end();

	bool readyToCheck();

	bool checkPoint(glm::vec2 testPt);

	void draw();

	void reset();

private:
	void precalc();

	std::vector<glm::vec2> m_vvec3LassoPoints;

	bool m_bLassoActive, m_bShowBBox, m_bShowConnector, m_bPrecalcsDone;
	glm::vec2 m_vec2MinBB, m_vec2MaxBB;

	// variables for precomputations to speed up Point-in-Poly test
	std::vector<float> m_vfConstants, m_vfMultiplicands;
};

#endif
