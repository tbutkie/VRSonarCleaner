#include "PointCleanProbe.h"

#include "InfoBoxManager.h"
#include "Renderer.h"
#include "DataLogger.h"

using namespace std::chrono_literals;

PointCleanProbe::PointCleanProbe(TrackedDeviceManager* pTDM, DataVolume* pointCloudVolume, vr::IVRSystem *pHMD)
	: ProbeBehavior(pTDM, pointCloudVolume)
	, m_bProbeActive(false)
	, m_bWaitForTriggerRelease(true)
	, m_pHMD(pHMD)
	, m_fPtHighlightAmt(1.f)
	, m_tpLastTime(std::chrono::high_resolution_clock::now())
	, m_fCursorHoopAngle(0.f)
	, m_nPointsSelected(0u)
{
}


PointCleanProbe::~PointCleanProbe()
{
}

void PointCleanProbe::update()
{	
	if (!m_pTDM->getPrimaryController())
		return;

	ProbeBehavior::update();

	if (m_bWaitForTriggerRelease && !m_pTDM->getPrimaryController()->isTriggerEngaged())
		m_bWaitForTriggerRelease = false;

	// Update time vars
	auto tick = std::chrono::high_resolution_clock::now();
	m_msElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(tick - m_tpLastTime);
	m_tpLastTime = tick;

	if (!m_bWaitForTriggerRelease && m_bEnabled)
		m_nPointsSelected = checkPoints();
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

	glm::vec4 diffCol = glm::mix(glm::vec4(0.125f, 0.125f, 0.125f, 1.f), glm::vec4(0.502f, 0.125f, 0.125f, 1.f), m_pTDM->getPrimaryController()->getTriggerPullAmount());
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

	// Trigger label and connector
	glm::mat4 triggerTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(-0.025f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

	Renderer::getInstance().drawText(
		"Clean Points",
		glm::mix(glm::vec4(1.f), glm::vec4(0.502f, 0.125f, 0.125f, 1.f), m_pTDM->getPrimaryController()->getTriggerPullAmount()),
		triggerTextAnchorTrans[3],
		glm::quat(triggerTextAnchorTrans),
		0.0075f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_RIGHT
	);

	Renderer::getInstance().drawConnector(
		triggerTextAnchorTrans[3],
		(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.03f, 0.05f)))[3],
		0.001f,
		glm::vec4(1.f, 1.f, 1.f, 0.75f)
	);

	// Point Count Label
	glm::mat4 statusTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

	Renderer::getInstance().drawText(
		std::to_string(m_nPointsSelected),
		glm::vec4(0.8f, 0.2f, 0.8f, 1.f),
		statusTextAnchorTrans[3],
		glm::quat(statusTextAnchorTrans),
		0.02f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_BOTTOM
	);

	Renderer::getInstance().drawText(
		std::string("Points Selected"),
		glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
		statusTextAnchorTrans[3],
		glm::quat(statusTextAnchorTrans),
		0.0075f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_TOP
	);
}

bool PointCleanProbe::isProbeActive()
{
	return m_bProbeActive;
}

bool PointCleanProbe::anyHits()
{
	return m_bAnyHits;
}

void PointCleanProbe::activateProbe()
{
	m_bProbeActive = true;

	if (DataLogger::getInstance().logging())
	{
		glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
		glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());

		std::stringstream ss;

		ss << "Probe Activated" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
		ss << ";";
		ss << "vol-dims:\"" << m_pDataVolume->getDimensions().x << "," << m_pDataVolume->getDimensions().y << "," << m_pDataVolume->getDimensions().z << "\"";
		ss << ";";
		ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
		ss << ";";
		ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";

		if (m_pTDM->getPrimaryController())
		{
			glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
			glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

			glm::mat4 probeTrans(getProbeToWorldTransform());

			ss << ";";
			ss << "probe-pos:\"" << probeTrans[3].x << "," << probeTrans[3].y << "," << probeTrans[3].z << "\"";
			ss << ";";
			ss << "probe-radius:\"" << getProbeRadius() << "\""; 
			ss << ";";
			ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			ss << ";";
			ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		}

		if (m_pTDM->getSecondaryController())
		{
			glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			ss << ";";
			ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		}

		DataLogger::getInstance().logMessage(ss.str());
	}
}

void PointCleanProbe::deactivateProbe()
{
	m_bProbeActive = false;

	if (DataLogger::getInstance().logging())
	{
		glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
		glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());

		std::stringstream ss;

		ss << "Probe Deactivated" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
		ss << ";";
		ss << "vol-dims:\"" << m_pDataVolume->getDimensions().x << "," << m_pDataVolume->getDimensions().y << "," << m_pDataVolume->getDimensions().z << "\"";
		ss << ";";
		ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
		ss << ";";
		ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";

		if (m_pTDM->getPrimaryController())
		{
			glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
			glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

			glm::mat4 probeTrans(getProbeToWorldTransform());

			ss << ";";
			ss << "probe-pos:\"" << probeTrans[3].x << "," << probeTrans[3].y << "," << probeTrans[3].z << "\"";
			ss << ";";
			ss << "probe-radius:\"" << getProbeRadius() << "\"";
			ss << ";";
			ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			ss << ";";
			ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		}

		if (m_pTDM->getSecondaryController())
		{
			glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			ss << ";";
			ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		}

		DataLogger::getInstance().logMessage(ss.str());
	}
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

unsigned int PointCleanProbe::checkPoints()
{
	if (!m_bActive)
		return 0u;

	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->poseValid()) 
		return 0u;

	glm::mat4 currentCursorPose = getProbeToWorldTransform();
	glm::mat4 lastCursorPose = getLastProbeToWorldTransform();

	float cursorRadius = m_fProbeRadius;

	bool clearPoints = m_pTDM->getPrimaryController()->isTriggerClicked();

	m_bAnyHits = false;

	unsigned int selectedPoints(0u);

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
					m_bAnyHits = true;
					cloud->markPoint(i, 1);
				}
				else
				{
					cloud->markPoint(i, 100.f + 100.f * m_fPtHighlightAmt);
					selectedPoints++;
				}

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
	
	if (m_bAnyHits)
		m_pHMD->TriggerHapticPulse(m_pTDM->getPrimaryController()->getIndex(), 0, 2000);

	return selectedPoints;
}