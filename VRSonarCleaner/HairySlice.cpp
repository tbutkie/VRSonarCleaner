#include "HairySlice.h"
#include "utilities.h"

using namespace std::chrono_literals;

HairySlice::HairySlice(CosmoVolume* cosmoVolume)
	: m_pCosmoVolume(cosmoVolume)
	, m_fLastUpdate(Renderer::getInstance().getElapsedSeconds())
	, m_glVBO(0)
	, m_glEBO(0)
	, m_glVAO(0)
	, m_glHaloVBO(0)
	, m_glHaloVAO(0)
	, m_glParticleVBO(0)
	, m_glParticleEBO(0)
	, m_glParticleVAO(0)
	, m_bShowHalos(false)
	, m_bShowGeometry(true)
	, m_bShowSeeds(false)
	, m_bShowFrame(true)
	, m_bShowGrid(true)
	, m_bShowReticule(true)
	, m_bShowTarget(false)
	, m_bSpinReticule(true)
	, m_bOscillate(false)
	, m_bJitterSeeds(true)
	, m_vec2CuttingPlaneSize(0.3f, 0.3f)
	, m_uiCuttingPlaneGridRes(20u)
	, m_fTubeRadius(0.003125f)
	, m_fOscAmpDeg(20.f)
	, m_fOscTime(2.5f)
	, m_uiNumTubeSegments(16u)
	, m_fRK4StepSize(0.5f)
	, m_fRK4StopVelocity(0.f)
	, m_uiRK4MaxPropagation_OneWay(10u)
	, m_fHaloRadiusFactor(2.f)
	, m_fParticleTail(0.5f)
	, m_arrfParticleLives(NULL)
	, m_nParticleCount(0)
	, m_fParticleLifetime(1.f)
	, m_fParticleBirthTime(0.f)
	, m_fParticleDeathTime(0.5f)
	, m_vec4ParticleHeadColor(1.f)
	, m_vec4ParticleTailColor(0.3f, 0.3f, 0.3f, 0.f)
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
	, m_iCurrentShader(0)
	, m_vstrGeomStyleNames({
		"STREAKLET",
		"LINE",
		"TUBE",
		"CONE"
		})
	, m_iCurrentGeomStyle(0)
{
	m_RNG.seed(std::random_device()());
	m_Distribution = std::uniform_real_distribution<float>(-1.f, 1.f);

	Renderer::getInstance().addShader("streamline", { "resources/shaders/streamline.vert", "resources/shaders/streamline.frag" }, true);
	Renderer::getInstance().addShader("streamline_gradient_animated", { "resources/shaders/streamline.vert", "resources/shaders/streamline_gradient_animated.frag" }, true);
	Renderer::getInstance().addShader("streamline_gradient_static", { "resources/shaders/streamline.vert", "resources/shaders/streamline_gradient_static.frag" }, true);
	Renderer::getInstance().addShader("streamline_ring_animated", { "resources/shaders/streamline.vert", "resources/shaders/streamline_ring_animated.frag" }, true);
	Renderer::getInstance().addShader("streamline_ring_static", { "resources/shaders/streamline.vert", "resources/shaders/streamline_ring_static.frag" }, true);
	Renderer::getInstance().addShader("grid", { "resources/shaders/grid.vert", "resources/shaders/grid.frag" }, true);
	
	glLineWidth(2.f);

	buildReticule();
	reseed();
	set();
}


HairySlice::~HairySlice()
{
	destroyGeometry();
}

void HairySlice::update()
{
	float tick = Renderer::getInstance().getElapsedSeconds();
	float elapsedTime = tick - m_fLastUpdate;
	m_fLastUpdate = tick;


	float oscTimeMS = m_fOscTime * 1000.f;

	float ratio = glm::mod(Renderer::getInstance().getElapsedMilliseconds(), oscTimeMS) / oscTimeMS;

	float amount = glm::sin(glm::two_pi<float>() * ratio);

	float rotNow = amount * (m_fOscAmpDeg / 2.f);

	if (m_bOscillate)
	{
		m_qPlaneOrientation = glm::rotate(glm::mat4(), glm::radians(rotNow), glm::vec3(0.f, 1.f, 0.f));
	}
	else
	{
		m_qPlaneOrientation = glm::quat();
	}

	for (auto &shader : m_vstrShaderNames)
	{
		GLuint* shaderHandle = Renderer::getInstance().getShader(shader);
		if (shaderHandle)
		{
			glUseProgram(*shaderHandle);
			glUniform1f(glGetUniformLocation(*shaderHandle, "nStreamLineSegments"), (m_uiRK4MaxPropagation_OneWay * 2.f));
		}
	}

	// Deal with particles
	updateParticles(elapsedTime);
}

void HairySlice::draw()
{
	if (m_bShowGeometry)
	{
		m_rs.modelToWorldTransform = m_rsHalo.modelToWorldTransform = m_rsParticle.modelToWorldTransform = glm::mat4_cast(m_qPlaneOrientation);// m_pCosmoVolume->getTransformRawDomainToVolume();

		Renderer::getInstance().addToDynamicRenderQueue(m_iCurrentGeomStyle == 0 ? m_rsParticle : m_rs);

		if (m_bShowHalos || m_iCurrentGeomStyle == 4)
			Renderer::getInstance().addToDynamicRenderQueue(m_rsHalo);
	}

	// Reticule
	if (m_bShowReticule)
	{
		float spinRate = 5.f;
		float rotAngle = m_bSpinReticule ? 360.f * glm::mod(Renderer::getInstance().getElapsedSeconds(), spinRate) / spinRate : 0.f;
		m_rsReticule.modelToWorldTransform = glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), m_vec3Reticule + glm::vec3(0.f, 0.f, 1.f) * 0.0001f) * glm::rotate(glm::mat4(), glm::radians(rotAngle), glm::vec3(0.f, 0.f, 1.f)) * glm::scale(glm::mat4(), glm::vec3(0.05f));
		Renderer::getInstance().addToDynamicRenderQueue(m_rsReticule);

		//Renderer::getInstance().drawPrimitive("icosphere", glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), m_vec3Reticule) * glm::scale(glm::mat4(), glm::vec3(0.001f)), glm::vec4(1.f, 0.f, 0.f, 1.f));
	}

	// Target vector
	if (m_bShowTarget)
	{
		float extendRate = 2.f;
		float maxExtend = 0.05f;
		float minExtend = 0.005f;
		float extend = glm::mix(minExtend, maxExtend, glm::mod(Renderer::getInstance().getElapsedSeconds(), extendRate) / extendRate);
		Renderer::getInstance().drawPointerLit(glm::mat3_cast(m_qPlaneOrientation) * m_vec3Reticule, glm::mat3_cast(m_qPlaneOrientation) * (m_vec3Reticule + glm::normalize(m_vec3FlowAtReticule) * extend), 0.0025f, glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f));
	}

	if (m_bShowSeeds)
		for (auto &seed : m_vvec3StreamlineSeeds)
			Renderer::getInstance().drawPrimitive("icosphere", glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), seed) * glm::scale(glm::mat4(), glm::vec3(0.001f)), glm::vec4(1.f, 0.f, 0.f, 1.f));

	if (m_bShowFrame)
	{
		glm::vec3 x0y0 = glm::mat4_cast(m_qPlaneOrientation) * glm::vec4(m_vec3PlacedFrame_x0y0, 1.f);
		glm::vec3 x0y1 = glm::mat4_cast(m_qPlaneOrientation) * glm::vec4(m_vec3PlacedFrame_x0y1, 1.f);
		glm::vec3 x1y0 = glm::mat4_cast(m_qPlaneOrientation) * glm::vec4(m_vec3PlacedFrame_x1y0, 1.f);
		glm::vec3 x1y1 = glm::mat4_cast(m_qPlaneOrientation) * glm::vec4(m_vec3PlacedFrame_x1y1, 1.f);

		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y0, x0y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y1, x1y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y1, x1y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y0, x0y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
	}

	// Semitransparent grid
	if (m_bShowGrid)
	{
		Renderer::RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "grid";
		rs.modelToWorldTransform = glm::mat4_cast(m_qPlaneOrientation) * glm::scale(glm::mat4(), glm::vec3(m_vec2CuttingPlaneSize, 1.f));
		rs.VAO = Renderer::getInstance().getPrimitiveVAO();		
		rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("quaddouble");
		rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("quaddouble");
		rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("quaddouble");
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.hasTransparency = true;

		m_rsReticule.transparencySortPosition = glm::vec4(0.f, 0.f, 50.f, 1.f);
		rs.transparencySortPosition = glm::vec4(0.f, 0.f, -100.f, 1.f);
		//m_rs.transparencySortPosition = glm::vec4(0.f, 0.f, 100.f, 1.f);

		Renderer::getInstance().addToDynamicRenderQueue(rs);
	}

	// Particles
	//drawParticleHeads(0.001f);
}

void HairySlice::set()
{	
	m_mat4PlacedFrameWorldPose = glm::mat4();
	sampleCuttingPlane();
	rebuildGeometry();
	m_vec3Reticule = randomSeedOnPlane();
	m_vec3FlowAtReticule = m_pCosmoVolume->getFlowWorldCoords(m_mat4PlacedFrameWorldPose * glm::vec4(m_vec3Reticule, 1.f));
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

bool HairySlice::setShader(std::string shaderName)
{
	auto it = std::find(m_vstrShaderNames.begin(), m_vstrShaderNames.end(), shaderName);

	if (it != m_vstrShaderNames.end())
	{
		m_iCurrentShader = it - m_vstrShaderNames.begin();
		m_rs.shaderName = *it;
		return true;
	}

	return false;
}

void HairySlice::nextGeomStyle()
{
	if (++m_iCurrentGeomStyle == m_vstrGeomStyleNames.size())
		m_iCurrentGeomStyle = 0;
	
	rebuildGeometry();
}

void HairySlice::prevGeomStyle()
{
	if (--m_iCurrentGeomStyle < 0)
		m_iCurrentGeomStyle = m_vstrGeomStyleNames.size() - 1;

	rebuildGeometry();
}

std::string HairySlice::getGeomStyle()
{
	return m_vstrGeomStyleNames[m_iCurrentGeomStyle];
}

bool HairySlice::setGeomStyle(std::string geomType)
{
	auto it = std::find(m_vstrGeomStyleNames.begin(), m_vstrGeomStyleNames.end(), geomType);

	if (it != m_vstrGeomStyleNames.end())
	{
		m_iCurrentGeomStyle = it - m_vstrGeomStyleNames.begin();
		rebuildGeometry();
		return true;
	}

	return false;
}

void HairySlice::reseed()
{
	m_vvec3StreamlineSeeds.clear();

	glm::vec3 xDir(1.f, 0.f, 0.f);
	glm::vec3 yDir(0.f, 1.f, 0.f);

	float xCellSize = m_vec2CuttingPlaneSize.x / static_cast<float>(m_uiCuttingPlaneGridRes);
	float yCellSize = m_vec2CuttingPlaneSize.y / static_cast<float>(m_uiCuttingPlaneGridRes);
	
	float maxJitterX = 0.25f * xCellSize;
	float maxJitterY = 0.25f * yCellSize;

	for (int i = 0; i < m_uiCuttingPlaneGridRes; ++i)
	{
		float ratioWidth = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)i / (m_uiCuttingPlaneGridRes - 1) - 0.5f;

		for (int j = 0; j < m_uiCuttingPlaneGridRes; ++j)
		{
			float ratioHeight = m_uiCuttingPlaneGridRes == 1 ? 0.f : (float)j / (m_uiCuttingPlaneGridRes - 1) - 0.5f;

			glm::vec3 pos = xDir * ratioWidth * (m_vec2CuttingPlaneSize.x - xCellSize) - yDir * ratioHeight * (m_vec2CuttingPlaneSize.y - yCellSize);

			if (m_bJitterSeeds)
				pos += m_Distribution(m_RNG) * xDir * maxJitterX + m_Distribution(m_RNG) * yDir * maxJitterY;
			
			m_vvec3StreamlineSeeds.push_back(pos);
		}
	}

	m_vec3PlacedFrame_x0y0 = (xDir * m_vec2CuttingPlaneSize.x * -0.5f - yDir * m_vec2CuttingPlaneSize.y * -0.5f);
	m_vec3PlacedFrame_x0y1 = (xDir * m_vec2CuttingPlaneSize.x * -0.5f - yDir * m_vec2CuttingPlaneSize.y *  0.5f);
	m_vec3PlacedFrame_x1y0 = (xDir * m_vec2CuttingPlaneSize.x *  0.5f - yDir * m_vec2CuttingPlaneSize.y * -0.5f);
	m_vec3PlacedFrame_x1y1 = (xDir * m_vec2CuttingPlaneSize.x *  0.5f - yDir * m_vec2CuttingPlaneSize.y *  0.5f);
}

glm::vec3 HairySlice::randomSeedOnPlane()
{
	glm::vec2 minExtents = -m_vec2CuttingPlaneSize * (1.f / 3.f);
	glm::vec2 maxExtents = m_vec2CuttingPlaneSize * (1.f / 3.f);

	glm::vec3 ret;

	do {
		ret = *(utils::select_randomly(m_vvec3StreamlineSeeds.begin(), m_vvec3StreamlineSeeds.end()));
	} while (ret.x < minExtents.x || ret.x > maxExtents.x || ret.y < minExtents.y || ret.y > maxExtents.y);

	return ret;
}

glm::vec3 HairySlice::randomPointOnPlane()
{
	glm::vec3 xDir(1.f, 0.f, 0.f);
	glm::vec3 yDir(0.f, 1.f, 0.f);

	glm::vec2 maxJitter = m_vec2CuttingPlaneSize * (1.f / 3.f);

	return m_Distribution(m_RNG) * xDir * maxJitter.x + m_Distribution(m_RNG) * yDir * maxJitter.y;
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

		for (auto &vert : rev)
			vert = m_pCosmoVolume->getTransformRawDomainToVolume() * glm::vec4(vert, 1.f);

		m_vvvec3RawStreamlines.push_back(rev);
	}
	
	// reset particle seeds
	initParticles();
}

void HairySlice::rebuildGeometry()
{
	destroyGeometry();

	switch (m_iCurrentGeomStyle)
	{
	case 0:
		//buildStreamTubes(m_fTubeRadius * 0.2f);		
		break;
	case 1:
		buildStreamLines();
		break;
	case 2:
		buildStreamTubes(m_fTubeRadius * 0.75f);
		break;
	case 3:
		buildStreamCones(m_fTubeRadius);
		break;
	default:
		break;
	}
}

void HairySlice::buildStreamLines()
{
	struct PrimVert {
		glm::vec3 p; // point
		glm::vec4 c; // color
	};

	std::vector<PrimVert> verts;
	std::vector<GLuint> inds;
	int currentInd = 0;
	for (auto &sl : m_vvvec3RawStreamlines)
	{
		size_t slSize = sl.size();

		if (slSize < 2)
			continue;

		currentInd = verts.size();

		for (int i = 0; i < slSize - 1; ++i)
		{
			float ratio = i / static_cast<float>(slSize - 1);
			verts.push_back({ sl[i] , glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, ratio) });
			inds.push_back(currentInd++);
			inds.push_back(currentInd);
		}

		verts.push_back({ sl[slSize - 1] , m_vec4ParticleHeadColor });
	}	   

	//if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, verts.size() * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, inds.size() * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}


	//if (!m_glVAO)
	{
		glGenVertexArrays(1, &m_glVAO);
		glBindVertexArray(m_glVAO);
			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);

			// Set the vertex attribute pointers
			glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
			glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glBindVertexArray(0);

		m_rs.VAO = m_glVAO;
		m_rs.glPrimitiveType = GL_LINES;
		m_rs.shaderName = "flat";
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 0.f;
		m_rs.hasTransparency = false;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();
}

void HairySlice::buildStreamTubes(float radius)
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

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(sl[i])));

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				PrimVert pvTube, pvHalo;
				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius, 1.f));
				pvTube.n = glm::normalize(pvTube.p - sl[i]);
				//pv.c = glm::vec4(vel, 0.f, 1.f - vel, 1.f);
				pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);
				pvTube.t = glm::vec2(j / (circleVerts.size() - 1), i);

				GLuint thisInd(verts.size());

				verts.push_back(pvTube);

				pvHalo = pvTube;

				pvHalo.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius * m_fHaloRadiusFactor, 1.f));
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

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(pvTube.p)));
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

				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius, 1.f));
				verts.push_back(pvTube);

				pvHalo.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius * m_fHaloRadiusFactor, 1.f));
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
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glHaloVBO)
	{
		glCreateBuffers(1, &m_glHaloVBO);
		glNamedBufferStorage(m_glHaloVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
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
		m_rs.specularColor = glm::vec4(0.7f);
		m_rs.specularExponent = 12.f;
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

void HairySlice::buildStreamCones(float radius)
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

			float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(sl[i])));

			//float ratioAlongLine = static_cast<float>(i) / (ribOrientations.size() - 1u);
			float ratioAlongLine = 1.f - static_cast<float>(i) / (ribOrientations.size() - 1u);

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				PrimVert pvTube;
				//pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius * ratioAlongLine * dimratio, 1.f));
				pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius * ratioAlongLine, 1.f)); //keeps constant world-space width
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
		//glm::quat endCap = glm::rotate(ribOrientations.back(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		glm::quat endCap = ribOrientations.front();

		PrimVert pvTube;
		pvTube.n = glm::vec3(glm::rotate(endCap, glm::vec3(0.f, 0.f, -1.f)));
		pvTube.t = glm::vec2(0.5f, sl.size());

		// base vertex
		int baseVert = verts.size();

		// center vertex
		//pvTube.p = sl.back();
		pvTube.p = sl.front();

		float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(pvTube.p)));
		//pv.c = glm::vec4(vel, 0.f, 1.f - vel, 1.f);
		pvTube.c = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);

		verts.push_back(pvTube);
		
		glm::mat4 xform(glm::toMat3(endCap));
		xform[3] = glm::vec4(pvTube.p, 1.f);

		// circle verts (no need for last and first vert to be same)
		for (int i = 0; i < circleVerts.size(); ++i)
		{
			GLuint thisVert = verts.size();

			//pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius * dimratio, 1.f));
			pvTube.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius, 1.f));
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
		glNamedBufferStorage(m_glVBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	//if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, m_uiCuttingPlaneGridRes * m_uiCuttingPlaneGridRes * (100 * 6 + 2 * 3) * m_uiNumTubeSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
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
		m_rs.specularColor = glm::vec4(0.7f);
		m_rs.specularExponent = 12.f;
		m_rs.hasTransparency = false;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();
}

void HairySlice::buildReticule()
{
	struct PrimVert {
		glm::vec3 p; // point
		glm::vec3 n; // normal
		glm::vec4 c; // color
		glm::vec2 t; // texture coord
	};

	std::vector<PrimVert> verts;
	std::vector<GLuint> inds;

	PrimVert tV; // temp Vertex

// top triangle
	tV.p = glm::vec3(0.25f, 1.f, 0.f);
	tV.c = glm::vec4(1.f);
	tV.n = glm::vec3(0.f, 0.f, 1.f);
	tV.t = glm::vec2(0.f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(-0.25f, 1.f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(0.f, 0.1f, 0.f);
	verts.push_back(tV);

	// left triangle
	tV.p = glm::vec3(-1.f, 0.25f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(-1.f, -0.25f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(-0.1f, 0.f, 0.f);
	verts.push_back(tV);

	// bottom triangle
	tV.p = glm::vec3(-0.25f, -1.f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(0.25f, -1.f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(0.f, -0.1f, 0.f);
	verts.push_back(tV);

	// right triangle
	tV.p = glm::vec3(1.f, -0.25f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(1.f, 0.25f, 0.f);
	verts.push_back(tV);

	tV.p = glm::vec3(0.1f, 0.f, 0.f);
	verts.push_back(tV);


	int nFaces = 4;
	GLsizei offset = 0;
	for (int i = 0; i < nFaces; ++i)
	{
		inds.push_back(offset);
		inds.push_back(offset + 1);
		inds.push_back(offset + 2);

		offset += 3;
	}

	//if (!m_glReticuleVBO)
	{
		glCreateBuffers(1, &m_glReticuleVBO);
		glNamedBufferStorage(m_glReticuleVBO, verts.size() * sizeof(PrimVert), verts.data(), 0);
	}

	//if (!m_glReticuleEBO)
	{
		glCreateBuffers(1, &m_glReticuleEBO);
		glNamedBufferStorage(m_glReticuleEBO, inds.size() * sizeof(GLuint), inds.data(), 0);
	}


	//if (!m_glReticuleVAO)
	{
		glGenVertexArrays(1, &m_glReticuleVAO);
		glBindVertexArray(this->m_glReticuleVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glReticuleVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glReticuleEBO);

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

		m_rsReticule.VAO = m_glReticuleVAO;
		m_rsReticule.glPrimitiveType = GL_TRIANGLES;
		m_rsReticule.shaderName = "flat";
		m_rsReticule.indexType = GL_UNSIGNED_INT;
		m_rsReticule.diffuseColor = glm::vec4(1.f, 0.f, 0.f, 0.25f);
		m_rsReticule.specularColor = glm::vec4(0.f);
		m_rsReticule.specularExponent = 32.f;
		m_rsReticule.hasTransparency = true;
		m_rsReticule.vertCount = inds.size();
	}
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

void HairySlice::initParticles()
{
	if (m_arrfParticleLives)
	{
		delete m_arrfParticleLives;
		m_arrfParticleLives = NULL;
	}

	m_nParticleCount = m_vvvec3RawStreamlines.size();
	size_t bufSize = sizeof(float) * m_nParticleCount;

	m_arrfParticleLives = (float*)malloc(bufSize);

	m_qpBirthingParticles = std::deque<std::pair<int, float>>();
	m_qpDyingParticles = std::deque<std::pair<int, float>>();
	m_qiDeadParticles = std::queue<int>();

	auto dist = std::uniform_real_distribution<float>(0.f, m_fParticleLifetime + m_fParticleDeathTime);

	for (int i = 0; i < m_nParticleCount; ++i)
	{
		if (true)
		{
			m_arrfParticleLives[i] = dist(m_RNG);
		}
		else
		{
			m_arrfParticleLives[i] = -1.f;
			m_qiDeadParticles.push(i);
		}
	}



	struct PrimVert {
		glm::vec3 p; // point
		glm::vec4 c; // color
	};

	if (!m_glParticleVBO)
	{
		glCreateBuffers(1, &m_glParticleVBO);
	}

	if (!m_glParticleEBO)
	{
		glCreateBuffers(1, &m_glParticleEBO);
	}

	glNamedBufferStorage(m_glParticleVBO, 50000 * (m_uiRK4MaxPropagation_OneWay * 2 + 1) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(m_glParticleEBO, 50000 * (m_uiRK4MaxPropagation_OneWay * 2 + 1) * 2 * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);

	if (!m_glParticleVAO)
	{
		glGenVertexArrays(1, &m_glParticleVAO);
		glBindVertexArray(m_glParticleVAO);
			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, m_glParticleVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glParticleEBO);

			// Set the vertex attribute pointers
			glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
			glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glBindVertexArray(0);

		m_rsParticle.VAO = m_glParticleVAO;
		m_rsParticle.glPrimitiveType = GL_LINES;
		m_rsParticle.shaderName = "flat";
		m_rsParticle.indexType = GL_UNSIGNED_INT;
		m_rsParticle.diffuseColor = glm::vec4(1.f);
		m_rsParticle.hasTransparency = false;
	}
}

void HairySlice::updateParticles(float elapsedTime)
{
	if (elapsedTime > 1.f / 60.f)
		elapsedTime = 1.f / 60.f;

	// Take care of dying particles which are now dead
	{
		for (auto &p : m_qpDyingParticles)
			p.second -= elapsedTime;

		while (!m_qpDyingParticles.empty() && m_qpDyingParticles.front().second < 0.f)
		{
			m_qiDeadParticles.push(m_qpDyingParticles.front().first);
			m_qpDyingParticles.pop_front();
		}
	}

	// Take care of particles being birthed
	{
		for (auto &p : m_qpBirthingParticles)
			p.second -= elapsedTime;

		while (!m_qpBirthingParticles.empty() && m_qpBirthingParticles.front().second < 0.f)
		{
			m_arrfParticleLives[m_qpBirthingParticles.front().first] = m_fParticleLifetime;
			m_qpBirthingParticles.pop_front();
		}
	}

	// Tick all living particles
	for (int i = 0; i < m_nParticleCount; ++i)
	{
		float* p = &m_arrfParticleLives[i];

		if (*p < 0.f)
			continue;

		*p -= elapsedTime;

		if (*p < 0.f)
		{
			*p = -1.f;
			m_qpDyingParticles.push_back(std::make_pair(i, m_fParticleTail * m_fParticleLifetime));
		}
	}

	// Take care of dead particles
	while (!m_qiDeadParticles.empty())
	{
		m_qpBirthingParticles.push_back(std::make_pair(m_qiDeadParticles.front(), m_fParticleBirthTime));
		m_qiDeadParticles.pop();
	}

	// Tails
	buildParticleTails();
}

void HairySlice::drawParticleHeads(float radius)
{
	for (int i = 0; i < m_nParticleCount; ++i)
	{
		if (m_arrfParticleLives[i] < 0.f)
			continue;

		float ratio = glm::clamp(1.f - m_arrfParticleLives[i] / m_fParticleLifetime, 0.f, 1.f);

		auto sl = m_vvvec3RawStreamlines[i];

		float indexPoint = ratio * (sl.size() - 1);
		int segmentBeginIndex = floor(indexPoint);

		float leftover = indexPoint - static_cast<float>(segmentBeginIndex);

		glm::vec3 segmentDir = sl[segmentBeginIndex + 1] - sl[segmentBeginIndex];

		glm::vec3 pos = sl[segmentBeginIndex] + segmentDir * leftover;

		float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(pos)));

		Renderer::getInstance().drawPrimitive("icosphere", glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), pos) * glm::scale(glm::mat4(), glm::vec3(radius)), glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel));
	}

	for (auto &p : m_qpBirthingParticles)
	{
		glm::vec3 pos = m_vvvec3RawStreamlines[p.first].front();
		float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(pos)));
		glm::vec4 color = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);
		float ratio = glm::clamp(1.f - p.second / m_fParticleBirthTime, 0.f, 1.f);
		Renderer::getInstance().drawPrimitive("icosphere", glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), pos) * glm::scale(glm::mat4(), glm::vec3(radius)), glm::vec4(glm::vec3(color), ratio));
	}

	for (auto &p : m_qpDyingParticles)
	{
		glm::vec3 pos = m_vvvec3RawStreamlines[p.first].back();
		float vel = sqrtf(m_pCosmoVolume->getRelativeVelocity(m_pCosmoVolume->convertToRawDomainCoords(pos)));
		glm::vec4 color = glm::mix(m_vec4VelColorMin, m_vec4VelColorMax, vel);
		float ratio = p.second / m_fParticleDeathTime;
		Renderer::getInstance().drawPrimitive("icosphere", glm::mat4_cast(m_qPlaneOrientation) * glm::translate(glm::mat4(), pos) * glm::scale(glm::mat4(), glm::vec3(radius) * ratio), color);
	}
}

void HairySlice::buildParticleTails()
{
	struct PrimVert {
		glm::vec3 pos;
		glm::vec4 color;
	};

	std::vector<PrimVert> verts;
	std::vector<GLuint> inds;

	for (int i = 0; i < m_nParticleCount; ++i)
	{
		if (m_arrfParticleLives[i] < 0.f)
			continue;

		float ratio = glm::clamp(1.f - m_arrfParticleLives[i] / m_fParticleLifetime, 0.f, 1.f);

		auto sl = m_vvvec3RawStreamlines[i];

		auto nSegments = sl.size() - 1;

		float indexPoint = ratio * nSegments;
		float endIndexPoint = (ratio - m_fParticleTail) * nSegments;

		int segmentBeginIndex = floor(indexPoint);

		float headLeftover = indexPoint - static_cast<float>(segmentBeginIndex);

		glm::vec3 segmentDir = sl[segmentBeginIndex + 1] - sl[segmentBeginIndex];

		glm::vec3 headPos = sl[segmentBeginIndex] + segmentDir * headLeftover;


		float segmentSize = 1.f / nSegments;


		int segmentEndIndex = ceil(endIndexPoint);
		if (segmentEndIndex < 0)
			segmentEndIndex = 0;

		float tailLeftover = static_cast<float>(segmentEndIndex) - endIndexPoint;
		glm::vec3 tailPos = sl[segmentEndIndex] + (sl[segmentEndIndex - 1] - sl[segmentEndIndex]) * tailLeftover;

		float colorMix = 0.f;
		float totalSize = m_fParticleTail * nSegments;

		if (endIndexPoint < 0.f)
		{
			segmentEndIndex = 0;
			tailLeftover = 0.f;
			tailPos = sl[0];
		}
		else
		{
			inds.push_back(verts.size());
			verts.push_back({ tailPos, glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

			colorMix += tailLeftover * segmentSize / m_fParticleTail;

			inds.push_back(verts.size());
			verts.push_back({ sl[segmentEndIndex], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

		}

		for (int j = segmentEndIndex; j < segmentBeginIndex; ++j)
		{
			inds.push_back(verts.size());
			verts.push_back({ sl[j], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

			colorMix += segmentSize / m_fParticleTail;

			inds.push_back(verts.size());
			verts.push_back({ sl[j + 1], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });
		}

		inds.push_back(verts.size());
		verts.push_back({ sl[segmentBeginIndex], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

		colorMix += headLeftover * segmentSize / m_fParticleTail;

		inds.push_back(verts.size());
		verts.push_back({ headPos, glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });
	}

	for (auto &p : m_qpDyingParticles)
	{
		auto sl = m_vvvec3RawStreamlines[p.first];

		float totalTailTime = m_fParticleTail * m_fParticleLifetime;

		float ratio = p.second / totalTailTime;

		float totalTailSize = (sl.size() - 1) * m_fParticleTail;

		float tailRemaining = ratio * totalTailSize;

		int tailIndex = ceil((sl.size() - 1) - tailRemaining);

		float tailLeftover = tailRemaining - static_cast<float>((sl.size() - 1) - tailIndex);

		glm::vec3 tailPos = sl[tailIndex] + (sl[tailIndex - 1] - sl[tailIndex]) * tailLeftover;

		float colorMix = 0.f;
		float segmentSize = (1.f / (sl.size() - 1));

		if (tailRemaining < 0.f)
		{
			tailIndex = 0;
			tailLeftover = 0.f;
			tailPos = sl[0];
		}
		else
		{
			inds.push_back(verts.size());
			verts.push_back({ tailPos, glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

			colorMix += tailLeftover * segmentSize / m_fParticleTail;

			inds.push_back(verts.size());
			verts.push_back({ sl[tailIndex], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });
		}
			   
		for (int j = tailIndex; j < sl.size() - 1; ++j)
		{
			inds.push_back(verts.size());
			verts.push_back({ sl[j], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });

			colorMix += segmentSize / m_fParticleTail;

			inds.push_back(verts.size());
			verts.push_back({ sl[j + 1], glm::mix(m_vec4ParticleTailColor, m_vec4ParticleHeadColor, colorMix) });
		}
	}

	glNamedBufferSubData(m_glParticleVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glParticleEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rsParticle.vertCount = inds.size();
}


glm::quat HairySlice::getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up)
{
	glm::vec3 w(glm::normalize(segmentDirection));
	glm::vec3 u(glm::normalize(glm::cross(up, w)));
	glm::vec3 v(glm::normalize(glm::cross(w, u)));
	return glm::toQuat(glm::mat3(u, v, w));
}