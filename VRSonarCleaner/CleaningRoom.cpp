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

	Vector4 up(0.f, 1.f, 0.f, 0.f);
	Vector4 currentUpVec = (currentCursorPose * up).normalize();
	Vector4 lastUpVec = (lastCursorPose * up).normalize();

	Vector4 lastToCurrentVec = currentCtrPos - lastCtrPos;
	float dist = lastToCurrentVec.length();	
	float verticalDistBetweenCursors = abs(lastToCurrentVec.dot(lastUpVec));
	float cylLen = verticalDistBetweenCursors / static_cast<float>(sensitivity);
	float cylLen_sq = cylLen * cylLen;
	float radius_sq = radius * radius;

	if (currentUpVec.dot(lastUpVec) <= 0.f)
	{
		printf("Last two cursor orientations don't look valid; aborting...");
		return false;
	}
	
	// used to flip the side of the cursor from which the cylinder is extended
	// -1 = below cursor; 1 = above cursor
	float cylSide = currentUpVec.dot(lastToCurrentVec) < 0.f ? -1.f : 1.f;
	
	// Initialize quaternions used for interpolating orientation
	Quaternion lastQuat, thisQuat;
	lastQuat = lastCursorPose;
	thisQuat = currentCursorPose;

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
	
	// Interpolate cursor positions and store the subcylinder axial endpoints
	for (unsigned int i = 1; i < sensitivity - 1; ++i)
	{
		float ratio = static_cast<float>(i) / static_cast<float>(sensitivity - 1);
		Vector4 ptOnPathLine = lastCtrPos + lastToCurrentVec * ratio;
		Quaternion q = Quaternion().slerp(lastQuat, thisQuat, ratio);
		Matrix4 mat = q.createMatrix();
		cylAxisPtPairs.push_back(ptOnPathLine + mat * Vector4(0.f, 1.f, 0.f, 1.f) * cylLen * 0.75);
		cylAxisPtPairs.push_back(ptOnPathLine + mat * -Vector4(0.f, 1.f, 0.f, 1.f) * cylLen * 0.75);
	}

	cylAxisPtPairs.push_back(cylBeginCurrent);
	cylAxisPtPairs.push_back(cylEndCurrent);

	for (unsigned int i = 0; i < sensitivity; ++i)
	{
		Vector3 start, end;
		tableVolume->convertToInnerCoords(cylAxisPtPairs[i * 2].x, cylAxisPtPairs[i * 2].y, cylAxisPtPairs[i * 2].z, &start.x, &start.y, &start.z);
		tableVolume->convertToInnerCoords(cylAxisPtPairs[i * 2 + 1].x, cylAxisPtPairs[i * 2 + 1].y, cylAxisPtPairs[i * 2 + 1].z, &end.x, &end.y, &end.z);
		clouds->getCloud(0)->setPoint(i * 2, start.x, start.z, start.y);
		clouds->getCloud(0)->markPoint(i * 2, 2);
		clouds->getCloud(0)->setPoint(i * 2 + 1, end.x, end.z, end.y);
		clouds->getCloud(0)->markPoint(i * 2 + 1, 3);

		//std::cout << "Path pt " << i << ": " << pt1 << " | " << pt2 << std::endl;
	}
	//std::cout << "-------------------------------------------" << std::endl << std::endl;

	clouds->getCloud(0)->setRefreshNeeded();

	std::vector<Vector3> pts = clouds->getCloud(0)->getPointPositions();	

	// check if points are within volume
	bool refreshNeeded = false;
	for (int i = 100; i < pts.size(); ++i)
	{
		//skip already marked points
		if (clouds->getCloud(0)->getPointMark(i) != 0)
			continue;		
		
		Vector3 ptInWorldCoords;
		tableVolume->convertToWorldCoords(pts[i].x, pts[i].y, pts[i].z, &ptInWorldCoords.x, &ptInWorldCoords.y, &ptInWorldCoords.z);
		
		for (int j = 0; j < cylAxisPtPairs.size(); j += 2)
		{
			if (cylTest(cylAxisPtPairs[j], cylAxisPtPairs[j+1], cylLen_sq, radius_sq, ptInWorldCoords) > 0.f)
			{
				anyHits = true;
				//printf("Cleaned point (%f, %f, %f) from cloud.\n", pts[i].x, pts[i].y, pts[i].z);
				clouds->getCloud(0)->markPoint(i, 1);
				refreshNeeded = true;
				break;
			}
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