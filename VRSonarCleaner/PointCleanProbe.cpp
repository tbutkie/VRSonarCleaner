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

	long long rate_ms_per_rev = 2000ll / (1.f + 10.f * m_pController->getTriggerPullAmount());

	// Update time vars
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
	m_LastTime = std::chrono::high_resolution_clock::now();

	// Update rotation angle
	float angleNeeded = (360.f) * (elapsed_ms.count() % rate_ms_per_rev) / rate_ms_per_rev;
	m_fCursorHoopAngle += angleNeeded;

	glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(m_fProbeRadius));
	glm::mat4 rot;

	glm::vec4 diffCol = m_bProbeActive ? glm::vec4(0.502f, 0.125f, 0.125f, 1.f) : glm::vec4(0.125f, 0.125f, 0.125f, 1.f);
	glm::vec4 specColor = m_bProbeActive ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f);
	float specExp = 130.f;

	for (int n = 0; n < 3; ++n)
	{
		if (n == 0)
			rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(1.f, 0.f, 0.f));
		if (n == 1)
			rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		if (n == 2)
			rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 0.f, 1.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
		
		glm::mat4 torusToWorldTransform = getProbeToWorldTransform() * rot * scl;

		Renderer::getInstance().drawPrimitive("torus", torusToWorldTransform, diffCol, specColor, specExp);
	}

	//glm::mat4 matSphere = getProbeToWorldTransform() * glm::scale(glm::mat4(), glm::vec3(m_fProbeRadius));
	//
	//Renderer::getInstance().drawPrimitive("inverse_icosphere", matSphere, diffCol, diffCol, 0.f);
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