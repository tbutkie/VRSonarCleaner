#include "CleaningRoom.h"

CleaningRoom::CleaningRoom(float SizeX, float SizeY, float SizeZ)
{
	roomSizeX = SizeX;
	roomSizeY = SizeY;
	roomSizeZ = SizeZ;

	holodeck = new HolodeckBackground(roomSizeX, roomSizeY, roomSizeZ);

	minX = 0 - (roomSizeX / 2);
	minY = 0 - (roomSizeY / 2);
	minZ = 0;

	maxX = (roomSizeX / 2);
	maxY = (roomSizeY / 2);
	maxZ = roomSizeZ;
		

}

CleaningRoom::~CleaningRoom()
{

}

void CleaningRoom::draw()
{
	holodeck->draw();

}