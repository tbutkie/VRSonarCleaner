#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <vector>

#include <glm.hpp>

class HolodeckBackground
{
public:
	HolodeckBackground(glm::vec3 roomSizeMeters, float gridSpacingMeters);
	virtual ~HolodeckBackground();

	GLuint getVAO();
	GLuint getVertexCount();

private:
	struct HolodeckVertex {
		glm::vec3 pos;
		glm::vec4 col;

		HolodeckVertex(glm::vec3 p, glm::vec4 c)
			: pos(p)
			, col(c)
		{}
	};


private:
	void draw();
	void drawGrids(glm::vec3 color, float spacingFactor);

private:
	glm::vec3 m_vec3RoomSize;
	float m_fGridSpacing;
	glm::vec3 m_vec3RoomSpacings;
	glm::vec3 m_vec3Spaces;
	glm::vec3 m_vec3RoomMin;
	glm::vec3 m_vec3RoomMax;

	std::vector<HolodeckVertex> m_vHolodeckGeometry;

	GLuint m_glVAO;
	GLuint m_glVBO, m_glEBO;
};