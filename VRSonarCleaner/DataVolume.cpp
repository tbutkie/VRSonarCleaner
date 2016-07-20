#include "DataVolume.h"

DataVolume::DataVolume(float PosX, float PosY, float PosZ, int Orientation, float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	posX = PosX;
	posY = PosY;
	posZ = PosZ;

	minX = PosX - (sizeX / 2);
	minY = PosY - (sizeY / 2);
	minZ = PosZ - (sizeZ / 2);

	maxX = PosX + (sizeX / 2);
	maxY = PosY + (sizeY / 2);
	maxZ = PosZ + (sizeZ / 2);

	orientation = Orientation;

}

DataVolume::~DataVolume()
{

}

void DataVolume::setSize(float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	minX = posX - (sizeX / 2);
	minY = posY - (sizeY / 2);
	minZ = posZ - (sizeZ / 2);

	maxX = posX + (sizeX / 2);
	maxY = posY + (sizeY / 2);
	maxZ = posZ + (sizeZ / 2);

	recalcScaling();
}

void DataVolume::setPosition(float PosX, float PosY, float Pos)
{
	minX = posX - (sizeX / 2);
	minY = posY - (sizeY / 2);
	minZ = posZ - (sizeZ / 2);

	maxX = posX + (sizeX / 2);
	maxY = posY + (sizeY / 2);
	maxZ = posZ + (sizeZ / 2);

	recalcScaling();
}

void DataVolume::setInnerCoords(double MinX, double MaxX, double MinY, double MaxY, double MinZ, double MaxZ)
{
	innerMinX = MinX;
	innerMaxX = MaxX;
	innerMinY = MinY;
	innerMaxY = MaxY;
	innerMinZ = MinZ;
	innerMaxZ = MaxZ;
	innerSizeX = MaxX - MinX;
	innerSizeY = MaxY - MinY;
	innerSizeZ = MaxZ - MinZ;

	recalcScaling();
}

void DataVolume::recalcScaling()
{
	if (orientation == 0) //z-up table
	{
		XZscale = (float)std::min((double)sizeX / innerSizeX, (double)sizeZ / innerSizeZ);
		//float newWidth = innerSizeX*XYscale;
		//float newHeight = innerSizeY*XYscale;

		depthScale = sizeY / (float)innerSizeY;
	}
	else if (orientation == 1) //rotated wall view
	{
		XZscale = (float)std::min((double)sizeX / innerSizeX, (double)sizeY / innerSizeZ);
		
		depthScale = sizeZ / (float)innerSizeY;
	}

	printf("Scaling Factor for Orientation %d is XZ: %f, Y: %f", orientation, XZscale, depthScale);

}//end recalc scaling

void DataVolume::convertToInnerCoords(float xWorld , float yWorld, float zWorld, float *xInner, float *yInner, float *zInner)
{

}

void DataVolume::convertToWorldCoords(float xInner, float yInner, float zInner, float *xWorld, float *yWorld, float *zWorld)
{

}

void DataVolume::activateTransformationMatrix()
{
	//store current matrix so we can restore it later in deactivateTransformationMatrix(), this isn't working right now for some reason
	glMatrixMode(GL_MODELVIEW);
	glGetDoublev(GL_MODELVIEW, storedMatrix);

	glTranslatef(posX, posY, posZ);
	if (orientation == 0) //table view 
	{
		//glScalef(-1.0, 1.0, 1.0);
		//glRotatef(180.0, 0.0, 0.0, 1.0);
		//glRotatef(90.0, 1.0, 0.0, 0.0);
		glScalef(XZscale, depthScale, XZscale);
		glTranslatef(-((float)innerSizeX) / 2, ((float)innerSizeY) / 2, -((float)innerSizeZ) / 2);
	}
	if (orientation == 1) //wall view
	{
		//glScalef(-1.0, 1.0, 1.0);
		//glRotatef(180.0, 0.0, 0.0, 1.0);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		//glScalef(XZscale, XZscale, depthScale);
		glScalef(XZscale, depthScale, XZscale);
		glTranslatef(-((float)innerSizeX) / 2, ((float)innerSizeY) / 2, -((float)innerSizeZ) / 2);
	}
	
}


//This doesn't work, not sure why, using glPushMatrix and glPopMatrix outisde of here instead.
void DataVolume::deactivateTransformationMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(storedMatrix);
}


void DataVolume::drawBBox()
{
	//printf("In Holodeck Draw()\n");
	//glColor3f(0.75, 0.0, 0.0);
	glColor4f(0.22, 0.25, 0.34, 1.0);
	glLineWidth(1.0);
	
	float BBox[6];
	BBox[0] = minX;
	BBox[1] = maxX;
	BBox[2] = minY;
	BBox[3] = maxY;
	BBox[4] = minZ;
	BBox[5] = maxZ;


	glBegin(GL_LINES);

	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[2], BBox[5]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[2], BBox[4]);



	glVertex3f(BBox[0], BBox[2], BBox[4]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);

	glVertex3f(BBox[1], BBox[2], BBox[5]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[2], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);


	glVertex3f(BBox[0], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[4]);


	glVertex3f(BBox[1], BBox[3], BBox[4]);
	glVertex3f(BBox[1], BBox[3], BBox[5]);

	glVertex3f(BBox[1], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[5]);

	glVertex3f(BBox[0], BBox[3], BBox[5]);
	glVertex3f(BBox[0], BBox[3], BBox[4]);

	glEnd();

}

void DataVolume::drawBacking()
{
	//printf("In Holodeck Draw()\n");
	glColor4f(0.22, 0.25, 0.34, 1.0);

	float BBox[6];
	BBox[0] = minX;
	BBox[1] = maxX;
	BBox[2] = minY;
	BBox[3] = maxY;
	BBox[4] = minZ;
	BBox[5] = maxZ;

	glBegin(GL_QUADS);

	if (orientation == 0)
	{
		glVertex3f(BBox[0], BBox[2], BBox[4]);
		glVertex3f(BBox[1], BBox[2], BBox[4]);
		glVertex3f(BBox[1], BBox[2], BBox[5]);
		glVertex3f(BBox[0], BBox[2], BBox[5]);
	}
	else if (orientation == 1)
	{
		glVertex3f(BBox[0], BBox[2], BBox[5]);
		glVertex3f(BBox[1], BBox[2], BBox[5]);
		glVertex3f(BBox[1], BBox[3], BBox[5]);
		glVertex3f(BBox[0], BBox[3], BBox[5]);
	}

	glEnd();
}

void DataVolume::drawAxes()
{
	//printf("In Holodeck Draw()\n");
	glColor3f(0.75, 0.0, 0.0);
	glLineWidth(1.0);

	float smallSize = sizeX / 40;
	float bigSize = sizeX / 20;

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(posX - smallSize, 0, 0);
	glVertex3f(posX + bigSize, 0, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, posX - smallSize, 0);
	glVertex3f(0, posX + bigSize, 0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, posX - smallSize);
	glVertex3f(0, 0, posX + bigSize);
	glEnd();
}