#pragma once
#include <vector>
#include <unordered_map>
#include <glm.hpp>
#include <gtc/constants.hpp>
#include "utilities.h"

struct PrimVert {
	glm::vec3 p; // point
	glm::vec3 n; // normal
	glm::vec4 c; // color
	glm::vec2 t; // texture coord
};

namespace PrimitiveFactory {
	void generateIcosphere(int recursionLevel, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateDisc(int numSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateTorus(float coreRadius, float meridianRadius, int numCoreSegments, int numMeridianSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateCylinder(int numSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateFacetedCone(int numSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateSmoothCone(int numSegments, std::vector<PrimVert>& verts, std::vector<unsigned short>& inds);
	void generatePlane(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateCube(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateBBox(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
	void generateFullscreenQuad(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds);
}

