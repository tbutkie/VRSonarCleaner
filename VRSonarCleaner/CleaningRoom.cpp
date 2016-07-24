#include "CleaningRoom.h"

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
	tableVolume = new DataVolume(0, 0.75, 0, 0, 2.25, 0.75, 2.25);
	wallVolume = new DataVolume(0, (roomSizeY / 2)+(roomSizeY*0.15), (roomSizeZ / 2)-0.42, 1, (roomSizeX*0.9), (roomSizeY*0.80), 0.8);
	
	tableVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
	wallVolume->setInnerCoords(clouds->getXMin(), clouds->getXMax(), clouds->getMinDepth(), clouds->getMaxDepth(), clouds->getYMin(), clouds->getYMax());

}

CleaningRoom::~CleaningRoom()
{

}

void CleaningRoom::setRoomSize(float SizeX, float SizeY, float SizeZ)
{
	roomSizeX = SizeX;
	roomSizeY = SizeY;
	roomSizeZ = SizeZ;
}

bool CleaningRoom::checkCleaningTable(Vector3 cursorCtr, Vector3 forwardRadiusPos, Vector3 cylAxisEndPos)
{
	bool hit = false;

	std::vector<Vector3> pts = clouds->getCloud(0)->getPointPositions();
	
	float cylAxisLen = (cylAxisEndPos - cursorCtr).length();
	float cylAxisLen_sq = cylAxisLen * cylAxisLen;
	
	float radius = (forwardRadiusPos - cursorCtr).length();
	float radius_sq = radius * radius;
	
	for (int i = 0; i < pts.size(); ++i)
	{
		Vector3 ptInWorldCoords;
		tableVolume->convertToWorldCoords(pts[i].x, pts[i].y, pts[i].z, &ptInWorldCoords.x, &ptInWorldCoords.y, &ptInWorldCoords.z);

		float dist_sq = cylTest(cursorCtr, cylAxisEndPos, cylAxisLen_sq, radius_sq, ptInWorldCoords);
		
		if (dist_sq >= 0.f)
		{
			hit = true;
			clouds->getCloud(0)->setPoint(i, 0.f, 0.f, 0.f);
			clouds->getCloud(0)->setRefreshNeeded();
		}
	}

	return hit;
}

// This code taken from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
// with credit to Greg James @ Nvidia
float CleaningRoom::cylTest(const Vector3 & pt1, const Vector3 & pt2, float lengthsq, float radius_sq, const Vector3 & testpt)
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
	holodeck->draw();

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