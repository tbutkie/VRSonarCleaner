#pragma once

#include <vector>

// GL Includes
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include "ShaderUtils.h"

class DebugDrawer
{
public:
	static DebugDrawer& getInstance()
	{
		static DebugDrawer instance;
		return instance;
	}

	// Set the debug drawer's transform using a GLM 4x4 matrix
	// NOTE: Set this before drawing if you want to specify non-world-space coordinates
	// (e.g., set it to an object's model transformation matrix before drawing the model in local space)
	void setTransform(glm::mat4 &m)
	{
		m_mat4Transform = m;
	}

	// Set the debug drawer's transform using an OpenGL-style float[16] array
	// NOTE: Set this before drawing if you want to specify non-world-space coordinates
	// (e.g., set it to an object's model transformation matrix before drawing the model in local space)
	void setTransform(float *m)
	{
		m_mat4Transform = glm::make_mat4(m);
	}

	// Reset the debug drawer's drawing transform so that it draws in world space
	void setTransformDefault()
	{
		m_mat4Transform = glm::mat4();
	}

	// Draw a line using the debug drawer. To draw in a different coordinate space, use setTransform()
	void drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &col = glm::vec3(1.f))
	{
		glm::vec3 from_xformed = glm::vec3(m_mat4Transform * glm::vec4(from, 1.f));
		glm::vec3 to_xformed = glm::vec3(m_mat4Transform * glm::vec4(to, 1.f));

		m_vVertices.push_back(DebugVertex(from_xformed, col));
		m_vVertices.push_back(DebugVertex(to_xformed, col));
	}

	void drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &color)
	{
		drawLine(v0, v1, color);
		drawLine(v1, v2, color);
		drawLine(v2, v0, color);
	}
	void drawTransform(const glm::mat4 &transform, float orthoLen)
	{
		glm::vec3 start(transform[3]);
		drawLine(start, start + glm::vec3(transform * glm::vec4(orthoLen, 0, 0, 0)), glm::vec3(0.7f, 0, 0));
		drawLine(start, start + glm::vec3(transform * glm::vec4(0, orthoLen, 0, 0)), glm::vec3(0, 0.7f, 0));
		drawLine(start, start + glm::vec3(transform * glm::vec4(0, 0, orthoLen, 0)), glm::vec3(0, 0, 0.7f));
	}

	void drawArc(const glm::vec3 &center, const glm::vec3 &normal, const glm::vec3 &axis, float radiusA, float radiusB, float minAngle, float maxAngle,
		const glm::vec3 &color, bool drawSect, float stepDegrees = float(10.f))
	{
		const glm::vec3& vx = axis;
		glm::vec3 vy = glm::cross(normal, axis);
		float step = glm::degrees(stepDegrees);
		int nSteps = (int)glm::abs((maxAngle - minAngle) / step);
		if (!nSteps) nSteps = 1;
		glm::vec3 prev = center + radiusA * vx * glm::cos(minAngle) + radiusB * vy * glm::sin(minAngle);
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
		for (int i = 1; i <= nSteps; i++)
		{
			float angle = minAngle + (maxAngle - minAngle) * float(i) / float(nSteps);
			glm::vec3 next = center + radiusA * vx * glm::cos(angle) + radiusB * vy * glm::sin(angle);
			drawLine(prev, next, color);
			prev = next;
		}
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
	}

	void drawSphere(float radius, float stepDegrees, glm::vec3 &color)
	{
		float minTh = -glm::half_pi<float>();
		float maxTh = glm::half_pi<float>();
		float minPs = -glm::half_pi<float>();
		float maxPs = glm::half_pi<float>();
		glm::vec3 center(0.f, 0.f, 0.f);
		glm::vec3 up(0.f, 1.f, 0.f);
		glm::vec3 axis(1.f, 0.f, 0.f);
		drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
		drawSpherePatch(center, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
	}

	virtual void drawBox(const glm::vec3 &bbMin, const glm::vec3 &bbMax, const glm::vec3 &color)
	{
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMin[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMax[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMax[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMin[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMin[2]), glm::vec3(bbMin[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMin[2]), glm::vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMax[2]), glm::vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMax[2]), glm::vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMax[2]), glm::vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMax[2]), glm::vec3(bbMin[0], bbMin[1], bbMax[2]), color);
	}
	virtual void drawBox(const glm::vec3 &bbMin, const glm::vec3 &bbMax, const glm::mat4 &trans, const glm::vec3 &color)
	{
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMin[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMin[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMin[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMin[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMin[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMax[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMin[1], bbMax[2], 1.f)), glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMax[0], bbMax[1], bbMax[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMax[2], 1.f)), color);
		drawLine(glm::vec3(trans * glm::vec4(bbMin[0], bbMax[1], bbMax[2], 1.f)), glm::vec3(trans * glm::vec4(bbMin[0], bbMin[1], bbMax[2], 1.f)), color);
	}

	// Render the mesh
	void render(glm::mat4 matVP)
	{
		glUseProgram(m_glTransformProgramID);

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * sizeof(DebugVertex), m_vVertices.data(), GL_STREAM_DRAW);

		glUniformMatrix4fv(m_glViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(matVP));
		
		// Draw mesh
		glBindVertexArray(this->m_glVAO);
		glDrawArrays(GL_LINES, 0, m_vVertices.size());
		glBindVertexArray(0);

		glUseProgram(0);

		_flushLines();
	}

private:
	struct DebugVertex {
		glm::vec3 pos;
		glm::vec3 col;

		DebugVertex(glm::vec3 p, glm::vec3 c)
			: pos(p)
			, col(c)
		{}
	};

	GLuint m_glVAO, m_glVBO;
	GLuint m_glTransformProgramID;
	GLint m_glViewProjectionMatrixLocation;
	std::vector<DebugVertex> m_vVertices;
	glm::mat4 m_mat4Transform;

	static DebugDrawer *s_instance;

	// CTOR
	DebugDrawer()
	{
		_createShader();
		_initGL();
	}

	void drawSpherePatch(const glm::vec3 &center, const glm::vec3 &up, const glm::vec3 &axis, float radius,
		float minTh, float maxTh, float minPs, float maxPs, const glm::vec3 &color, float stepDegrees = float(10.f), bool drawCenter = true)
	{
		glm::vec3 vA[74];
		glm::vec3 vB[74];
		glm::vec3 *pvA = vA, *pvB = vB, *pT;
		glm::vec3 npole = center + up * radius;
		glm::vec3 spole = center - up * radius;
		glm::vec3 arcStart;
		float step = glm::radians(stepDegrees);
		const glm::vec3& kv = up;
		const glm::vec3& iv = axis;
		glm::vec3 jv = glm::cross(kv, iv);
		bool drawN = false;
		bool drawS = false;
		if (minTh <= -glm::half_pi<float>())
		{
			minTh = -glm::half_pi<float>() + step;
			drawN = true;
		}
		if (maxTh >= glm::half_pi<float>())
		{
			maxTh = glm::half_pi<float>() - step;
			drawS = true;
		}
		if (minTh > maxTh)
		{
			minTh = -glm::half_pi<float>() + step;
			maxTh = glm::half_pi<float>() - step;
			drawN = drawS = true;
		}
		int n_hor = (int)((maxTh - minTh) / step) + 1;
		if (n_hor < 2) n_hor = 2;
		float step_h = (maxTh - minTh) / float(n_hor - 1);
		bool isClosed = false;
		if (minPs > maxPs)
		{
			minPs = -glm::pi<float>() + step;
			maxPs = glm::pi<float>();
			isClosed = true;
		}
		else if ((maxPs - minPs) >= glm::pi<float>() * float(2.f))
		{
			isClosed = true;
		}
		else
		{
			isClosed = false;
		}
		int n_vert = (int)((maxPs - minPs) / step) + 1;
		if (n_vert < 2) n_vert = 2;
		float step_v = (maxPs - minPs) / float(n_vert - 1);
		for (int i = 0; i < n_hor; i++)
		{
			float th = minTh + float(i) * step_h;
			float sth = radius * glm::sin(th);
			float cth = radius * glm::cos(th);
			for (int j = 0; j < n_vert; j++)
			{
				float psi = minPs + float(j) * step_v;
				float sps = glm::sin(psi);
				float cps = glm::cos(psi);
				pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
				if (i)
				{
					drawLine(pvA[j], pvB[j], color);
				}
				else if (drawS)
				{
					drawLine(spole, pvB[j], color);
				}
				if (j)
				{
					drawLine(pvB[j - 1], pvB[j], color);
				}
				else
				{
					arcStart = pvB[j];
				}
				if ((i == (n_hor - 1)) && drawN)
				{
					drawLine(npole, pvB[j], color);
				}

				if (drawCenter)
				{
					if (isClosed)
					{
						if (j == (n_vert - 1))
						{
							drawLine(arcStart, pvB[j], color);
						}
					}
					else
					{
						if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1))))
						{
							drawLine(center, pvB[j], color);
						}
					}
				}
			}
			pT = pvA; pvA = pvB; pvB = pT;
		}
	}

	void _flushLines()
	{
		m_vVertices.clear();
	}

	void _initGL()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->m_glVAO);
		glGenBuffers(1, &this->m_glVBO);

		glBindVertexArray(this->m_glVAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)0);
		// Vertex Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, col));

		glBindVertexArray(0);
	}
	
	bool _createShader()
	{
		m_glTransformProgramID = CompileGLShader(
			"Debugger",

			// vertex shader
			"#version 410\n"
			"layout(location = 0) in vec3 v3Position;\n"
			"layout(location = 1) in vec3 v3ColorIn;\n"
			"uniform mat4 matVP;\n"
			"out vec4 v4Color;\n"
			"void main()\n"
			"{\n"
			"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
			"	gl_Position = matVP * vec4(v3Position, 1.0);\n"
			"}\n",

			// fragment shader
			"#version 410\n"
			"in vec4 v4Color;\n"
			"out vec4 outputColor;\n"
			"void main()\n"
			"{\n"
			"   outputColor = v4Color;\n"
			"}\n"
		);

		m_glViewProjectionMatrixLocation = glGetUniformLocation(m_glTransformProgramID, "matVP");
		if (m_glViewProjectionMatrixLocation == -1)
		{
			printf("Unable to find view projection matrix uniform in debug shader\n");
			return false;
		}

		return m_glTransformProgramID != 0;
	}

// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	DebugDrawer(DebugDrawer const&) = delete;
	void operator=(DebugDrawer const&) = delete;
};
