#pragma once

#include <unordered_map>

#include <glm.hpp>

#include <GL/glew.h>

class Icosphere
{
public:	
	Icosphere(int recursionLevel);
	~Icosphere(void);

	void Icosphere::recalculate(int recursionLevel);

	std::vector<glm::vec3> getVertices(void);
	std::vector<GLushort> getIndices(void);

	GLuint getVAO();

private:	

    struct TriangleIndices
    {
        GLushort v1;
		GLushort v2;
		GLushort v3;

        TriangleIndices(GLushort v1, GLushort v2, GLushort v3)
        {
            this->v1 = v1;
            this->v2 = v2;
            this->v3 = v3;
        }
    };

	struct IcoVert
	{
		glm::vec3 v;
		glm::vec3 n;
		glm::vec4 c;
		glm::vec2 t;
	};

	GLushort addVertex(glm::vec3 p);
	GLushort getMiddlePoint(GLushort p1, GLushort p2, std::unordered_map<int64_t, GLushort> &midPointMap);

	std::vector<glm::vec3> vertices;
	std::vector<GLushort> indices;
	
	GLushort index;
};

