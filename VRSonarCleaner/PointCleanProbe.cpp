#include "PointCleanProbe.h"

#include "InfoBoxManager.h"
#include "Renderer.h"

using namespace std::chrono_literals;

PointCleanProbe::PointCleanProbe(TrackedDeviceManager* pTDM, DataVolume* pointCloudVolume, vr::IVRSystem *pHMD)
	: ProbeBehavior(pTDM, pointCloudVolume)
	, m_bProbeActive(false)
	, m_pHMD(pHMD)
	, m_fPtHighlightAmt(1.f)
	, m_tpLastTime(std::chrono::high_resolution_clock::now())
	, m_fCursorHoopAngle(0.f)
{
	m_bWaitForTriggerRelease = m_pTDM->getPrimaryController()->isTriggerClicked() ? true : false;

	InfoBoxManager::getInstance().addInfoBox(
		"Editing Label",
		"editctrlrlabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Clean Label",
		"cleanrightlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(-0.05f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);
}


PointCleanProbe::~PointCleanProbe()
{
	InfoBoxManager::getInstance().removeInfoBox("Editing Label");
	InfoBoxManager::getInstance().removeInfoBox("Clean Label");
}

void PointCleanProbe::update()
{	
	ProbeBehavior::update();

	if (m_bWaitForTriggerRelease && !m_pTDM->getPrimaryController()->isTriggerClicked())
		m_bWaitForTriggerRelease = false;

	// Update time vars
	auto tick = std::chrono::high_resolution_clock::now();
	m_msElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(tick - m_tpLastTime);
	m_tpLastTime = tick;

	if (!m_bWaitForTriggerRelease)
		checkPoints();
}

void PointCleanProbe::draw()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->readyToRender())
		return;

	drawProbe(m_fProbeOffset - m_fProbeRadius);	

	float rate_ms_per_rev = POINT_CLOUD_CLEAN_PROBE_ROTATION_RATE.count() / (1.f + 10.f * m_pTDM->getPrimaryController()->getTriggerPullAmount());
	

	// Update rotation angle
	float angleNeeded = (360.f) * fmodf(m_msElapsedTime.count(), rate_ms_per_rev) / rate_ms_per_rev;
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

bool checkAABBtoAABBIntersection(glm::vec3 aabb1Min, glm::vec3 aabb1Max, glm::vec3 aabb2Min, glm::vec3 aabb2Max)
{
	return aabb1Max.x > aabb2Min.x && aabb1Min.x < aabb2Max.x &&
		   aabb1Max.y > aabb2Min.y && aabb1Min.y < aabb2Max.y &&
		   aabb1Max.z > aabb2Min.z && aabb1Min.z < aabb2Max.z;
}

bool checkPointInAABB(glm::vec3 point, glm::vec3 aabbMin, glm::vec3 aabbMax)
{
	return point.x > aabbMin.x && point.x < aabbMax.x &&
		   point.y > aabbMin.y && point.y < aabbMax.y &&
		   point.z > aabbMin.z && point.z < aabbMax.z;
}

void PointCleanProbe::checkPoints()
{
	if (!m_bActive)
		return;

	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->poseValid()) 
		return;

	glm::mat4 currentCursorPose = getProbeToWorldTransform();
	glm::mat4 lastCursorPose = getLastProbeToWorldTransform();

	float cursorRadius = m_fProbeRadius;

	bool clearPoints = m_pTDM->getPrimaryController()->isTriggerClicked();

	bool anyHits = false;

	float delta = m_msElapsedTime.count() / POINT_CLOUD_HIGHLIGHT_BLINK_RATE.count();
	m_fPtHighlightAmt = fmodf(m_fPtHighlightAmt + delta, 1.f);

	glm::vec3 vec3CurrentCursorPos = getPosition();
	glm::vec3 vec3LastCursorPos = getLastPosition();

	glm::vec3 vec3MinProbeAABB = glm::vec3(
		(std::min)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) - m_fProbeRadius,
		(std::min)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) - m_fProbeRadius,
		(std::min)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) - m_fProbeRadius
	);
	glm::vec3 vec3MaxProbeAABB = glm::vec3(
		(std::max)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) + m_fProbeRadius,
		(std::max)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) + m_fProbeRadius,
		(std::max)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) + m_fProbeRadius
	);

	for (auto &pc : m_pDataVolume->getDatasets())
	{
		SonarPointCloud* cloud = static_cast<SonarPointCloud*>(pc);

		glm::vec3 cloudMinBound = m_pDataVolume->convertToWorldCoords(cloud->getAdjustedMinBounds());
		glm::vec3 cloudMaxBound = m_pDataVolume->convertToWorldCoords(cloud->getAdjustedMaxBounds());

		//if (!checkAABBtoAABBIntersection(vec3MinProbeAABB, vec3MaxProbeAABB, cloudMinBound, cloudMaxBound))
		//	continue;

		glm::mat4 mat4CurrentVolumeXform = m_pDataVolume->getCurrentDataTransform(cloud);
		glm::mat4 mat4LastVolumeXform = m_pDataVolume->getLastDataTransform(cloud);

		if (mat4LastVolumeXform == glm::mat4())
			mat4LastVolumeXform = mat4CurrentVolumeXform;

		bool performCylTest = true;
		if (vec3CurrentCursorPos == vec3LastCursorPos) performCylTest = false;

		float cyl_len_sq = (vec3CurrentCursorPos.x - vec3LastCursorPos.x) * (vec3CurrentCursorPos.x - vec3LastCursorPos.x) +
			(vec3CurrentCursorPos.y - vec3LastCursorPos.y) * (vec3CurrentCursorPos.y - vec3LastCursorPos.y) +
			(vec3CurrentCursorPos.z - vec3LastCursorPos.z) * (vec3CurrentCursorPos.z - vec3LastCursorPos.z);

		// POINTS CHECK
		bool pointsRefresh = false;

		for (unsigned int i = 0u; i < cloud->getPointCount(); ++i)
		{
			//skip already marked points
			if (cloud->getPointMark(i) == 1)
				continue;

			glm::vec3 thisPt = glm::vec3(mat4CurrentVolumeXform * glm::vec4(cloud->getAdjustedPointPosition(i), 1.f));

			// fast point-in-AABB failure test
			if (!checkPointInAABB(thisPt, vec3MinProbeAABB, vec3MaxProbeAABB))
			{
				if (cloud->getPointMark(i) != 0)
				{
					cloud->markPoint(i, 0);
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
					cloud->markPoint(i, 1);
				}
				else
					cloud->markPoint(i, 100.f + 100.f * m_fPtHighlightAmt);

				pointsRefresh = true;
			}
			else if (cloud->getPointMark(i) != 0)
			{
				cloud->markPoint(i, 0);
				pointsRefresh = true;
			}
		}

		if (pointsRefresh)
			cloud->setRefreshNeeded();
	}
	
	if (anyHits)
		m_pHMD->TriggerHapticPulse(m_pTDM->getPrimaryController()->getIndex(), 0, 2000);
}