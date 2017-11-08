#include "LassoTool.h"

#include "DataLogger.h"

#include <gtc/type_ptr.hpp> // glm::value_ptr
#include <gtc/matrix_transform.hpp> // glm::project

#include <GL/glew.h>

#include <sstream>

const glm::vec4 g_vec4ActiveLineColor(0.25f, 0.65f, 0.25f, 1.f);
const glm::vec4 g_vec4LineColor(0.65f, 0.25f, 0.25f, 1.f);
const glm::vec4 g_vec4ConnectorColor(0.75f, 0.75f, 0.75f, 1.f);
const glm::vec4 g_vec4BBoxColor(1.f, 1.f, 1.f, 1.f);
const glm::vec4 g_vec4WindowFrameColor(1.f, 0.f, 1.f, 1.f);

const float g_fBBoxPadding(2.f);

LassoTool::LassoTool()
	: m_bLassoActive(false)
	, m_bShowBBox(false)
	, m_bShowConnector(true)
	, m_bPrecalcsDone(false)
	, m_vec2MinBB(glm::vec2(0.f))
	, m_vec2MaxBB(glm::vec2(0.f))
{
}

LassoTool::~LassoTool()
{
}

void LassoTool::init()
{
	glGenVertexArrays(1, &m_glVAO);
	glGenBuffers(1, &m_glVBO);
	glGenBuffers(1, &m_glEBO);

	glBindVertexArray(this->m_glVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);

	glBindVertexArray(0);
}

void LassoTool::update()
{
}

void LassoTool::draw()
{
	int n = m_vvec3LassoPoints.size();

	if (n < 3)
		return;

	int ptsToErase = 0;

	m_vvec4Colors = std::vector<glm::vec4>(m_vvec3LassoPoints.size(), m_bLassoActive ? g_vec4ActiveLineColor : g_vec4LineColor);

	// Lasso segments
	for (int i = 0; i < n - 1; ++i)
	{
		m_vusLassoIndices.push_back((unsigned short)i);
		m_vusLassoIndices.push_back((unsigned short)i + 1);
	}

	// connecting line from last to first points
	if (m_bLassoActive)
	{
		if (m_bShowConnector)
		{
			m_vvec3LassoPoints.push_back(m_vvec3LassoPoints.back());
			m_vvec4Colors.push_back(g_vec4ConnectorColor);
			m_vvec3LassoPoints.push_back(m_vvec3LassoPoints.front());
			m_vvec4Colors.push_back(g_vec4ConnectorColor);
			m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 1);
			m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 2);

			ptsToErase += 2;
		}
	}
	else if (n > 2)
	{
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 1);
		m_vusLassoIndices.push_back((unsigned short)0);
	}

	// Lasso bounding box
	if (m_bShowBBox && m_bLassoActive)
	{
		// Push bbox points
		m_vvec3LassoPoints.push_back(glm::vec3(m_vec2MinBB.x - g_fBBoxPadding, m_vec2MinBB.y - g_fBBoxPadding, 0.f));
		m_vvec4Colors.push_back(g_vec4BBoxColor);
		m_vvec3LassoPoints.push_back(glm::vec3(m_vec2MinBB.x - g_fBBoxPadding, m_vec2MaxBB.y + g_fBBoxPadding, 0.f));
		m_vvec4Colors.push_back(g_vec4BBoxColor);
		m_vvec3LassoPoints.push_back(glm::vec3(m_vec2MaxBB.x + g_fBBoxPadding, m_vec2MaxBB.y + g_fBBoxPadding, 0.f));
		m_vvec4Colors.push_back(g_vec4BBoxColor);
		m_vvec3LassoPoints.push_back(glm::vec3(m_vec2MaxBB.x + g_fBBoxPadding, m_vec2MinBB.y - g_fBBoxPadding, 0.f));
		m_vvec4Colors.push_back(g_vec4BBoxColor);

		ptsToErase += 4;

		// Push bbox indices
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 4);
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 3);

		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 3);
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 2);

		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 2);
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 1);

		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 1);
		m_vusLassoIndices.push_back((unsigned short)m_vvec3LassoPoints.size() - 4);
	}

	assert(m_vvec3LassoPoints.size() == m_vvec4Colors.size());

	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	// Buffer orphaning
	glBufferData(GL_ARRAY_BUFFER, m_vvec3LassoPoints.size() * sizeof(glm::vec3) + m_vvec4Colors.size() * sizeof(glm::vec4), 0, GL_STREAM_DRAW);
	// Sub buffer data for points, then colors
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_vvec3LassoPoints.size() * sizeof(glm::vec3), &m_vvec3LassoPoints[0]);
	glBufferSubData(GL_ARRAY_BUFFER, m_vvec3LassoPoints.size() * sizeof(glm::vec3), m_vvec4Colors.size() * sizeof(glm::vec4), &m_vvec4Colors[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vusLassoIndices.size() * sizeof(unsigned short), 0, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vusLassoIndices.size() * sizeof(unsigned short), &m_vusLassoIndices[0], GL_STREAM_DRAW);

	// Set color sttribute pointer now that point array size is known
	glBindVertexArray(this->m_glVAO);
	glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(m_vvec3LassoPoints.size() * sizeof(glm::vec3)));
	glBindVertexArray(0);

	Renderer::RendererSubmission rs;
	rs.modelToWorldTransform = glm::mat4();
	rs.glPrimitiveType = GL_LINES;
	rs.shaderName = "flat";
	rs.VAO = m_glVAO;
	rs.vertCount = m_vusLassoIndices.size();
	Renderer::getInstance().addToUIRenderQueue(rs);

	m_vvec3LassoPoints.erase(m_vvec3LassoPoints.end() - ptsToErase, m_vvec3LassoPoints.end());
	m_vusLassoIndices.erase(m_vusLassoIndices.end() - ptsToErase * 2, m_vusLassoIndices.end());
	m_vvec4Colors.clear();
}

// begin arcball rotation
void LassoTool::start(int mx, int my)
{
	reset();

	glm::vec3 newPt(mx, my, 0.f);

	m_vvec3LassoPoints.push_back(newPt);
	m_vec2MinBB = m_vec2MaxBB = glm::vec2(newPt);
	m_bLassoActive = true;

	std::stringstream ss;

	ss << "Lasso Start" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
	ss << "\t";
	ss << "mouse-pos:\"" << newPt.x << "," << newPt.y << "\"";

	DataLogger::getInstance().logMessage(ss.str());
}

// update current arcball rotation
void LassoTool::move(int mx, int my)
{
	if (!m_bLassoActive)
		return;
	
	glm::vec3 newPt(mx, my, 0.f);

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

	std::stringstream ss;

	ss << "Lasso End" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
	ss << "\t";
	ss << "mouse-pos:\"" << m_vvec3LassoPoints.back().x << "," << m_vvec3LassoPoints.back().y << "\"";

	DataLogger::getInstance().logMessage(ss.str());
}

bool LassoTool::readyToCheck()
{
	if (m_bLassoActive || m_vvec3LassoPoints.size() < 3)
		return false;

	if (!m_bPrecalcsDone)
		precalc();

	return true;
}

std::vector<glm::vec3> LassoTool::getPoints()
{
	return m_vvec3LassoPoints;
}

void LassoTool::reset()
{
	m_vvec3LassoPoints.clear();
	m_vusLassoIndices.clear();
	m_vvec4Colors.clear();
	m_vec2MinBB = glm::vec2(0.f);
	m_vec2MaxBB = glm::vec2(0.f);
	m_bPrecalcsDone = false;
	m_vfConstants.clear();
	m_vfMultiplicands.clear();
}

bool LassoTool::checkPoint(glm::vec2 testPt)
{
	size_t n = m_vvec3LassoPoints.size();

	if (m_bLassoActive || n < 3)
		return false;

	if (!m_bPrecalcsDone)
		precalc();

	// fast-fail via bounding box
	if (testPt.x < m_vec2MinBB.x
		|| testPt.y < m_vec2MinBB.y
		|| testPt.x > m_vec2MaxBB.x
		|| testPt.y > m_vec2MaxBB.y)
		return false;

	// within bounding box, so do a full check
	int j = n - 1;
	bool oddNodes = false;

	for (int i = 0; i < n; ++i) {
		if ((m_vvec3LassoPoints[i].y < testPt.y && m_vvec3LassoPoints[j].y >= testPt.y
			|| m_vvec3LassoPoints[j].y < testPt.y && m_vvec3LassoPoints[i].y >= testPt.y)) {
			oddNodes ^= (testPt.y * m_vfMultiplicands[i] + m_vfConstants[i] < testPt.x);
		}
		j = i;
	}

	return oddNodes;
}

void LassoTool::precalc()
{
	size_t n = m_vvec3LassoPoints.size();

	m_vfConstants.resize(n);
	m_vfMultiplicands.resize(n);

	int j = n - 1;
	for (int i = 0; i < n; ++i) {
		if (m_vvec3LassoPoints[j].y == m_vvec3LassoPoints[i].y) {
			m_vfConstants[i] = m_vvec3LassoPoints[i].x;
			m_vfMultiplicands[i] = 0;
		}
		else {
			m_vfConstants[i] = m_vvec3LassoPoints[i].x - (m_vvec3LassoPoints[i].y * m_vvec3LassoPoints[j].x) / (m_vvec3LassoPoints[j].y - m_vvec3LassoPoints[i].y) + (m_vvec3LassoPoints[i].y * m_vvec3LassoPoints[i].x) / (m_vvec3LassoPoints[j].y - m_vvec3LassoPoints[i].y);
			m_vfMultiplicands[i] = (m_vvec3LassoPoints[j].x - m_vvec3LassoPoints[i].x) / (m_vvec3LassoPoints[j].y - m_vvec3LassoPoints[i].y);
		}
		j = i;
	}

	m_bPrecalcsDone = true;
}