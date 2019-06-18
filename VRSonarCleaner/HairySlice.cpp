#include "HairySlice.h"

using namespace std::chrono_literals;

HairySlice::HairySlice(CosmoVolume* cosmoVolume)
	: m_pCosmoVolume(cosmoVolume)
	, m_eSeedType(STREAMTUBES)
	, m_glVBO(0)
	, m_glEBO(0)
	, m_glVAO(0)
	, m_glHaloVBO(0)
	, m_glHaloVAO(0)
	, m_bShowHalos(true)
	, m_bShowStreamtubes(true)
	, m_bCuttingPlaneJitter(true)
	, m_bCuttingPlaneSet(false)
	, m_fCuttingPlaneWidth(0.5f)
	, m_fCuttingPlaneHeight(0.5f)
	, m_uiCuttingPlaneGridRes(30u)
	, m_fTubeRadius(0.005f)
	, m_uiNumTubeSegments(16u)
	, m_fRK4StepSize(1.f)
	, m_fRK4StopVelocity(0.f)
	, m_uiRK4MaxPropagation_OneWay(25u)
	, m_fHaloRadiusFactor(2.f)
	, m_vec4HaloColor(glm::vec4(0.f, 0.f, 0.f, 1.f))
	, m_vec4VelColorMin(glm::vec4(0.f, 0.f, 0.5f, 1.f))
	, m_vec4VelColorMax(glm::vec4(1.f, 1.f, 0.f, 1.f))
{
	m_RNG.seed(std::random_device()());
	m_Distribution = std::uniform_real_distribution<float>(-1.f, 1.f);

	Renderer::getInstance().addShader("streamline", { "resources/shaders/streamline.vert", "resources/shaders/streamline.frag" }, true);

	sampleVolume();
	buildStreamTubes();
}


HairySlice::~HairySlice()
{
	for (auto &buff : { m_glVBO, m_glHaloVBO, m_glEBO })
		if (buff)
			glDeleteBuffers(1, &buff);
	
	for (auto &vertarr : { m_glVAO, m_glHaloVAO })
		if (vertarr)
			glDeleteVertexArrays(1, &vertarr);
	
}

void HairySlice::update()
{

}

void HairySlice::draw()
{
	m_rs.modelToWorldTransform = m_rsHalo.modelToWorldTransform = m_pCosmoVolume->getTransformRawDomainToVolume();

	if (m_bShowStreamtubes)
	{
		Renderer::getInstance().addToDynamicRenderQueue(m_rs);

		if (m_bShowHalos)
			Renderer::getInstance().addToDynamicRenderQueue(m_rsHalo);

		if (m_bCuttingPlaneSet)
		{
			glm::vec3 x0y0 = m_pCosmoVolume->convertToWorldCoords(m_vec3PlacedFrameDomain_x0y0);
			glm::vec3 x0y1 = m_pCosmoVolume->convertToWorldCoords(m_vec3PlacedFrameDomain_x0y1);
			glm::vec3 x1y0 = m_pCosmoVolume->convertToWorldCoords(m_vec3PlacedFrameDomain_x1y0);
			glm::vec3 x1y1 = m_pCosmoVolume->convertToWorldCoords(m_vec3PlacedFrameDomain_x1y1);

			Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y0, x0y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
			Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y1, x1y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
			Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y1, x1y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
			Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y0, x0y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		}
	}
}

void HairySlice::set()
{	
	m_mat4PlacedFrameWorldPose = m_pCosmoVolume->getTransformVolume();
	m_fCuttingPlaneWidth = m_pCosmoVolume->getDimensions().x * 2.f;
	m_fCuttingPlaneHeight = m_pCosmoVolume->getDimensions().y * 2.f;
	m_bCuttingPlaneSet = true;
	m_bCuttingPlaneJitter = true;
	sampleCuttingPlane(true);
	buildStreamTubes();
}


void HairySlice::sampleCuttingPlane(bool reseed)
{
	m_vvvec3RawStreamlines.clear();

	if (reseed)
	{
		m_vvec3StreamlineSeedsDomain.clear();

		m_fCuttingPlaneWidth = 1.f;
		m_fCuttingPlaneHeight = 1.f;

		glm::mat4 probeMat = m_bCuttingPlaneSet ? m_mat4PlacedFrameWorldPose : m_pCosmoVolume->getTransformVolume();
		glm::vec3 probePos = probeMat[3];

		float maxJitterX = 0.25f * (m_fCuttingPlaneWidth / static_cast<float>(m_uiCuttingPlaneGridRes - 1));
		float maxJitterY = 0.25f * (m_fCuttingPlaneHeight / static_cast<float>(m_uiCuttingPlaneGridRes - 1));

		for (int i = 0; i < m_uiCuttingPlaneGridRes; ++i)
		{
			float ratioWidth = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)i / (m_uiCuttingPlaneGridRes - 1) - 0.5f;

			for (int j = 0; j < m_uiCuttingPlaneGridRes; ++j)
			{
				float ratioHeight = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)j / (m_uiCuttingPlaneGridRes - 1) - 0.5f;
				glm::vec3 pos = probePos + glm::vec3(probeMat[0]) * ratioWidth * m_fCuttingPlaneWidth - glm::vec3(probeMat[1]) * ratioHeight * m_fCuttingPlaneHeight;

				if (m_bCuttingPlaneJitter)
					pos += m_Distribution(m_RNG) * glm::vec3(probeMat[0]) * maxJitterX + m_Distribution(m_RNG) * glm::vec3(probeMat[1]) * maxJitterY;

				glm::dvec3 domainPos = m_pCosmoVolume->convertToRawDomainCoords(pos);

				// Calculate forward and backward RK4 streamlines
				std::vector<glm::vec3> fwd = m_pCosmoVolume->getStreamline(domainPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity);
				std::vector<glm::vec3> rev = m_pCosmoVolume->getStreamline(domainPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity, true);

				// Now glue them together into a single streamline without duplicating seed point
				std::reverse(rev.begin(), rev.end());
				rev.insert(rev.end(), fwd.begin() + 1, fwd.end());

				// Save streamline and seed
				m_vvvec3RawStreamlines.push_back(rev);
				m_vvec3StreamlineSeedsDomain.push_back(domainPos);
			}
		}

		m_vec3PlacedFrameDomain_x0y0 = m_pCosmoVolume->convertToRawDomainCoords(probePos + glm::vec3(probeMat[0]) * -0.5f * m_fCuttingPlaneWidth - glm::vec3(probeMat[1]) * -0.5f * m_fCuttingPlaneHeight);
		m_vec3PlacedFrameDomain_x0y1 = m_pCosmoVolume->convertToRawDomainCoords(probePos + glm::vec3(probeMat[0]) * -0.5f * m_fCuttingPlaneWidth - glm::vec3(probeMat[1]) * 0.5f * m_fCuttingPlaneHeight);
		m_vec3PlacedFrameDomain_x1y0 = m_pCosmoVolume->convertToRawDomainCoords(probePos + glm::vec3(probeMat[0]) * 0.5f * m_fCuttingPlaneWidth - glm::vec3(probeMat[1]) * -0.5f * m_fCuttingPlaneHeight);
		m_vec3PlacedFrameDomain_x1y1 = m_pCosmoVolume->convertToRawDomainCoords(probePos + glm::vec3(probeMat[0]) * 0.5f * m_fCuttingPlaneWidth - glm::vec3(probeMat[1]) * 0.5f * m_fCuttingPlaneHeight);
	}
	else
	{
		for (auto &seed : m_vvec3StreamlineSeedsDomain)
		{
			std::vector<glm::vec3> fwd = m_pCosmoVolume->getStreamline(seed, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity);
			std::vector<glm::vec3> rev = m_pCosmoVolume->getStreamline(seed, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity, true);
			std::reverse(rev.begin(), rev.end());
			rev.insert(rev.end(), fwd.begin() + 1, fwd.end());

			m_vvvec3RawStreamlines.push_back(rev);
		}
	}
}

void HairySlice::sampleVolume(unsigned int gridRes)
{
	m_vvvec3RawStreamlines.clear();
	m_vvec3StreamlineSeedsDomain.clear();

	for (int i = 0; i < gridRes; ++i)
		for (int j = 0; j < gridRes; ++j)
			for (int k = 0; k < gridRes; ++k)
			{
				glm::vec3 seedPos((1.f / (gridRes + 1)) + glm::vec3(i, j, k) * (1.f / (gridRes + 1)));
				//glm::vec3 seedPos(glm::vec3(-1.f + (2.f / (gridRes-1)) * glm::vec3(i, j, k)));
				std::vector<glm::vec3> fwd = m_pCosmoVolume->getStreamline(seedPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity);
				std::vector<glm::vec3> rev = m_pCosmoVolume->getStreamline(seedPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity, true);
				std::reverse(rev.begin(), rev.end());
				rev.insert(rev.end(), fwd.begin() + 1, fwd.end());

				m_vvvec3RawStreamlines.push_back(rev);

				m_vvec3StreamlineSeedsDomain.push_back(seedPos);
			}
}

void HairySlice::buildStreamTubes()
{
	// For each streamline segment
	// construct local coordinate frame matrix using orthogonal axes u, v, w; where the w axis is the segment (Point_[n+1] - Point_n) itself
	// the u & v axes are unit vectors scaled to the radius of the tube
	// except for the first and last, each circular 'rib' will be on the uv-plane of the averaged coordinate frames between the two connected segments

	// make unit circle for 'rib' that will be moved along streamline
	int numCircleVerts = m_uiNumTubeSegments + 1;
	std::vector<glm::vec3> circleVerts;
	for (int i = 0; i < numCircleVerts; ++i)
	{
		float angle = ((float)i / (float)(numCircleVerts - 1)) * glm::two_pi<float>();
		circleVerts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
	}

	struct PrimVert {
		glm::vec3 p; // point
		glm::vec3 n; // normal
		glm::vec4 c; // color
		glm::vec2 t; // texture coord
	};

	std::vector<PrimVert> verts, haloverts;
	std::vector<GLuint> inds;
	for (auto &sl : m_vvvec3RawStreamlines)
	{
		if (sl.size() < 2)
			continue;

		std::vector<glm::quat> ribOrientations;

		ribOrientations.push_back(getSegmentOrientationMatrixNormalized(sl[1] - sl[0]));

		for (size_t i = 0; i < sl.size() - 2; ++i)
		{
			glm::quat q1(getSegmentOrientationMatrixNormalized(sl[i + 1] - sl[i]));
			glm::quat q2(getSegmentOrientationMatrixNormalized(sl[i + 2] - sl[i + 1]));

			ribOrientations.push_back(glm::slerp(q1, q2, 0.5f));
		}

		ribOrientations.push_back(getSegmentOrientationMatrixNormalized(sl[sl.size() - 1] - sl[sl.size() - 2]));

		assert(ribOrientations.size() == sl.size());

		// Make the shaft of the streamtube
		for (size_t i = 0; i < ribOrientations.size(); ++i)
		{
			glm::mat4 xform(glm::toMat3(ribOrientations[i]));
			xform[3] = glm::vec4(sl[i], 1.f);

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(sl[i]));

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				PrimVert pvTube, pvHalo;
				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[j] * m_fTubeRadius, 1.f));
				pvTube.n = glm::normalize(pvTube.p - sl[i]);
				//pv.c = glm::vec4(vel, 0.f, 1.f - vel, 1.f);
				pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);
				pvTube.t = glm::vec2(j / (circleVerts.size() - 1), i);

				GLuint thisInd(verts.size());

				verts.push_back(pvTube);

				pvHalo = pvTube;

				pvHalo.p = glm::vec3(xform * glm::vec4(circleVerts[j] * m_fTubeRadius * m_fHaloRadiusFactor, 1.f));
				pvHalo.c = glm::vec4(1.f);
				haloverts.push_back(pvHalo);

				if (i > 0 && j > 0)
				{
					inds.push_back(thisInd);
					inds.push_back(thisInd - numCircleVerts);
					inds.push_back(thisInd - numCircleVerts - 1);

					inds.push_back(thisInd);
					inds.push_back(thisInd - numCircleVerts - 1);
					inds.push_back(thisInd - 1);
				}
			}
		}

		// Make the endcaps
		glm::quat frontCap = ribOrientations.front();
		glm::quat endCap = glm::rotate(ribOrientations.back(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));

		for (auto &q : { frontCap, endCap })
		{
			PrimVert pvTube, pvHalo;
			pvTube.n = glm::vec3(glm::rotate(q, glm::vec3(0.f, 0.f, -1.f)));
			pvTube.t = glm::vec2(0.5f, q == frontCap ? 0.f : 1.f * sl.size());

			// base vertex
			int baseVert = verts.size();

			// center vertex
			pvTube.p = q == frontCap ? sl.front() : sl.back();

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(pvTube.p));
			//pv.c = glm::vec4(vel, 0.f, 1.f - vel, 1.f);
			pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);

			verts.push_back(pvTube);

			pvHalo = pvTube;
			pvHalo.c = glm::vec4(1.f);
			haloverts.push_back(pvHalo);

			glm::mat4 xform(glm::toMat3(q));
			xform[3] = glm::vec4(pvTube.p, 1.f);

			// circle verts (no need for last and first vert to be same)
			for (int i = 0; i < circleVerts.size(); ++i)
			{
				GLuint thisVert = verts.size();

				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[i] * m_fTubeRadius, 1.f));
				verts.push_back(pvTube);

				pvHalo.p = glm::vec3(xform * glm::vec4(circleVerts[i] * m_fTubeRadius * m_fHaloRadiusFactor, 1.f));
				haloverts.push_back(pvHalo);

				if (i > 0)
				{
					inds.push_back(baseVert);
					inds.push_back(thisVert - 1);
					inds.push_back(thisVert);
				}
			}
		}
	}

	if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	if (!m_glHaloVBO)
	{
		glCreateBuffers(1, &m_glHaloVBO);
		glNamedBufferStorage(m_glHaloVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}


	if (!m_glVAO)
	{
		glGenVertexArrays(1, &m_glVAO);
		glBindVertexArray(this->m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
		glBindVertexArray(0);

		m_rs.VAO = m_glVAO;
		m_rs.glPrimitiveType = GL_TRIANGLES;
		m_rs.shaderName = "streamline";
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 32.f;
		m_rs.hasTransparency = false;
	}

	if (!m_glHaloVAO)
	{
		glGenVertexArrays(1, &m_glHaloVAO);
		glBindVertexArray(this->m_glHaloVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glHaloVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
		glBindVertexArray(0);

		m_rsHalo.VAO = m_glHaloVAO;
		m_rsHalo.glPrimitiveType = GL_TRIANGLES;
		m_rsHalo.shaderName = "flat";
		m_rsHalo.indexType = GL_UNSIGNED_INT;
		m_rsHalo.diffuseColor = m_vec4HaloColor;
		m_rsHalo.specularColor = glm::vec4(0.f);
		m_rsHalo.specularExponent = 0.f;
		m_rsHalo.hasTransparency = false;
		m_rsHalo.vertWindingOrder = GL_CW;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glHaloVBO, 0, haloverts.size() * sizeof(PrimVert), haloverts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();
	m_rsHalo.vertCount = inds.size();
}

glm::quat HairySlice::getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up)
{
	glm::vec3 w(glm::normalize(segmentDirection));
	glm::vec3 u(glm::normalize(glm::cross(up, w)));
	glm::vec3 v(glm::normalize(glm::cross(w, u)));
	return glm::toQuat(glm::mat3(u, v, w));
}