#include "CleaningRoom.h"
#include "DebugDrawer.h"

CleaningRoom::CleaningRoom()
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	roomSizeX = 10;
	roomSizeY = 4;
	roomSizeZ = 6;

	holodeck = new HolodeckBackground(roomSizeX, roomSizeY, roomSizeZ, 0.25);

	minX = 0 - (roomSizeX / 2);
	minY = 0 - (roomSizeY / 2);
	minZ = 0;

	maxX = (roomSizeX / 2);
	maxY = (roomSizeY / 2);
	maxZ = roomSizeZ;
		
	//tableVolume = new DataVolume(0, 0.25, 0, 0, 1.25, 0.4, 1.25);
	tableVolume = new DataVolume(0, 1.10, 0, 0, 2.25, 0.75, 2.25);
	wallVolume = new DataVolume(0, (roomSizeY / 2)+(roomSizeY*0.09), (roomSizeZ / 2)-0.42, 1, (roomSizeX*0.9), 0.8, (roomSizeY*0.80));
	
	tableVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
	wallVolume->setInnerCoords(clouds->getXMin(), clouds->getXMax(), clouds->getMinDepth(), clouds->getMaxDepth(), clouds->getYMin(), clouds->getYMax());

	m_fPtHighlightAmt = 1.f;
	m_LastTime = std::chrono::high_resolution_clock::now();
}

CleaningRoom::~CleaningRoom()
{

}

void CleaningRoom::recalcVolumeBounds()
{
	tableVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
	wallVolume->setInnerCoords(clouds->getXMin(), clouds->getXMax(), clouds->getMinDepth(), clouds->getMaxDepth(), clouds->getYMin(), clouds->getYMax());
}

void CleaningRoom::setRoomSize(float SizeX, float SizeY, float SizeZ)
{
	roomSizeX = SizeX;
	roomSizeY = SizeY;
	roomSizeZ = SizeZ;
}

// This function works but could be improved in efficiency if needed
//bool CleaningRoom::checkCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, unsigned int sensitivity)
//{
//	bool anyHits = false;
//
//	float radius_sq = radius * radius;
//
//	Vector4 ctr(0.f, 0.f, 0.f, 1.f);
//
//	Vector4 currentCtrPos = currentCursorPose * ctr;
//	Vector4 lastCtrPos = lastCursorPose * ctr;
//
//	Vector4 up(0.f, 1.f, 0.f, 0.f);
//	Vector4 currentUpVec = (currentCursorPose * up).normalize();
//	Vector4 lastUpVec = (lastCursorPose * up).normalize();
//
//	Vector4 lastToCurrentVec = currentCtrPos - lastCtrPos;
//	float dist = lastToCurrentVec.length();	
//	float verticalDistBetweenCursors = abs(lastToCurrentVec.dot(lastUpVec));
//
//	float cylLen = verticalDistBetweenCursors / static_cast<float>(sensitivity);
//	if (cylLen < 0.001f) cylLen = 0.001f;
//	float cylLen_sq = cylLen * cylLen;
//
//	float subCylOverlap = 0.25f; // overlap on each side of subcylinder; 0.f = no overlap, 1.f = 100% overlap
//	float subCylLen = cylLen * (1.f + 2.f * subCylOverlap);
//	float subCylLen_sq = subCylLen * subCylLen;
//
//	if (currentUpVec.dot(lastUpVec) <= 0.f)
//	{
//		printf("Last two cursor orientations don't look valid; aborting point check...");
//		return false;
//	}
//	
//	// used to flip the side of the cursor from which the cylinder is extended
//	// -1 = below cursor (-y); 1 = above cursor (+y)
//	float cylSide = currentUpVec.dot(lastToCurrentVec) < 0.f ? -1.f : 1.f;
//	
//	// Initialize quaternions used for interpolating orientation
//	Quaternion lastQuat, thisQuat;
//	lastQuat = lastCursorPose;
//	thisQuat = currentCursorPose;
//
//	// Initialize subcylinder axis endpoints
//	Vector4 cylBeginLast, cylEndLast;
//	Vector4 cylBeginCurrent, cylEndCurrent;
//	std::vector<Vector4> cylAxisPtPairs; // organized as cyl1 begin, cyl1 end, cyl2 begin, cyl2 end, ...
//
//	cylBeginLast = lastCtrPos;
//	cylEndLast = lastCtrPos + lastUpVec * cylLen * cylSide;
//	cylBeginCurrent = currentCtrPos;
//	cylEndCurrent = currentCtrPos - currentUpVec * cylLen * cylSide;
//	cylAxisPtPairs.push_back(cylBeginLast);
//	cylAxisPtPairs.push_back(cylEndLast);
//	
//	// Interpolate cursor positions and store the subcylinder axial endpoints
//	for (unsigned int i = 1; i < sensitivity - 1; ++i)
//	{
//		float ratio = static_cast<float>(i) / static_cast<float>(sensitivity - 1);
//		Vector4 ptOnPathLine = lastCtrPos + lastToCurrentVec * ratio;
//		Quaternion q = Quaternion().slerp(lastQuat, thisQuat, ratio);
//		Matrix4 mat = q.createMatrix();
//		// move cylinder axis endpoints out from center point that lies along the path line of the cursor centers
//		cylAxisPtPairs.push_back(ptOnPathLine + mat * Vector4(0.f, 1.f, 0.f, 1.f) * cylLen * (0.5f + subCylOverlap));
//		cylAxisPtPairs.push_back(ptOnPathLine + mat * Vector4(0.f, -1.f, 0.f, 1.f) * cylLen * (0.5f + subCylOverlap));
//	}
//
//	cylAxisPtPairs.push_back(cylBeginCurrent);
//	cylAxisPtPairs.push_back(cylEndCurrent);
//	
//	std::vector<Vector3> pts = clouds->getCloud(0)->getPointPositions();	
//
//	// check if points are within volume
//	bool refreshNeeded = false;
//	for (int i = 0; i < pts.size(); ++i)
//	{
//		//skip already marked points
//		if (clouds->getCloud(0)->getPointMark(i) != 0)
//			continue;		
//		
//		Vector3 ptInWorldCoords;
//		tableVolume->convertToWorldCoords(pts[i].x, pts[i].y, pts[i].z, &ptInWorldCoords.x, &ptInWorldCoords.y, &ptInWorldCoords.z);
//		
//		for (int j = 0; j < cylAxisPtPairs.size() / 2; ++j)
//		{
//			float len_sq;
//			if (j == 0 || j == cylAxisPtPairs.size() - 1)
//				len_sq = cylLen_sq;
//			else
//				len_sq = subCylLen_sq;
//
//			if (cylTest(cylAxisPtPairs[j * 2], cylAxisPtPairs[j * 2 +1], len_sq, radius_sq, ptInWorldCoords) > 0.f)
//			{
//				anyHits = true;
//				//printf("Cleaned point (%f, %f, %f) from cloud.\n", pts[i].x, pts[i].y, pts[i].z);
//				clouds->getCloud(0)->markPoint(i, 1);
//				refreshNeeded = true;
//				break;
//			}
//		}
//	}
//
//	if (refreshNeeded)
//		clouds->getCloud(0)->setRefreshNeeded();
//
//	return anyHits;
//}

bool CleaningRoom::editCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, bool clearPoints)
{
	glm::mat4 mat4CurrentVolumeXform = tableVolume->getCurrentTransform();
	glm::mat4 mat4LastVolumeXform = tableVolume->getLastTransform();

	if (mat4LastVolumeXform == glm::mat4()) mat4LastVolumeXform = mat4CurrentVolumeXform;

	glm::vec3 vec3CurrentCursorPos = glm::vec3(glm::make_mat4(currentCursorPose.get())[3]);
	glm::vec3 vec3LastCursorPos = glm::vec3((mat4CurrentVolumeXform * glm::inverse(mat4LastVolumeXform) * glm::make_mat4(lastCursorPose.get()))[3]);

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

	std::vector<Vector3> points = clouds->getCloud(0)->getPointPositions();

	for (size_t i = 0ull; i < points.size(); ++i)
	{
		//skip already marked points
		if (clouds->getCloud(0)->getPointMark(i) == 1)
			continue;

		glm::vec3 thisPt = glm::vec3(mat4CurrentVolumeXform * glm::vec4(points[i].x, points[i].y, points[i].z, 1.f));

		// fast point-in-AABB failure test (NEEDS REVISION)
		if (thisPt.x < std::min(vec3CurrentCursorPos.x, vec3LastCursorPos.x) - radius ||
			thisPt.x > std::max(vec3CurrentCursorPos.x, vec3LastCursorPos.x) + radius ||
			thisPt.y < std::min(vec3CurrentCursorPos.y, vec3LastCursorPos.y) - radius ||
			thisPt.y > std::max(vec3CurrentCursorPos.y, vec3LastCursorPos.y) + radius ||
			thisPt.z < std::min(vec3CurrentCursorPos.z, vec3LastCursorPos.z) - radius ||
			thisPt.z > std::max(vec3CurrentCursorPos.z, vec3LastCursorPos.z) + radius)
		{			
			if (clouds->getCloud(0)->getPointMark(i) != 0)
			{
				clouds->getCloud(0)->markPoint(i, 0);
				pointsRefresh = true;
			}
			continue;
		}

		float radius_sq = radius * radius;
		float current_dist_sq = (thisPt.x - vec3CurrentCursorPos.x) * (thisPt.x - vec3CurrentCursorPos.x) +
			(thisPt.y - vec3CurrentCursorPos.y) * (thisPt.y - vec3CurrentCursorPos.y) +
			(thisPt.z - vec3CurrentCursorPos.z) * (thisPt.z - vec3CurrentCursorPos.z);
		
		if (current_dist_sq <= radius_sq ||
			(performCylTest && cylTest(Vector4(vec3CurrentCursorPos.x, vec3CurrentCursorPos.y, vec3CurrentCursorPos.z, 1.f),
									   Vector4(vec3LastCursorPos.x, vec3LastCursorPos.y, vec3LastCursorPos.z, 1.f),
									   cyl_len_sq,
									   radius_sq,
									   Vector3(thisPt.x, thisPt.y, thisPt.z)) >= 0)
			)
		{
			if (clearPoints)
			{
				anyHits = true;
				clouds->getCloud(0)->markPoint(i, 1);
			}
			else
				clouds->getCloud(0)->markPoint(i, 100.f + 100.f * m_fPtHighlightAmt);

			pointsRefresh = true;
		}
		else if (clouds->getCloud(0)->getPointMark(i) != 0)
		{
			clouds->getCloud(0)->markPoint(i, 0);
			pointsRefresh = true;
		}
	}

	if (pointsRefresh)
		clouds->getCloud(0)->setRefreshNeeded();

	return anyHits;
}

// This code taken from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
// with credit to Greg James @ Nvidia
float CleaningRoom::cylTest(const Vector4 & pt1, const Vector4 & pt2, float lengthsq, float radius_sq, const Vector3 & testpt)
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

bool CleaningRoom::gripCleaningTable(const Matrix4 *controllerPose)
{
	if (!controllerPose)
	{
		if (tableVolume->isBeingRotated())
		{
			tableVolume->endRotation();
			//printf("|| Rotation Ended\n");
		}

		return false;
	}

	if (!tableVolume->isBeingRotated())
	{
		tableVolume->startRotation(controllerPose->get());
		//printf("++ Rotation Started\n");
		return true;
	}
	else
	{
		tableVolume->continueRotation(controllerPose->get());
		//printf("==== Rotating\n");
	}
	return false;
}


void CleaningRoom::draw()
{
	//printf("In CleaningRoom Draw()\n");
	holodeck->drawSolid();

	//draw debug
	wallVolume->drawBBox();
	wallVolume->drawBacking();
	wallVolume->drawAxes();
	tableVolume->drawBBox();
	tableVolume->drawBacking();
	tableVolume->drawAxes();

	
	//draw table
	glPushMatrix();
	tableVolume->activateTransformationMatrix();
	clouds->getCloud(0)->draw();
	//clouds->getCloud(0)->drawAxes();
	//tableVolume->deactivateTransformationMatrix();
	glPopMatrix();

	//draw wall
	glPushMatrix();
	wallVolume->activateTransformationMatrix();
	clouds->drawAllClouds();
	//cloud->drawAxes();
	glPopMatrix();
	//wallVolume->deactivateTransformationMatrix();


}

void CleaningRoom::resetVolumes()
{
	wallVolume->resetPositionAndOrientation();
	tableVolume->resetPositionAndOrientation();
}