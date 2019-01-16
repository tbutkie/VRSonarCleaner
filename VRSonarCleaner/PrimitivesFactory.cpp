#include "PrimitivesFactory.h"

namespace PrimitiveFactory {
	void generateIcosphere(int recursionLevel, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		std::vector<glm::vec3> vertices;
		int index = 0;

		auto addVertex = [&](glm::vec3 pt) { vertices.push_back(glm::normalize(pt)); return index++; };

		std::unordered_map<int64_t, unsigned short> middlePointIndexCache;

		// create 12 vertices of a icosahedron
		float t = (1.f + sqrt(5.f)) / 2.f;

		addVertex(glm::vec3(-1.f, t, 0.f));
		addVertex(glm::vec3(1.f, t, 0.f));
		addVertex(glm::vec3(-1.f, -t, 0.f));
		addVertex(glm::vec3(1.f, -t, 0.f));

		addVertex(glm::vec3(0.f, -1.f, t));
		addVertex(glm::vec3(0.f, 1.f, t));
		addVertex(glm::vec3(0.f, -1.f, -t));
		addVertex(glm::vec3(0.f, 1.f, -t));

		addVertex(glm::vec3(t, 0.f, -1.f));
		addVertex(glm::vec3(t, 0.f, 1.f));
		addVertex(glm::vec3(-t, 0.f, -1.f));
		addVertex(glm::vec3(-t, 0.f, 1.f));

		struct TriangleIndices
		{
			unsigned short v1, v2, v3;
		};

		// create 20 triangles of the icosahedron
		std::list<TriangleIndices> faces;

		// 5 faces around point 0
		faces.push_back({ 0, 11, 5 });
		faces.push_back({ 0, 5, 1 });
		faces.push_back({ 0, 1, 7 });
		faces.push_back({ 0, 7, 10 });
		faces.push_back({ 0, 10, 11 });

		// 5 adjacent faces 
		faces.push_back({ 1, 5, 9 });
		faces.push_back({ 5, 11, 4 });
		faces.push_back({ 11, 10, 2 });
		faces.push_back({ 10, 7, 6 });
		faces.push_back({ 7, 1, 8 });

		// 5 faces around point 3
		faces.push_back({ 3, 9, 4 });
		faces.push_back({ 3, 4, 2 });
		faces.push_back({ 3, 2, 6 });
		faces.push_back({ 3, 6, 8 });
		faces.push_back({ 3, 8, 9 });

		// 5 adjacent faces 
		faces.push_back({ 4, 9, 5 });
		faces.push_back({ 2, 4, 11 });
		faces.push_back({ 6, 2, 10 });
		faces.push_back({ 8, 6, 7 });
		faces.push_back({ 9, 8, 1 });


		auto getMiddlePoint = [&](unsigned short p1, unsigned short p2) {
			// first check if we have it already
			bool firstIsSmaller = p1 < p2;
			int64_t smallerIndex = firstIsSmaller ? p1 : p2;
			int64_t greaterIndex = firstIsSmaller ? p2 : p1;
			int64_t key = (smallerIndex << 32) + greaterIndex;

			// look to see if middle point already computed
			if (middlePointIndexCache.find(key) != middlePointIndexCache.end())
				return middlePointIndexCache[key];

			// not in cache, calculate it
			glm::vec3 point1 = vertices[p1];
			glm::vec3 point2 = vertices[p2];
			glm::vec3 middle = (point1 + point2) / 2.f;

			// add vertex makes sure point is on unit sphere
			unsigned short i = addVertex(middle);

			// store it, return index
			middlePointIndexCache[key] = i;
			return i;
		};



		// refine triangles
		for (int i = 0; i < recursionLevel; i++)
		{
			std::list<TriangleIndices> faces2;
			for (auto &tri : faces)
			{
				// replace triangle by 4 triangles
				unsigned short a = getMiddlePoint(tri.v1, tri.v2);
				unsigned short b = getMiddlePoint(tri.v2, tri.v3);
				unsigned short c = getMiddlePoint(tri.v3, tri.v1);

				faces2.push_back({ tri.v1, a, c });
				faces2.push_back({ tri.v2, b, a });
				faces2.push_back({ tri.v3, c, b });
				faces2.push_back({ a, b, c });
			}
			faces = faces2;
		}

		// done, now add triangles to mesh
		for (auto &tri : faces)
		{
			inds.push_back(tri.v1);
			inds.push_back(tri.v2);
			inds.push_back(tri.v3);
		}



		for (auto v : vertices)
		{
			PrimVert iv;
			iv.p = v;
			iv.n = v;
			iv.c = glm::vec4(1.f);
			iv.t = glm::vec2(0.5f);

			verts.push_back(iv);
		}
	}

	void generateDisc(int numSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		size_t baseInd = verts.size();

		glm::vec3 p(0.f, 0.f, 0.f);
		glm::vec3 n(0.f, 0.f, 1.f);
		glm::vec4 c(1.f);
		glm::vec2 t(0.5f, 0.5f);

		verts.push_back(PrimVert({ p, n, c, t }));

		// Front 
		for (float i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments)) * glm::two_pi<float>();

			p = glm::vec3(sin(angle), cos(angle), 0.f);
			t = (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f;

			verts.push_back(PrimVert({ p, n, c, t }));

			if (i > 0)
			{
				inds.push_back(0); // ctr pt of endcap
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);
			}
		}
		inds.push_back(0);
		inds.push_back(1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);

		//Back
		//verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
		//for (float i = 0; i < numSegments; ++i)
		//{
		//	float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));
		//
		//	if (i > 0)
		//	{
		//		inds.push_back(verts.size() - (i + 2));
		//		inds.push_back(verts.size() - 2);
		//		inds.push_back(verts.size() - 1);
		//	}
		//}
		//inds.push_back(0);
		//inds.push_back(verts.size() - 1);
		//inds.push_back(1);
	}

	void generateCylinder(int numSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		assert(numSegments >= 3);

		unsigned short baseInd = static_cast<unsigned short>(verts.size());

		// Base endcap
		verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
		for (int i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

			if (i > 0)
			{
				inds.push_back(baseInd);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
			}
		}
		inds.push_back(baseInd);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
		inds.push_back(baseInd + 1);

		// Distal endcap
		verts.push_back(PrimVert({ glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
		for (int i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

			if (i > 0)
			{
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - (i + 2)); // ctr pt of endcap
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);
			}
		}
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - (numSegments + 1));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - (numSegments));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);

		// Shaft
		for (int i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 0.f) }));

			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 1.f) }));

			if (i > 0)
			{
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 4);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 3);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);

				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 3);
				inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
			}
		}
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - numSegments * 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);

		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - numSegments * 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - numSegments * 2 + 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseInd) - 1);
	}

	void generateFacetedCone(int numSegments, std::vector<PrimVert>& verts, std::vector<unsigned short>& inds)
	{
		assert(numSegments >= 3);

		size_t baseInd = verts.size();

		// Base endcap
		verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
		for (int i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments)) * glm::two_pi<float>();
			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

			inds.push_back(static_cast<unsigned short>(0));
			inds.push_back(static_cast<unsigned short>(utils::getIndexWrapped(i + 1, numSegments, 1)));
			inds.push_back(static_cast<unsigned short>(utils::getIndexWrapped(i + 2, numSegments, 1)));
		}

		// Body
		baseInd = verts.size() - baseInd;
		for (int i = 0; i < numSegments; ++i)
		{
			float ratio = (float)i / (float)(numSegments);
			float nextRatio = (float)(i + 1) / (float)(numSegments);
			float angle = ratio * glm::two_pi<float>();
			float nextAngle = nextRatio * glm::two_pi<float>();

			glm::vec3 basePt(sin(angle), cos(angle), 0.f);
			glm::vec3 nextBasePt(sin(nextAngle), cos(nextAngle), 0.f);
			glm::vec3 tipPt(0.f, 0.f, 1.f);
			glm::vec3 n = glm::normalize(glm::cross(nextBasePt - tipPt, basePt - tipPt));

			verts.push_back(PrimVert({ tipPt, n, glm::vec4(1.f), glm::vec2((nextRatio - ratio) / 2.f, 1.f) }));
			verts.push_back(PrimVert({ basePt, n, glm::vec4(1.f), glm::vec2(ratio, 0.f) }));
			verts.push_back(PrimVert({ nextBasePt, n, glm::vec4(1.f), glm::vec2(nextRatio, 0.f) }));

			inds.push_back(static_cast<unsigned short>(baseInd + (i * 3) + 0));
			inds.push_back(static_cast<unsigned short>(baseInd + (i * 3) + 2));
			inds.push_back(static_cast<unsigned short>(baseInd + (i * 3) + 1));
		}
	}

	void generateSmoothCone(int numSegments, std::vector<PrimVert>& verts, std::vector<unsigned short>& inds)
	{
		assert(numSegments >= 3);

		size_t baseInd = verts.size();

		// Base endcap
		verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
		for (int i = 0; i < numSegments; ++i)
		{
			float angle = ((float)i / (float)(numSegments)) * glm::two_pi<float>();
			verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

			inds.push_back(static_cast<unsigned short>(0));
			inds.push_back(static_cast<unsigned short>(utils::getIndexWrapped(i + 1, numSegments, 1)));
			inds.push_back(static_cast<unsigned short>(utils::getIndexWrapped(i + 2, numSegments, 1)));
		}

		// Body
		baseInd = verts.size() - baseInd;
		// make the ring base first
		for (int i = 0; i < numSegments; ++i)
		{
			float ratio = (float)i / (float)(numSegments);
			float nextRatio = (float)(i + 1) / (float)(numSegments);
			float prevRatio = (float)utils::getIndexWrapped(i - 1, numSegments - 1) / (float)(numSegments);
			float angle = ratio * glm::two_pi<float>();
			float nextAngle = nextRatio * glm::two_pi<float>();
			float prevAngle = prevRatio * glm::two_pi<float>();

			glm::vec3 basePt(sin(angle), cos(angle), 0.f);
			glm::vec3 nextBasePt(sin(nextAngle), cos(nextAngle), 0.f);
			glm::vec3 prevBasePt(sin(prevAngle), cos(prevAngle), 0.f);
			glm::vec3 tipPt(0.f, 0.f, 1.f);
			glm::vec3 n1 = glm::normalize(glm::cross(nextBasePt - tipPt, basePt - tipPt));
			glm::vec3 n2 = glm::normalize(glm::cross(basePt - tipPt, prevBasePt - tipPt));
			glm::vec3 n = (n1 + n2) / 2.f;

			verts.push_back(PrimVert({ basePt, n, glm::vec4(1.f), glm::vec2(ratio, 0.f) }));
		}

		// now make tip points and mesh
		for (int i = 0; i < numSegments; ++i)
		{
			float ratio = (float)i / (float)(numSegments);
			float nextRatio = (float)(i + 1) / (float)(numSegments);
			float angle = ratio * glm::two_pi<float>();
			float nextAngle = nextRatio * glm::two_pi<float>();

			glm::vec3 basePt(sin(angle), cos(angle), 0.f);
			glm::vec3 nextBasePt(sin(nextAngle), cos(nextAngle), 0.f);
			glm::vec3 tipPt(0.f, 0.f, 1.f);
			glm::vec3 tipNorm = glm::normalize(glm::cross(nextBasePt - tipPt, basePt - tipPt));

			verts.push_back(PrimVert({ tipPt, tipNorm, glm::vec4(1.f), glm::vec2((nextRatio - ratio) / 2.f, 1.f) }));

			inds.push_back(static_cast<unsigned short>(baseInd + numSegments + i)); // tip
			inds.push_back(static_cast<unsigned short>(baseInd + utils::getIndexWrapped(i + 1, numSegments - 1)));
			inds.push_back(static_cast<unsigned short>(baseInd + utils::getIndexWrapped(i + 0, numSegments - 1)));
		}
	}


	void generateTorus(float coreRadius, float meridianRadius, int numCoreSegments, int numMeridianSegments, std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		int nVerts = numCoreSegments * numMeridianSegments;

		for (int i = 0; i < numCoreSegments; i++)
			for (int j = 0; j < numMeridianSegments; j++)
			{
				float u = i / (numCoreSegments - 1.f);
				float v = j / (numMeridianSegments - 1.f);
				float theta = u * 2.f * glm::pi<float>();
				float rho = v * 2.f * glm::pi<float>();
				float x = cos(theta) * (coreRadius + meridianRadius * cos(rho));
				float y = sin(theta) * (coreRadius + meridianRadius * cos(rho));
				float z = meridianRadius * sin(rho);
				float nx = cos(theta)*cos(rho);
				float ny = sin(theta)*cos(rho);
				float nz = sin(rho);
				float s = u;
				float t = v;

				PrimVert currentVert = { glm::vec3(x, y, z), glm::vec3(nx, ny, nz), glm::vec4(1.f), glm::vec2(s, t) };
				verts.push_back(currentVert);

				unsigned short uvInd = i * numMeridianSegments + j;
				unsigned short uvpInd = i * numMeridianSegments + (j + 1) % numMeridianSegments;
				unsigned short umvInd = (((i - 1) % numCoreSegments + numCoreSegments) % numCoreSegments) * numMeridianSegments + j; // true modulo (not C++ remainder operand %) for negative wraparound
				unsigned short umvpInd = (((i - 1) % numCoreSegments + numCoreSegments) % numCoreSegments) * numMeridianSegments + (j + 1) % numMeridianSegments;

				inds.push_back(uvInd);   // (u    , v)
				inds.push_back(uvpInd);  // (u    , v + 1)
				inds.push_back(umvInd);  // (u - 1, v)

				inds.push_back(umvInd);  // (u - 1, v)
				inds.push_back(uvpInd);  // (u    , v + 1)
				inds.push_back(umvpInd); // (u - 1, v + 1)
			}
	}

	void generatePlane(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		// Front face
		verts.push_back(PrimVert({ glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f),	glm::vec4(1.f), glm::vec2(0.f, 0.f) }));

		inds.push_back(0);
		inds.push_back(1);
		inds.push_back(2);

		inds.push_back(2);
		inds.push_back(3);
		inds.push_back(0);

		// Back face
		verts.push_back(PrimVert({ glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));

		inds.push_back(4);
		inds.push_back(5);
		inds.push_back(6);

		inds.push_back(6);
		inds.push_back(7);
		inds.push_back(4);
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	void generateFullscreenQuad(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		verts.push_back({ glm::vec3(-1.f, -1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(0.f, 0.f) });
		verts.push_back({ glm::vec3(1.f, -1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(1.f, 0.f) });
		verts.push_back({ glm::vec3(1.f, 1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(1.f, 1.f) });
		verts.push_back({ glm::vec3(-1.f, 1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(0.f, 1.f) });

		inds.push_back(0);
		inds.push_back(1);
		inds.push_back(2);

		inds.push_back(2);
		inds.push_back(3);
		inds.push_back(0);
	}

	void generateCube(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		size_t baseVert = verts.size();

		glm::vec3 bboxMin(-0.5f);
		glm::vec3 bboxMax(0.5f);

		// Bottom
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);

		// Top
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);

		// Left
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);

		// Right
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);

		// Front
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);

		// Back
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f) }));
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 4);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 3);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 2);
		inds.push_back(static_cast<unsigned short>(verts.size() - baseVert) - 1);
	}

	// Essentially a unit cube wireframe
	void generateBBox(std::vector<PrimVert> &verts, std::vector<unsigned short> &inds)
	{
		size_t baseVert = verts.size();

		glm::vec3 bboxMin(-0.5f);
		glm::vec3 bboxMax(0.5f);

		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
		verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));

		for (size_t i = 0; i < (12 * 2); ++i)
			inds.push_back(static_cast<unsigned short>(i));
	}
} 
