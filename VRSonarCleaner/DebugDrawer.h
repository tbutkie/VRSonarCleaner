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

#include "preamble.glsl"

class DebugDrawer
{
public:
	// Singleton instance access
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
	
	void setTransform(const float *m)
	{
		m_mat4Transform = glm::make_mat4(m);
	}

	// Reset the debug drawer's drawing transform so that it draws in world space
	void setTransformDefault()
	{
		m_mat4Transform = glm::mat4();
	}

	// Draw a line using the debug drawer. To draw in a different coordinate space, use setTransform()
	void drawPoint(const glm::vec3 &pos, const glm::vec4 &col = glm::vec4(1.f))
	{
		glm::vec3 pos_xformed = glm::vec3(m_mat4Transform * glm::vec4(pos, 1.f));

		m_vPointsVertices.push_back(DebugVertex(pos_xformed, col));
	}

	// Draw a line using the debug drawer. To draw in a different coordinate space, use setTransform()
	void drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &col = glm::vec4(1.f))
	{
		glm::vec3 from_xformed = glm::vec3(m_mat4Transform * glm::vec4(from, 1.f));
		glm::vec3 to_xformed = glm::vec3(m_mat4Transform * glm::vec4(to, 1.f));

		m_vLinesVertices.push_back(DebugVertex(from_xformed, col));
		m_vLinesVertices.push_back(DebugVertex(to_xformed, col));
	}

	// Draw a line using the debug drawer. To draw in a different coordinate space, use setTransform()
	void drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec4 &colFrom, const glm::vec4 &colTo)
	{
		glm::vec3 from_xformed = glm::vec3(m_mat4Transform * glm::vec4(from, 1.f));
		glm::vec3 to_xformed = glm::vec3(m_mat4Transform * glm::vec4(to, 1.f));

		m_vLinesVertices.push_back(DebugVertex(from_xformed, colFrom));
		m_vLinesVertices.push_back(DebugVertex(to_xformed, colTo));
	}

	void drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec4 &color)
	{
		drawLine(v0, v1, color);
		drawLine(v1, v2, color);
		drawLine(v2, v0, color);
	}

	void drawSolidTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec4 &color)
	{
		glm::vec3 v0_xformed = glm::vec3(m_mat4Transform * glm::vec4(v0, 1.f));
		glm::vec3 v1_xformed = glm::vec3(m_mat4Transform * glm::vec4(v1, 1.f));
		glm::vec3 v2_xformed = glm::vec3(m_mat4Transform * glm::vec4(v2, 1.f));

		m_vTrianglesVertices.push_back(DebugVertex(v0_xformed, color));
		m_vTrianglesVertices.push_back(DebugVertex(v1_xformed, color));
		m_vTrianglesVertices.push_back(DebugVertex(v2_xformed, color));
	}

	void drawTransform(float orthoLen)
	{
		glm::vec3 start(0.f);
		drawLine(start, start + glm::vec3(glm::vec4(orthoLen, 0.f, 0.f, 0.f)), glm::vec4(0.7f, 0.f, 0.f, 0.25f));
		drawLine(start, start + glm::vec3(glm::vec4(0.f, orthoLen, 0.f, 0.f)), glm::vec4(0.f, 0.7f, 0.f, 0.25f));
		drawLine(start, start + glm::vec3(glm::vec4(0.f, 0.f, orthoLen, 0.f)), glm::vec4(0.f, 0.f, 0.7f, 0.25f));
	}

	void drawArc(float radiusX, float radiusY, float minAngle, float maxAngle, const glm::vec4 &color, bool drawSect, float stepDegrees = float(10.f))
	{
		glm::vec3 center(0.f);

		const glm::vec3 vx(1.f, 0.f, 0.f);
		glm::vec3 vy(0.f, 1.f, 0.f);
		float step = glm::radians(stepDegrees);
		int nSteps = (int)glm::abs((maxAngle - minAngle) / step);
		if (!nSteps) nSteps = 1;
		glm::vec3 prev = center + radiusX * vx * glm::cos(glm::radians(minAngle)) + radiusY * vy * glm::sin(glm::radians(minAngle));
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
		for (int i = 1; i <= nSteps; i++)
		{
			float angle = minAngle + (maxAngle - minAngle) * float(i) / float(nSteps);
			glm::vec3 next = center + radiusX * vx * glm::cos(glm::radians(angle)) + radiusY * vy * glm::sin(glm::radians(angle));
			drawLine(prev, next, color);
			prev = next;
		}
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
	}

	void drawSphere(float radius, float stepDegrees, glm::vec4 &color)
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

	virtual void drawBox(const glm::vec3 &bbMin, const glm::vec3 &bbMax, const glm::vec4 &color)
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

	// Render the mesh
	void render()
	{
		size_t nPointsVerts = m_vPointsVertices.size();
		size_t nLinesVerts = m_vLinesVertices.size();
		size_t nTrisVerts = m_vTrianglesVertices.size();

		std::vector<DebugVertex> buffer;

		std::copy(m_vPointsVertices.begin(), m_vPointsVertices.end(), std::back_inserter(buffer));
		std::copy(m_vLinesVertices.begin(), m_vLinesVertices.end(),	std::back_inserter(buffer));
		std::copy(m_vTrianglesVertices.begin(), m_vTrianglesVertices.end(), std::back_inserter(buffer));
		
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(DebugVertex), NULL, GL_STREAM_DRAW); // buffer orphaning
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(DebugVertex), buffer.data(), GL_STREAM_DRAW);
				
		// Draw mesh
		glBindVertexArray(this->m_glVAO);
		glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(nPointsVerts));
		glDrawArrays(GL_LINES, static_cast<GLint>(nPointsVerts), static_cast<GLsizei>(nLinesVerts));
		glDrawArrays(GL_TRIANGLES, static_cast<GLint>(nPointsVerts + nLinesVerts), static_cast<GLsizei>(nTrisVerts));
		glBindVertexArray(0);
	}

	void flushLines()
	{
		m_vPointsVertices.clear();
		m_vLinesVertices.clear();
		m_vTrianglesVertices.clear();
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
	std::vector<DebugVertex> m_vPointsVertices, m_vLinesVertices, m_vTrianglesVertices;
	glm::mat4 m_mat4Transform;

	// CTOR
	DebugDrawer()
	{
		_initGL();
	}

	void drawSpherePatch(const glm::vec3 &center, const glm::vec3 &up, const glm::vec3 &axis, float radius,
		float minTh, float maxTh, float minPs, float maxPs, const glm::vec4 &color, float stepDegrees = float(10.f), bool drawCenter = true)
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

	void _initGL()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->m_glVAO);
		glGenBuffers(1, &this->m_glVBO);

		glBindVertexArray(this->m_glVAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)0);
		// Vertex Colors
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, col));

		glBindVertexArray(0);
	}

// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	DebugDrawer(DebugDrawer const&) = delete;
	void operator=(DebugDrawer const&) = delete;
};
