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
	

	tableVolume->setInnerCoords(cloud->getXMin(), cloud->getXMax(), cloud->getMinDepth(), cloud->getMaxDepth(), cloud->getYMin(), cloud->getYMax());
	wallVolume->setInnerCoords(cloud->getXMin(), cloud->getXMax(), cloud->getMinDepth(), cloud->getMaxDepth(), cloud->getYMin(), cloud->getYMax());

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
	cloud->draw();
	cloud->drawAxes();
	//tableVolume->deactivateTransformationMatrix();
	glPopMatrix();

	//draw wall
	glPushMatrix();
	wallVolume->activateTransformationMatrix();
	cloud->draw();
	cloud->drawAxes();
	glPopMatrix();
	//wallVolume->deactivateTransformationMatrix();


}