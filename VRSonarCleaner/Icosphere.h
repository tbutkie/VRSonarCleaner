#pragma once

#include <unordered_map>

#include <shared/glm/glm.hpp>

class Icosphere
{
public:	
	Icosphere(int recursionLevel);
	~Icosphere(void);

	void Icosphere::recalculate(int recursionLevel);

	std::vector<glm::vec3> getVertices(void);
	std::vector<unsigned int> getIndices(void);

	std::vector<glm::vec3> getUnindexedVertices(void);

private:	

    struct TriangleIndices
    {
        int v1;
        int v2;
        int v3;

        TriangleIndices(int v1, int v2, int v3)
        {
            this->v1 = v1;
            this->v2 = v2;
            this->v3 = v3;
        }
    };

	int addVertex(glm::vec3 p);
	int getMiddlePoint(int p1, int p2, std::unordered_map<int64_t, int> &midPointMap);

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	
    int index;
};

