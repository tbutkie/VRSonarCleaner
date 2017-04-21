#include "HolodeckBackground.h"

#include <shared/glm/glm.hpp>

#include "Renderer.h"

HolodeckBackground::HolodeckBackground(glm::vec3 roomSizeMeters, float spacingMeters)
	: m_vec3RoomSize(roomSizeMeters)
	, m_fGridSpacing(spacingMeters)
{
	m_vec3Spaces = floor(m_vec3RoomSize / m_fGridSpacing);
	m_vec3RoomSpacings = m_vec3RoomSize / m_vec3Spaces;

	m_vec3RoomMin = -m_vec3RoomSize * 0.5f;
	m_vec3RoomMin.y = 0.f;

	m_vec3RoomMax = m_vec3RoomSize * 0.5f;
	m_vec3RoomMax.y = m_vec3RoomSize.y;

	draw();

	Renderer::RendererSubmission rs;
	rs.primitiveType = GL_LINES;
	rs.modelToWorldTransform = glm::mat4();
	rs.shaderName = "debug";
	rs.VAO = m_glVAO;
	rs.vertCount = static_cast<GLuint>(m_vHolodeckGeometry.size());

	Renderer::getInstance().addToStaticRenderQueue(rs);
}

HolodeckBackground::~HolodeckBackground()
{
}

GLuint HolodeckBackground::getVAO()
{
	return m_glVAO;
}

GLuint HolodeckBackground::getVertexCount()
{
	return static_cast<GLuint>(m_vHolodeckGeometry.size());
}

void HolodeckBackground::draw()
{
	//DebugDrawer::getInstance().setTransformDefault();

	glm::vec3 majorGridColor(0.15f, 0.21f, 0.31f);
	glm::vec3 minorGridColor(0.23f, 0.29f, 0.39f);

	drawGrids(majorGridColor, 1.f);
	drawGrids(minorGridColor, 0.25f);

	std::vector<GLushort> inds;
	GLushort i = 0;
	for (auto v : m_vHolodeckGeometry)
		inds.push_back(i++);

	// Create buffers/arrays
	glGenVertexArrays(1, &m_glVAO);
	glGenBuffers(1, &m_glVBO);
	glGenBuffers(1, &m_glEBO);

	glBindVertexArray(this->m_glVAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	glBufferData(GL_ARRAY_BUFFER, m_vHolodeckGeometry.size() * sizeof(HolodeckVertex), m_vHolodeckGeometry.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLushort), inds.data(), GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(HolodeckVertex), (GLvoid*)0);
	// Vertex Colors
	glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
	glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(HolodeckVertex), (GLvoid*)offsetof(HolodeckVertex, col));

	glBindVertexArray(0);
}

void HolodeckBackground::drawGrids(glm::vec3 color, float spacingFactor)
{
	glm::vec4 floorOpacity(color, 0.3f);
	glm::vec4 ceilingOpacity(color, 0.03f);

	for (float x = m_vec3RoomMin.x; x <= m_vec3RoomMax.x; x += m_vec3RoomSpacings.x * spacingFactor)
	{
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), ceilingOpacity));
		
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), ceilingOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), floorOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), ceilingOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), ceilingOpacity));
	}

	for (float y = m_vec3RoomMin.y; y <= m_vec3RoomMax.y; y += m_vec3RoomSpacings.y * spacingFactor)
	{
		glm::vec4 inBetweenOpacity = ((1.f - (y / m_vec3RoomMax.y)) * (floorOpacity - ceilingOpacity)) + ceilingOpacity;

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), inBetweenOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), inBetweenOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), inBetweenOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), inBetweenOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), inBetweenOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), inBetweenOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity));
	}

	for (float z = m_vec3RoomMin.z; z <= m_vec3RoomMax.z; z += m_vec3RoomSpacings.z * spacingFactor)
	{
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), floorOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), ceilingOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), ceilingOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), ceilingOpacity));

		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), floorOpacity));
		m_vHolodeckGeometry.push_back(HolodeckVertex(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), ceilingOpacity));
	}
}