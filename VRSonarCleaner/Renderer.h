#pragma once

#include <vector>

// GL Includes
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iterator>

#include "ShaderUtils.h"

class Renderer
{
public:
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer instance;
		return instance;
	}

private:
	struct DebugVertex {
		glm::vec3 pos;
		glm::vec4 col;

		DebugVertex(glm::vec3 p, glm::vec4 c)
			: pos(p)
			, col(c)
		{}
	};

	GLuint m_glVAO, m_glVBO;
	GLuint m_glTransformProgramID;
	GLint m_glViewProjectionMatrixLocation;
	std::vector<DebugVertex> m_vPointsVertices, m_vLinesVertices, m_vTrianglesVertices;
	glm::mat4 m_mat4Transform;

	static Renderer *s_instance;

	// CTOR
	Renderer()
	{
		_init();
	}

	void _init()
	{
	}

// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;
};
