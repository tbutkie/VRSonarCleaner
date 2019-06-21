#include "HairySlice.h"

using namespace std::chrono_literals;

HairySlice::HairySlice(CosmoVolume* cosmoVolume)
	: m_pCosmoVolume(cosmoVolume)
	, m_glVBO(0)
	, m_glEBO(0)
	, m_glVAO(0)
	, m_glHaloVBO(0)
	, m_glHaloVAO(0)
	, m_bShowHalos(false)
	, m_bShowGeometry(true)
	, m_bOscillate(true)
	, m_bCuttingPlaneJitter(true)
	, m_bCuttingPlaneSet(false)
	, m_fCuttingPlaneWidth(0.3f)
	, m_fCuttingPlaneHeight(0.3f)
	, m_uiCuttingPlaneGridRes(20u)
	, m_fTubeRadius(0.0025f)
	, m_fOscAmpDeg(20.f)
	, m_fOscTime(2.5f)
	, m_uiNumTubeSegments(16u)
	, m_fRK4StepSize(0.5f)
	, m_fRK4StopVelocity(0.f)
	, m_uiRK4MaxPropagation_OneWay(20u)
	, m_fHaloRadiusFactor(2.f)
	, m_vec4HaloColor(glm::vec4(0.f, 0.f, 0.f, 1.f))
	, m_vec4VelColorMin(glm::vec4(0.f, 0.f, 0.5f, 1.f))
	, m_vec4VelColorMax(glm::vec4(1.f, 1.f, 0.f, 1.f))
	, m_vstrShaderNames({
		"streamline",
		"streamline_gradient_animated",
		"streamline_gradient_static",
		"streamline_ring_animated",
		"streamline_ring_static"
		})
	, m_iCurrentShader(4)
	, m_vstrGeomStyleNames({
		"STREAMLET_ANIMATED",
		"STREAMLET_STATIC",
		"STREAMTUBE_STATIC",
		"STREAMTUBE_ANIMATED",
		"STREAMTUBE_HALO" ,
		"STREAMCONE"
		})
	, m_iCurrentGeomStyle(5)
{
	m_RNG.seed(std::random_device()());
	m_Distribution = std::uniform_real_distribution<float>(-1.f, 1.f);

	Renderer::getInstance().addShader("streamline", { "resources/shaders/streamline.vert", "resources/shaders/streamline.frag" }, true);
	Renderer::getInstance().addShader("streamline_gradient_animated", { "resources/shaders/streamline.vert", "resources/shaders/streamline_gradient_animated.frag" }, true);
	Renderer::getInstance().addShader("streamline_gradient_static", { "resources/shaders/streamline.vert", "resources/shaders/streamline_gradient_static.frag" }, true);
	Renderer::getInstance().addShader("streamline_ring_animated", { "resources/shaders/streamline.vert", "resources/shaders/streamline_ring_animated.frag" }, true);
	Renderer::getInstance().addShader("streamline_ring_static", { "resources/shaders/streamline.vert", "resources/shaders/streamline_ring_static.frag" }, true);
	
	reseed();
	set();
}


HairySlice::~HairySlice()
{
	destroyGeometry();
}

void HairySlice::update()
{
	float oscTimeMS = m_fOscTime * 1000.f;

	float ratio = glm::mod(Renderer::getInstance().getElapsedMilliseconds(), oscTimeMS) / oscTimeMS;

	float amount = glm::sin(glm::two_pi<float>() * ratio);

	float rotNow = amount * (m_fOscAmpDeg / 2.f);

	//if (m_bOscillate)
	//{
	//	glm::mat3 trans = glm::toMat3(m_pCosmoVolume->getOriginalOrientation()) * glm::mat3(glm::rotate(glm::mat4(), glm::radians(rotNow), glm::vec3(0.f, 1.f, 0.f)));
	//	m_pCosmoVolume->setOrientation(glm::quat_cast(trans));
	//}
	//else
	//{
	//	m_pCosmoVolume->setOrientation(m_pCosmoVolume->getOriginalOrientation());
	//}
}

void HairySlice::draw()
{

	if (m_bShowGeometry)
	{
		m_rs.modelToWorldTransform = m_rsHalo.modelToWorldTransform = m_pCosmoVolume->getTransformRawDomainToVolume();

		Renderer::getInstance().addToDynamicRenderQueue(m_rs);

		if (m_bShowHalos || m_iCurrentGeomStyle == 4)
			Renderer::getInstance().addToDynamicRenderQueue(m_rsHalo);
	}

	if (m_bCuttingPlaneSet)
	{
		glm::vec3 x0y0 = glm::vec4(m_vec3PlacedFrame_x0y0, 1.f);
		glm::vec3 x0y1 = glm::vec4(m_vec3PlacedFrame_x0y1, 1.f);
		glm::vec3 x1y0 = glm::vec4(m_vec3PlacedFrame_x1y0, 1.f);
		glm::vec3 x1y1 = glm::vec4(m_vec3PlacedFrame_x1y1, 1.f);

		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y0, x0y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y1, x1y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y1, x1y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y0, x0y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
	}
}

void HairySlice::set()
{	
	m_mat4PlacedFrameWorldPose = glm::mat4();//m_pCosmoVolume->getTransformVolume();
	m_bCuttingPlaneSet = true;
	sampleCuttingPlane();
	rebuildGeometry();
}

void HairySlice::nextShader()
{
	if (++m_iCurrentShader == m_vstrShaderNames.size())
		m_iCurrentShader = 0;

	m_rs.shaderName = m_vstrShaderNames[m_iCurrentShader];
}

void HairySlice::prevShader()
{
	if (--m_iCurrentShader < 0)
		m_iCurrentShader = m_vstrShaderNames.size() - 1;

	m_rs.shaderName = m_vstrShaderNames[m_iCurrentShader];
}

std::string HairySlice::getShaderName()
{
	return m_vstrShaderNames[m_iCurrentShader];
}

void HairySlice::nextGeomStyle()
{
	if (++m_iCurrentGeomStyle == m_vstrGeomStyleNames.size())
		m_iCurrentGeomStyle = 0;
}

void HairySlice::prevGeomStyle()
{
	if (--m_iCurrentGeomStyle < 0)
		m_iCurrentGeomStyle = m_vstrGeomStyleNames.size() - 1;
}

std::string HairySlice::getGeomStyle()
{
	return m_vstrGeomStyleNames[m_iCurrentGeomStyle];
}

void HairySlice::reseed()
{
	m_vvec3StreamlineSeeds.clear();

	glm::vec3 xDir(1.f, 0.f, 0.f);
	glm::vec3 yDir(0.f, 1.f, 0.f);
	
	float maxJitterX = 0.25f * (m_fCuttingPlaneWidth / static_cast<float>(m_uiCuttingPlaneGridRes - 1));
	float maxJitterY = 0.25f * (m_fCuttingPlaneHeight / static_cast<float>(m_uiCuttingPlaneGridRes - 1));

	for (int i = 0; i < m_uiCuttingPlaneGridRes; ++i)
	{
		float ratioWidth = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)i / (m_uiCuttingPlaneGridRes - 1) - 0.5f;

		for (int j = 0; j < m_uiCuttingPlaneGridRes; ++j)
		{
			float ratioHeight = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)j / (m_uiCuttingPlaneGridRes - 1) - 0.5f;

			glm::vec3 pos = xDir * ratioWidth * m_fCuttingPlaneWidth - yDir * ratioHeight * m_fCuttingPlaneHeight;

			if (m_bCuttingPlaneJitter)
				pos += m_Distribution(m_RNG) * glm::vec3(1.f, 0.f, 0.f) * maxJitterX + m_Distribution(m_RNG) * glm::vec3(0.f, 1.f, 0.f) * maxJitterY;
			
			m_vvec3StreamlineSeeds.push_back(pos);
		}
	}

	m_vec3PlacedFrame_x0y0 = (xDir * m_fCuttingPlaneWidth * -0.5f - yDir * m_fCuttingPlaneHeight * -0.5f);
	m_vec3PlacedFrame_x0y1 = (xDir * m_fCuttingPlaneWidth * -0.5f - yDir * m_fCuttingPlaneHeight *  0.5f);
	m_vec3PlacedFrame_x1y0 = (xDir * m_fCuttingPlaneWidth *  0.5f - yDir * m_fCuttingPlaneHeight * -0.5f);
	m_vec3PlacedFrame_x1y1 = (xDir * m_fCuttingPlaneWidth *  0.5f - yDir * m_fCuttingPlaneHeight *  0.5f);
}

void HairySlice::sampleCuttingPlane()
{
	glm::vec3 dimratio = m_pCosmoVolume->getOriginalDimensions() / m_pCosmoVolume->getDimensions();

	m_vvvec3RawStreamlines.clear();

	for (auto &seed : m_vvec3StreamlineSeeds)
	{
		glm::dvec3 domainSeed = m_pCosmoVolume->convertToRawDomainCoords(m_mat4PlacedFrameWorldPose * glm::vec4(seed, 1.f));
		std::vector<glm::vec3> fwd = m_pCosmoVolume->getStreamline(domainSeed, dimratio.x * m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity);
		std::vector<glm::vec3> rev = m_pCosmoVolume->getStreamline(domainSeed, dimratio.x * m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity, true);
		std::reverse(rev.begin(), rev.end());
		rev.insert(rev.end(), fwd.begin() + 1, fwd.end());

		m_vvvec3RawStreamlines.push_back(rev);
	}	
}

void HairySlice::sampleVolume(unsigned int gridRes)
{
	m_vvvec3RawStreamlines.clear();
	m_vvec3StreamlineSeeds.clear();

	for (int i = 0; i < gridRes; ++i)
		for (int j = 0; j < gridRes; ++j)
			for (int k = 0; k < gridRes; ++k)
			{
				glm::vec3 seedPos((1.f / (gridRes + 1)) + glm::vec3(i, j, k) * (1.f / (gridRes + 1)));
				std::vector<glm::vec3> fwd = m_pCosmoVolume->getStreamline(seedPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity);
				std::vector<glm::vec3> rev = m_pCosmoVolume->getStreamline(seedPos, m_fRK4StepSize, m_uiRK4MaxPropagation_OneWay, m_fRK4StopVelocity, true);
				std::reverse(rev.begin(), rev.end());
				rev.insert(rev.end(), fwd.begin() + 1, fwd.end());

				m_vvvec3RawStreamlines.push_back(rev);

				m_vvec3StreamlineSeeds.push_back(seedPos);
			}
}

void HairySlice::rebuildGeometry()
{
	destroyGeometry();

	switch (m_iCurrentGeomStyle)
	{
	case 0:
	case 1:
		//buildStreamlets();
		break;
	case 2:
	case 3:
	case 4:
		buildStreamTubes();
		break;
	case 5:
		buildStreamCones();
		break;
	default:
		break;
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

	//if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glHaloVBO)
	{
		glCreateBuffers(1, &m_glHaloVBO);
		glNamedBufferStorage(m_glHaloVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}


	//if (!m_glVAO)
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
		m_rs.shaderName = m_vstrShaderNames[m_iCurrentShader];
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 32.f;
		m_rs.hasTransparency = false;
	}

	//if (!m_glHaloVAO)
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

void HairySlice::buildStreamCones(float coneEnlargementFactor)
{
	glm::vec3 dimratio =  m_pCosmoVolume->getOriginalDimensions() / m_pCosmoVolume->getDimensions();
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

	std::vector<PrimVert> verts;
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

		// Make the shaft of the streamcone
		for (size_t i = 0; i < ribOrientations.size(); ++i)
		{
			glm::mat4 xform(glm::toMat3(ribOrientations[i]));
			xform[3] = glm::vec4(sl[i], 1.f);

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(sl[i]));

			float ratioAlongLine = static_cast<float>(i) / (ribOrientations.size() - 1u);

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				PrimVert pvTube;
				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[j] * m_fTubeRadius * ratioAlongLine * coneEnlargementFactor * dimratio, 1.f));
				pvTube.n = i == 0 ? glm::normalize(glm::vec3(xform * glm::vec4(circleVerts[j], 1.f)) - sl[i]) : glm::normalize(pvTube.p - sl[i]);
				pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);
				pvTube.t = glm::vec2(j / (circleVerts.size() - 1), i);

				GLuint thisInd(verts.size());

				verts.push_back(pvTube);

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

		// Make the endcap
		glm::quat endCap = glm::rotate(ribOrientations.back(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));

		PrimVert pvTube;
		pvTube.n = glm::vec3(glm::rotate(endCap, glm::vec3(0.f, 0.f, -1.f)));
		pvTube.t = glm::vec2(0.5f, sl.size());

		// base vertex
		int baseVert = verts.size();

		// center vertex
		pvTube.p = sl.back();

		float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(pvTube.p));
		//pv.c = glm::vec4(vel, 0.f, 1.f - vel, 1.f);
		pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);

		verts.push_back(pvTube);
		
		glm::mat4 xform(glm::toMat3(endCap));
		xform[3] = glm::vec4(pvTube.p, 1.f);

		// circle verts (no need for last and first vert to be same)
		for (int i = 0; i < circleVerts.size(); ++i)
		{
			GLuint thisVert = verts.size();

			pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[i] * m_fTubeRadius * coneEnlargementFactor * dimratio, 1.f));
			verts.push_back(pvTube);
			
			if (i > 0)
			{
				inds.push_back(baseVert);
				inds.push_back(thisVert - 1);
				inds.push_back(thisVert);
			}
		}
	}

	//if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}


	//if (!m_glVAO)
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
		m_rs.shaderName = m_vstrShaderNames[m_iCurrentShader];
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 32.f;
		m_rs.hasTransparency = false;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();
}

void HairySlice::buildStreamlets()
{
	float coneEnlargementFactor = 5.f;

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

	std::vector<PrimVert> verts;
	std::vector<GLuint> inds;
	for (auto &sl : m_vvvec3RawStreamlines)
	{
		if (sl.size() < 2)
			continue;
			   
		for (size_t i = 0; i < sl.size() - 1; ++i)
		{
			glm::vec3 segmentMidpoint(sl[i] + (sl[i + 1] - sl[i]) * 0.5f);


		}
	}

	//if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}


	//if (!m_glVAO)
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
		m_rs.shaderName = m_vstrShaderNames[m_iCurrentShader];
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 32.f;
		m_rs.hasTransparency = false;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();

}

void HairySlice::destroyGeometry()
{
	for (auto &buff : { m_glVBO, m_glHaloVBO, m_glEBO })
		if (buff)
			glDeleteBuffers(1, &buff);

	for (auto &vertarr : { m_glVAO, m_glHaloVAO })
		if (vertarr)
			glDeleteVertexArrays(1, &vertarr);
}


glm::quat HairySlice::getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up)
{
	glm::vec3 w(glm::normalize(segmentDirection));
	glm::vec3 u(glm::normalize(glm::cross(up, w)));
	glm::vec3 v(glm::normalize(glm::cross(w, u)));
	return glm::toQuat(glm::mat3(u, v, w));
}