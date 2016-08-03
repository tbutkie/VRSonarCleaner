#pragma once

#include "../shared/Vectors.h"
#include <unordered_map>

class Icosphere
{
public:	
	Icosphere(int recursionLevel);
	~Icosphere(void);

	void Icosphere::recalculate(int recursionLevel);

	std::vector<Vector3> getVertices(void);
	std::vector<unsigned int> getIndices(void);

	std::vector<Vector3> getUnindexedVertices(void);

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

	int addVertex(Vector3 p);
	int getMiddlePoint(int p1, int p2);

	std::vector<Vector3> vertices;
	std::vector<unsigned int> indices;
	
    int index;
    std::unordered_map<int64_t, int> middlePointIndexCache;
};

