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

bool CleaningRoom::checkCleaningTable(Vector3 lastCursorCtr, Vector3 currentCursorCtr, GLfloat radius)
{
	Vector3 lastXformed, currentXformed;

	tableVolume->convertToInnerCoords(lastCursorCtr.x, lastCursorCtr.y, lastCursorCtr.z,
		&lastXformed.x, &lastXformed.y, &lastXformed.z);
	tableVolume->convertToInnerCoords(currentCursorCtr.x, currentCursorCtr.y, currentCursorCtr.z,
		&currentXformed.x, &currentXformed.y, &currentXformed.z);	 

	//std::cout << "last: (" << lastCursorCtr.x << ", " << lastCursorCtr.y << ", " << lastCursorCtr.z << ")" << std::endl;
	//std::cout << "curr: (" << currentCursorCtr.x << ", " << currentCursorCtr.y << ", " << currentCursorCtr.z << ")" << std::endl;
	//Vector3 v = currentCursorCtr - lastCursorCtr;
	//float dist = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	//std::cout << "dist: " << dist << std::endl << std::endl;

	//std::cout << "lastXform: (" << lastXformed.x << ", " << lastXformed.y << ", " << lastXformed.z << ")" << std::endl;
	//std::cout << "currXform: (" << currentXformed.x << ", " << currentXformed.y << ", " << currentXformed.z << ")" << std::endl;
	//Vector4 v2 = currentXformed - lastXformed;
	//dist = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);
	//std::cout << "dist: " << dist << std::endl << std::endl;
	//std::cout << "------------------------------------------------------------" << std::endl << std::endl;


	return clouds->getCloud(0)->checkForHit(Vector3(lastXformed.x, lastXformed.z, lastXformed.y), Vector3(currentXformed.x, currentXformed.z, currentXformed.y), 20.f);
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