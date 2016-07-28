#include "CleaningRoom.h"
#include "Quaternion.h"

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
	wallVolume = new DataVolume(0, (roomSizeY / 2)+(roomSizeY*0.09), (roomSizeZ / 2)-0.42, 1, (roomSizeX*0.9), (roomSizeY*0.80), 0.8);
	
	tableVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
	wallVolume->setInnerCoords(clouds->getXMin(), clouds->getXMax(), clouds->getMinDepth(), clouds->getMaxDepth(), clouds->getYMin(), clouds->getYMax());

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

bool CleaningRoom::checkCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, unsigned int sensitivity)
{
	bool anyHits = false;

	Vector4 ctr(0.f, 0.f, 0.f, 1.f);

	Vector4 currentCtrPos = currentCursorPose * ctr;
	Vector4 lastCtrPos = lastCursorPose * ctr;

	Vector4 lastToCurrentVec = currentCtrPos - lastCtrPos;
	float dist = lastToCurrentVec.length();
	float cylLen = dist / static_cast<float>(sensitivity);
	float cylLen_sq = cylLen * cylLen;
	float radius_sq = radius * radius;

	Vector4 up(0.f, 1.f, 0.f, 0.f);
	Vector4 currentUpVec = (currentCursorPose * up).normalize();
	Vector4 lastUpVec = (lastCursorPose * up).normalize();

	if (currentUpVec.dot(lastUpVec) <= 0.f)
	{
		printf("Last two cursor orientations don't look valid; aborting...");
		return false;
	}
	
	// used to flip the side of the cursor from which the cylinder is extended
	// -1 = below cursor; 1 = above cursor
	float cylSide = currentUpVec.dot(lastToCurrentVec) < 0.f ? -1.f : 1.f;
	
	// Initialize quaternions
	Quaternion lastQuat, thisQuat;
	lastQuat = lastCursorPose;
	thisQuat = currentCursorPose;
	std::vector<Quaternion> quats;
	quats.push_back(lastQuat);

	// Initialize subcylinder axis endpoints
	Vector4 cylBeginLast, cylEndLast;
	Vector4 cylBeginCurrent, cylEndCurrent;
	std::vector<Vector4> cylAxisPtPairs; // organized as cyl1 begin, cyl1 end, cyl2 begin, cyl2 end, ...

	cylBeginLast = lastCtrPos;
	cylEndLast = lastCtrPos + lastUpVec * cylLen * cylSide;
	cylBeginCurrent = currentCtrPos;
	cylEndCurrent = currentCtrPos - currentUpVec * cylLen * cylSide;
	cylAxisPtPairs.push_back(cylBeginLast);
	cylAxisPtPairs.push_back(cylEndLast);

	for (unsigned int i = 1; i < sensitivity - 1; ++i)
	{
		float ratio = static_cast<float>(i) / static_cast<float>(sensitivity);
		Vector4 ptOnPathLine = lastCtrPos + lastToCurrentVec * ratio;
		Quaternion q = Quaternion().slerp(lastQuat, thisQuat, ratio);
		Matrix4 mat;//= q.getMatrixWithCenter(ptOnPathLine) here, need to create a matrix from q and translate to ptOnPathLine
		// ptOnPathLine +  * cylLen * 0.75f
		cylAxisPtPairs.push_back(mat * Vector4(0.f, cylLen * 0.75, 0.f, 1.f));
		cylAxisPtPairs.push_back(mat * Vector4(0.f, -cylLen * 0.75, 0.f, 1.f));
	}


	std::vector<Vector3> pts = clouds->getCloud(0)->getPointPositions();

	

	// check if points are within volume
	bool refreshNeeded = false;
	for (int i = 0; i < pts.size(); ++i)
	{
		//skip already marked points
		if (clouds->getCloud(0)->getPointMark(i) != 0)
			continue;		

		bool hit = true;

		Vector3 ptInWorldCoords;
		tableVolume->convertToWorldCoords(pts[i].x, pts[i].y, pts[i].z, &ptInWorldCoords.x, &ptInWorldCoords.y, &ptInWorldCoords.z);

		float dist_sq_cyl1, dist_sq_cyl2 = -1.f;
		
		dist_sq_cyl1 = cylTest(cylBeginCurrent, cylEndCurrent, cylLen_sq, radius_sq, ptInWorldCoords);
		dist_sq_cyl2 = cylTest(cylBeginLast, cylEndLast, cylLen_sq, radius_sq, ptInWorldCoords);
		
		if (dist_sq_cyl1 >= 0.f || dist_sq_cyl2 >= 0.f)
		{
			anyHits = true;
			//printf("Cleaned point (%f, %f, %f) from cloud.\n", pts[i].x, pts[i].y, pts[i].z);
			clouds->getCloud(0)->markPoint(i, 1);
			refreshNeeded = true;
		}
	}

	if (refreshNeeded)
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


void CleaningRoom::draw()
{
	//printf("In CleaningRoom Draw()\n");
	holodeck->drawSolid();

	//draw debug
	wallVolume->drawBBox();
	wallVolume->drawBacking();
	tableVolume->drawBBox();
	tableVolume->drawBacking();

	
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