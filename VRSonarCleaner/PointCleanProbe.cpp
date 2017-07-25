#include "PointCleanProbe.h"

#include "GLSLpreamble.h"

#include "Renderer.h"

PointCleanProbe::PointCleanProbe(ViveController* controller, DataVolume* pointCloudVolume, SonarPointCloud *pCloud, vr::IVRSystem *pHMD)
	: ProbeBehavior(controller, pointCloudVolume)
	, m_bProbeActive(false)
	, m_pPointCloud(pCloud)
	, m_pHMD(pHMD)
	, m_fPtHighlightAmt(1.f)
{
	glGenVertexArrays(1, &m_glVAO);
	glGenBuffers(1, &m_glVBO);
	glGenBuffers(1, &m_glEBO);

	glBindVertexArray(this->m_glVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glBindVertexArray(0);

	generateCylinder(16);
}


PointCleanProbe::~PointCleanProbe()
{
}

void PointCleanProbe::update()
{
	checkPoints();
}

void PointCleanProbe::draw()
{
	Renderer::RendererSubmission rs;

	rs.primitiveType = GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.VAO = m_glVAO;
	rs.diffuseTex;
	rs.specularTex;
	rs.specularExponent = 30.f;
	rs.vertCount = m_nCylVertices;
	rs.modelToWorldTransform = m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(0.01f, 0.01f, m_fProbeOffset));
	rs.indexType = GL_UNSIGNED_SHORT;

	Renderer::getInstance().addToDynamicRenderQueue(rs);

}

void PointCleanProbe::activateProbe()
{
	m_bProbeActive = true;
}

void PointCleanProbe::deactivateProbe()
{
	m_bProbeActive = false;
}

// This code taken from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
// with credit to Greg James @ Nvidia
float cylTest(const glm::vec4 & pt1, const glm::vec4 & pt2, float lengthsq, float radius_sq, const glm::vec3 & testpt)
{
	float dx, dy, dz;	// vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;	// vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2.x - pt1.x;	// translate so pt1 is origin.  Make vector from
	dy = pt2.y - pt1.y;     // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z - pt1.z;

	pdx = testpt.x - pt1.x;		// vector from pt1 to test point.
	pdy = testpt.y - pt1.y;
	pdz = testpt.z - pt1.z;

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if (dot < 0.f || dot > lengthsq)
		return(-1.f);
	else
	{
		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot / lengthsq;

		if (dsq > radius_sq)
			return(-1.f);
		else
			return(dsq);		// return distance squared to axis
	}
}

void PointCleanProbe::checkPoints()
{
	if (!m_pController || !m_pController->poseValid()) 
		return;

	glm::mat4 currentCursorPose = m_pController->getPose() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
	glm::mat4 lastCursorPose = m_pController->getLastPose() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);

	float cursorRadius = m_fProbeRadius;

	bool clearPoints = m_pController->isTriggerClicked();
	


	glm::mat4 mat4CurrentVolumeXform = m_pDataVolume->getCurrentDataTransform();
	glm::mat4 mat4LastVolumeXform = m_pDataVolume->getLastDataTransform();

	if (mat4LastVolumeXform == glm::mat4())
		mat4LastVolumeXform = mat4CurrentVolumeXform;

	glm::vec3 vec3CurrentCursorPos = glm::vec3(currentCursorPose[3]);
	glm::vec3 vec3LastCursorPos = glm::vec3((mat4CurrentVolumeXform * glm::inverse(mat4LastVolumeXform) * lastCursorPose)[3]);

	bool performCylTest = true;
	if (vec3CurrentCursorPos == vec3LastCursorPos) performCylTest = false;

	float cyl_len_sq = (vec3CurrentCursorPos.x - vec3LastCursorPos.x) * (vec3CurrentCursorPos.x - vec3LastCursorPos.x) +
		(vec3CurrentCursorPos.y - vec3LastCursorPos.y) * (vec3CurrentCursorPos.y - vec3LastCursorPos.y) +
		(vec3CurrentCursorPos.z - vec3LastCursorPos.z) * (vec3CurrentCursorPos.z - vec3LastCursorPos.z);

	// CLOCK UPDATE
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
	m_LastTime = std::chrono::high_resolution_clock::now();

	float blink_rate_ms = 250.f;

	float delta = static_cast<float>(elapsed_ms.count()) / blink_rate_ms;
	m_fPtHighlightAmt = fmodf(m_fPtHighlightAmt + delta, 1.f);

	// POINTS CHECK
	bool anyHits = false;
	bool pointsRefresh = false;

	std::vector<glm::vec3> points = m_pPointCloud->getPointPositions();

	for (size_t i = 0ull; i < points.size(); ++i)
	{
		//skip already marked points
		if (m_pPointCloud->getPointMark(i) == 1)
			continue;

		glm::vec3 thisPt = glm::vec3(mat4CurrentVolumeXform * glm::vec4(points[i].x, points[i].y, points[i].z, 1.f));

		// fast point-in-AABB failure test
		if (thisPt.x < (std::min)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) - m_fProbeRadius ||
			thisPt.x >(std::max)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) + m_fProbeRadius ||
			thisPt.y < (std::min)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) - m_fProbeRadius ||
			thisPt.y >(std::max)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) + m_fProbeRadius ||
			thisPt.z < (std::min)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) - m_fProbeRadius ||
			thisPt.z >(std::max)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) + m_fProbeRadius)
		{
			if (m_pPointCloud->getPointMark(i) != 0)
			{
				m_pPointCloud->markPoint(i, 0);
				pointsRefresh = true;
			}
			continue;
		}

		float radius_sq = m_fProbeRadius * m_fProbeRadius;
		float current_dist_sq = (thisPt.x - vec3CurrentCursorPos.x) * (thisPt.x - vec3CurrentCursorPos.x) +
			(thisPt.y - vec3CurrentCursorPos.y) * (thisPt.y - vec3CurrentCursorPos.y) +
			(thisPt.z - vec3CurrentCursorPos.z) * (thisPt.z - vec3CurrentCursorPos.z);

		if (current_dist_sq <= radius_sq ||
			(performCylTest && cylTest(glm::vec4(vec3CurrentCursorPos.x, vec3CurrentCursorPos.y, vec3CurrentCursorPos.z, 1.f),
				glm::vec4(vec3LastCursorPos.x, vec3LastCursorPos.y, vec3LastCursorPos.z, 1.f),
				cyl_len_sq,
				radius_sq,
				glm::vec3(thisPt.x, thisPt.y, thisPt.z)) >= 0)
			)
		{
			if (clearPoints)
			{
				anyHits = true;
				m_pPointCloud->markPoint(i, 1);
			}
			else
				m_pPointCloud->markPoint(i, 100.f + 100.f * m_fPtHighlightAmt);

			pointsRefresh = true;
		}
		else if (m_pPointCloud->getPointMark(i) != 0)
		{
			m_pPointCloud->markPoint(i, 0);
			pointsRefresh = true;
		}
	}

	if (pointsRefresh)
		m_pPointCloud->setRefreshNeeded();

	if (anyHits)
		m_pHMD->TriggerHapticPulse(m_pController->getIndex(), 0, 2000);
}

void PointCleanProbe::generateGeometry()
{
}

void PointCleanProbe::generateCylinder(int numSegments)
{
	std::vector<glm::vec3> pts;
	std::vector<glm::vec3> norms;
	std::vector<glm::vec2> texUVs;
	std::vector<unsigned short> inds;

	// Front endcap
	pts.push_back(glm::vec3(0.f));
	norms.push_back(glm::vec3(0.f, 0.f, -1.f));
	texUVs.push_back(glm::vec2(0.5f, 0.5f));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		norms.push_back(glm::vec3(0.f, 0.f, -1.f));
		texUVs.push_back((glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f);

		if (i > 0)
		{
			inds.push_back(0);
			inds.push_back(pts.size() - 2);
			inds.push_back(pts.size() - 1);
		}
	}
	inds.push_back(0);
	inds.push_back(pts.size() - 1);
	inds.push_back(1);

	// Back endcap
	pts.push_back(glm::vec3(0.f, 0.f, 1.f));
	norms.push_back(glm::vec3(0.f, 0.f, 1.f));
	texUVs.push_back(glm::vec2(0.5f, 0.5f));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 1.f));
		norms.push_back(glm::vec3(0.f, 0.f, 1.f));
		texUVs.push_back((glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f);

		if (i > 0)
		{
			inds.push_back(pts.size() - (i + 2)); // ctr pt of endcap
			inds.push_back(pts.size() - 1);
			inds.push_back(pts.size() - 2);
		}
	}
	inds.push_back(pts.size() - (numSegments + 1));
	inds.push_back(pts.size() - (numSegments));
	inds.push_back(pts.size() - 1);

	// Shaft
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		norms.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		texUVs.push_back(glm::vec2((float)i / (float)(numSegments - 1), 0.f));

		pts.push_back(glm::vec3(sin(angle), cos(angle), 1.f));
		norms.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		texUVs.push_back(glm::vec2((float)i / (float)(numSegments - 1), 1.f));

		if (i > 0)
		{
			inds.push_back(pts.size() - 4);
			inds.push_back(pts.size() - 3);
			inds.push_back(pts.size() - 2);

			inds.push_back(pts.size() - 2);
			inds.push_back(pts.size() - 3);
			inds.push_back(pts.size() - 1);
		}
	}
	inds.push_back(pts.size() - 2);
	inds.push_back(pts.size() - numSegments * 2);
	inds.push_back(pts.size() - 1);

	inds.push_back(pts.size() - numSegments * 2);
	inds.push_back(pts.size() - numSegments * 2 + 1);
	inds.push_back(pts.size() - 1);

	m_nCylVertices = inds.size();

	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	// Buffer orphaning
	glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3) + texUVs.size() * sizeof(glm::vec2), 0, GL_STREAM_DRAW);
	// Sub buffer data for points, then colors
	glBufferSubData(GL_ARRAY_BUFFER, 0, pts.size() * sizeof(glm::vec3), &pts[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3), norms.size() * sizeof(glm::vec3), &norms[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3), texUVs.size() * sizeof(glm::vec2), &texUVs[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), 0, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), &inds[0], GL_STREAM_DRAW);

	// Set attribute pointers now that point array sizes are known
	glBindVertexArray(this->m_glVAO);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(pts.size() * sizeof(glm::vec3)));
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)(pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3)));
	glBindVertexArray(0);
}

void PointCleanProbe::generateTorus()
{
}
