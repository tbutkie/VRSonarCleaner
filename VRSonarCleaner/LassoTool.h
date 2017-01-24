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

	bool precalc();
	bool checkPoint(glm::vec2 testPt);

	void draw();

	void reset();

private:
	std::vector<glm::vec2> m_vvec3LassoPoints;

	bool m_bLassoActive, m_bShowBBox;
	glm::vec2 m_vec2MinBB, m_vec2MaxBB;

	// variables for precomputations to speed up Point-in-Poly test
	std::vector<float> m_vfConstants, m_vfMultiplicands;
};

#endif
