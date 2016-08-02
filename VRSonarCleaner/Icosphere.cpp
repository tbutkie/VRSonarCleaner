#include "Icosphere.h"
#include <list>

Icosphere::Icosphere(int recursionLevel)
	: index(0)
{
	this->recalculate(recursionLevel);
}


Icosphere::~Icosphere(void)
{
	this->vertices.clear();
	this->indices.clear();
	this->middlePointIndexCache.clear();
}

void Icosphere::recalculate(int recursionLevel)
{
	this->vertices.clear();
	this->indices.clear();
	this->middlePointIndexCache.clear();
	this->index = 0;

	// create 12 vertices of a icosahedron
	float t = (1.f + sqrt(5.f)) / 2.f;

	addVertex(Vector3(-1.f, t, 0.f));
	addVertex(Vector3(1.f, t, 0.f));
	addVertex(Vector3(-1.f, -t, 0.f));
	addVertex(Vector3(1.f, -t, 0.f));

	addVertex(Vector3(0.f, -1.f, t));
	addVertex(Vector3(0.f, 1.f, t));
	addVertex(Vector3(0.f, -1.f, -t));
	addVertex(Vector3(0.f, 1.f, -t));

	addVertex(Vector3(t, 0.f, -1.f));
	addVertex(Vector3(t, 0.f, 1.f));
	addVertex(Vector3(-t, 0.f, -1.f));
	addVertex(Vector3(-t, 0.f, 1.f));


	// create 20 triangles of the icosahedron
	std::list<TriangleIndices> faces;

	// 5 faces around point 0
	faces.push_back(TriangleIndices(0, 11, 5));
	faces.push_back(TriangleIndices(0, 5, 1));
	faces.push_back(TriangleIndices(0, 1, 7));
	faces.push_back(TriangleIndices(0, 7, 10));
	faces.push_back(TriangleIndices(0, 10, 11));

	// 5 adjacent faces 
	faces.push_back(TriangleIndices(1, 5, 9));
	faces.push_back(TriangleIndices(5, 11, 4));
	faces.push_back(TriangleIndices(11, 10, 2));
	faces.push_back(TriangleIndices(10, 7, 6));
	faces.push_back(TriangleIndices(7, 1, 8));

	// 5 faces around point 3
	faces.push_back(TriangleIndices(3, 9, 4));
	faces.push_back(TriangleIndices(3, 4, 2));
	faces.push_back(TriangleIndices(3, 2, 6));
	faces.push_back(TriangleIndices(3, 6, 8));
	faces.push_back(TriangleIndices(3, 8, 9));

	// 5 adjacent faces 
	faces.push_back(TriangleIndices(4, 9, 5));
	faces.push_back(TriangleIndices(2, 4, 11));
	faces.push_back(TriangleIndices(6, 2, 10));
	faces.push_back(TriangleIndices(8, 6, 7));
	faces.push_back(TriangleIndices(9, 8, 1));


	// refine triangles
	for (int i = 0; i < recursionLevel; i++)
	{
		std::list<TriangleIndices> faces2;
		for (auto &tri : faces)
		{
			// replace triangle by 4 triangles
			int a = getMiddlePoint(tri.v1, tri.v2);
			int b = getMiddlePoint(tri.v2, tri.v3);
			int c = getMiddlePoint(tri.v3, tri.v1);

			faces2.push_back(TriangleIndices(tri.v1, a, c));
			faces2.push_back(TriangleIndices(tri.v2, b, a));
			faces2.push_back(TriangleIndices(tri.v3, c, b));
			faces2.push_back(TriangleIndices(a, b, c));
		}
		faces = faces2;
	}

	// done, now add triangles to mesh
	for (auto &tri : faces)
	{
		this->indices.push_back(tri.v1);
		this->indices.push_back(tri.v2);
		this->indices.push_back(tri.v3);
	}
}

std::vector<Vector3> Icosphere::getVertices(void) { return vertices; }

std::vector<unsigned int> Icosphere::getIndices(void) { return indices; }

std::vector<float> Icosphere::getUnindexedFlatVertices(void)
{
	std::vector<float> flatVerts;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		flatVerts.push_back(vertices[i].x);
		flatVerts.push_back(vertices[i].y);
		flatVerts.push_back(vertices[i].z);
	}

	return flatVerts;
}

// add vertex to mesh, fix position to be on unit sphere, return index
int Icosphere::addVertex(Vector3 p)
{
	vertices.push_back(p.normalize());
	return index++;
}

// return index of point in the middle of p1 and p2
int Icosphere::getMiddlePoint(int p1, int p2)
{
    // first check if we have it already
    bool firstIsSmaller = p1 < p2;
    int64_t smallerIndex = firstIsSmaller ? p1 : p2;
    int64_t greaterIndex = firstIsSmaller ? p2 : p1;
    int64_t key = (smallerIndex << 32) + greaterIndex;

	// look to see if middle point already computed
    if (this->middlePointIndexCache.find(key) != this->middlePointIndexCache.end())
    	return this->middlePointIndexCache[key];    

    // not in cache, calculate it
	Vector3 point1 = this->vertices[p1];
	Vector3 point2 = this->vertices[p2];
	Vector3 middle = (point1 + point2) / 2.f;

    // add vertex makes sure point is on unit sphere
    int i = addVertex(middle); 

    // store it, return index
    this->middlePointIndexCache[key] = i;
    return i;
}
