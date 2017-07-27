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
	GLubyte dkred[4] = { 0x80, 0x20, 0x20, 0xFF };
	GLubyte yellow[4] = { 0xFF, 0xFF, 0x00, 0xFF };

	glGenTextures(1, &m_glTorusDiffTex);
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glTorusDiffTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &dkred);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &m_glTorusSpecTex);
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glTorusSpecTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &yellow);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	generateTorus();
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
	if (!m_pController->readyToRender())
		return;

	drawProbe(m_fProbeOffset - m_fProbeRadius);	

	long long rate_ms_per_rev = 100ll / (1.f + 10.f * m_pController->getTriggerPullAmount());

	// Update time vars
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
	m_LastTime = std::chrono::high_resolution_clock::now();

	// Update rotation angle
	float angleNeeded = glm::two_pi<float>() * (elapsed_ms.count() % rate_ms_per_rev) / rate_ms_per_rev;
	m_fCursorHoopAngle += angleNeeded;

	glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(m_fProbeRadius));
	glm::mat4 rot;

	glm::vec4 diffCol = m_bProbeActive ? glm::vec4(0.502f, 0.125f, 0.125f, 1.f) : glm::vec4(0.125f, 0.125f, 0.125f, 1.f);
	glm::vec4 specColor = m_bProbeActive ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f);
	float specExp = 130.f;

	for (int n = 0; n < 3; ++n)
	{
		if (n == 0)
			rot = glm::rotate(glm::mat4(), m_fCursorHoopAngle, glm::vec3(1.f, 0.f, 0.f));
		if (n == 1)
			rot = glm::rotate(glm::mat4(), m_fCursorHoopAngle, glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		if (n == 2)
			rot = glm::rotate(glm::mat4(), m_fCursorHoopAngle, glm::vec3(0.f, 0.f, 1.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
		
		glm::mat4 torusToWorldTransform = getProbeToWorldTransform() * rot * scl;

		Renderer::getInstance().drawPrimitive("torus", torusToWorldTransform, diffCol, specColor, specExp);
	}
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

	glm::mat4 currentCursorPose = getProbeToWorldTransform();
	glm::mat4 lastCursorPose = getLastProbeToWorldTransform();

	float cursorRadius = m_fProbeRadius;

	bool clearPoints = m_pController->isTriggerClicked();
	


	glm::mat4 mat4CurrentVolumeXform = m_pDataVolume->getCurrentDataTransform();
	glm::mat4 mat4LastVolumeXform = m_pDataVolume->getLastDataTransform();

	if (mat4LastVolumeXform == glm::mat4())
		mat4LastVolumeXform = mat4CurrentVolumeXform;

	glm::vec3 vec3CurrentCursorPos = getPosition();
	glm::vec3 vec3LastCursorPos = getLastPosition();

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

void PointCleanProbe::generateTorus()
{
	float core_radius = 1.f;
	float meridian_radius = 0.025f;

	int nCoreSegs = 32;
	int nMeriSegs = 8;

	int nVerts = nCoreSegs * nMeriSegs;

	std::vector<glm::vec3> pts;
	std::vector<glm::vec3> norms;
	std::vector<glm::vec2> texUVs;
	std::vector<unsigned short> inds;

	for (int i = 0; i < nCoreSegs; i++)
		for (int j = 0; j < nMeriSegs; j++)
		{
			float u = i / (nCoreSegs - 1.f);
			float v = j / (nMeriSegs - 1.f);
			float theta = u * 2.f * M_PI;
			float rho = v * 2.f * M_PI;
			float x = cos(theta) * (core_radius + meridian_radius*cos(rho));
			float y = sin(theta) * (core_radius + meridian_radius*cos(rho));
			float z = meridian_radius*sin(rho);
			float nx = cos(theta)*cos(rho);
			float ny = sin(theta)*cos(rho);
			float nz = sin(rho);
			float s = u;
			float t = v;

			pts.push_back(glm::vec3(x, y, z));
			norms.push_back(glm::vec3(nx, ny, nz));
			texUVs.push_back(glm::vec2(s, t));

			unsigned short uvInd = i * nMeriSegs + j;
			unsigned short uvpInd = i * nMeriSegs + (j + 1) % nMeriSegs;
			unsigned short umvInd = (((i - 1) % nCoreSegs + nCoreSegs) % nCoreSegs) * nMeriSegs + j; // true modulo (not C++ remainder operand %) for negative wraparound
			unsigned short umvpInd = (((i - 1) % nCoreSegs + nCoreSegs) % nCoreSegs) * nMeriSegs + (j + 1) % nMeriSegs;

			inds.push_back(uvInd);   // (u    , v)
			inds.push_back(uvpInd);  // (u    , v + 1)
			inds.push_back(umvInd);  // (u - 1, v)
			
			inds.push_back(umvInd);  // (u - 1, v)
			inds.push_back(uvpInd);  // (u    , v + 1)
			inds.push_back(umvpInd); // (u - 1, v + 1)
		}	

	m_nTorusVertices = inds.size();

	glGenVertexArrays(1, &m_glTorusVAO);
	glGenBuffers(1, &m_glTorusVBO);
	glGenBuffers(1, &m_glTorusEBO);

	glBindVertexArray(this->m_glTorusVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glTorusVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glTorusEBO);

	// Set the vertex attribute pointers
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
	glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(pts.size() * sizeof(glm::vec3)));
	glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)(pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3)));
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, this->m_glTorusVBO);
	// Buffer orphaning
	glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3) + texUVs.size() * sizeof(glm::vec2), 0, GL_STREAM_DRAW);
	// Sub buffer data for points, normals, textures...
	glBufferSubData(GL_ARRAY_BUFFER, 0, pts.size() * sizeof(glm::vec3), &pts[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3), norms.size() * sizeof(glm::vec3), &norms[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3), texUVs.size() * sizeof(glm::vec2), &texUVs[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glTorusEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), 0, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), &inds[0], GL_STREAM_DRAW);
}
