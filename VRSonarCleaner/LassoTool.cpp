#include "LassoTool.h"

#include "PolyUtil.h"

#include <shared/glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <shared/glm/gtc/matrix_transform.hpp> // glm::unproject

#include <GL/glew.h>

const glm::vec3 g_vec3ActiveLineColor(0.25, 0.65, 0.25);
const glm::vec3 g_vec3LineColor(0.65, 0.25, 0.25);
const glm::vec3 g_vec3ConnectorColor(0.75, 0.75, 0.75);

LassoTool::LassoTool()
	: m_bLassoActive(false)
	, m_vec2MinBB(glm::vec2(0.f))
	, m_vec2MaxBB(glm::vec2(0.f))
{
}

LassoTool::~LassoTool()
{
}

// begin arcball rotation
void LassoTool::start(int mx, int my)
{
	reset();

	glm::vec2 newPt(mx, my);

	m_vvec3LassoPoints.push_back(newPt);
	m_vec2MinBB = m_vec2MaxBB = newPt;
	m_bLassoActive = true;
}

// update current arcball rotation
void LassoTool::move(int mx, int my)
{
	if (!m_bLassoActive)
		return;
	
	glm::vec2 newPt(mx, my);

	if (m_vvec3LassoPoints.back() != newPt)
	{
		m_vvec3LassoPoints.push_back(newPt);

		if (newPt.x < m_vec2MinBB.x)
			m_vec2MinBB.x = newPt.x;
		if (newPt.x > m_vec2MaxBB.x)
			m_vec2MaxBB.x = newPt.x;
		if (newPt.y < m_vec2MinBB.y)
			m_vec2MinBB.y = newPt.y;
		if (newPt.y > m_vec2MaxBB.y)
			m_vec2MaxBB.y = newPt.y;
	}
}

void LassoTool::end()
{
	m_bLassoActive = false;
}

bool LassoTool::checkPoint(glm::vec2 testPt)
{
	// no lasso = no check
	if (m_vvec3LassoPoints.size() < 1)
		return false;

	// fast-fail via bounding box
	if (testPt.x < m_vec2MinBB.x
		|| testPt.y < m_vec2MinBB.y
		|| testPt.x > m_vec2MaxBB.x
		|| testPt.y > m_vec2MaxBB.y)
		return false;

	// the meat
	return PolyUtil::inMultiPartPoly(m_vvec3LassoPoints, testPt);
}

void LassoTool::draw()
{	
	int n = m_vvec3LassoPoints.size();

	if (n == 0)
		return;

	if(m_bLassoActive)
		glColor3f(g_vec3ActiveLineColor.r, g_vec3ActiveLineColor.g, g_vec3ActiveLineColor.b);
	else
		glColor3f(g_vec3LineColor.r, g_vec3LineColor.g, g_vec3LineColor.b);

	glLineWidth(1.5);
	glBegin(GL_LINES);
		for (int i = 0; i < n - 1; ++i)
		{
			glVertex2f(m_vvec3LassoPoints[i].x, m_vvec3LassoPoints[i].y);
			glVertex2f(m_vvec3LassoPoints[i + 1].x, m_vvec3LassoPoints[i + 1].y);
		}

		// connecting line from last to first points
		if (m_bLassoActive) glColor3f(g_vec3ConnectorColor.r, g_vec3ConnectorColor.g, g_vec3ConnectorColor.b);
		glVertex2f(m_vvec3LassoPoints.back().x, m_vvec3LassoPoints.back().y);
		glVertex2f(m_vvec3LassoPoints.front().x, m_vvec3LassoPoints.front().y);
	glEnd();	
}

void LassoTool::reset()
{
	m_vvec3LassoPoints.clear();
	m_vec2MinBB = glm::vec2(0.f);
	m_vec2MaxBB = glm::vec2(0.f);
}
