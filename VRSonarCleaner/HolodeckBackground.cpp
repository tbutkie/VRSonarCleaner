#include "HolodeckBackground.h"

HolodeckBackground::HolodeckBackground(float SizeX, float SizeY, float SizeZ, float Spacing)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	spacing = Spacing;

	spacesX = floor(sizeX/spacing);
	spacesY = floor(sizeY/spacing);
	spacesZ = floor(sizeZ/spacing);

	spacingX = sizeX / spacesX;
	spacingY = sizeY / spacesY;
	spacingZ = sizeZ / spacesZ;

	minX = -sizeX/2;
	minY = 0 ;
	minZ = -sizeZ / 2;

	maxX = (sizeX / 2);
	maxY = sizeY;
	maxZ = (sizeZ / 2);

}

HolodeckBackground::~HolodeckBackground()
{

}

void HolodeckBackground::draw()
{
	//printf("In Holodeck Draw()\n");
	glColor3f(0.0, 1.0, 1.0);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	glVertex3f(0, 0, 0);
	glEnd();

	
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(-0.25, 0, 0);
		glVertex3f(0.5, 0, 0);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0, -0.25, 0);
		glVertex3f(0, 0.5, 0);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0, 0, -0.25);
		glVertex3f(0, 0, 0.5);
	glEnd();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 0.0, 0.45);
	glLineWidth(2.0);
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINES);

	float r = 1.0;
	float g = 1.0;
	float b = 0.0;
	float floorOpacity = 0.30;
	float ceilingOpacity = 0.03;
	float inBetweenOpacity; 
	
	for (float x = minX; x <= maxX ; x += spacingX)
	{
		glColor4f(r, g, b, floorOpacity);
		glVertex3f(x, minY, minZ);
		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(x, maxY, minZ);

		glColor4f(r, g, b, floorOpacity);
		glVertex3f(x, minY, maxZ);
		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(x, maxY, maxZ);

		glColor4f(r, g, b, floorOpacity);
		glVertex3f(x, minY, minZ);
		glVertex3f(x, minY, maxZ);

		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(x, maxY, minZ);
		glVertex3f(x, maxY, maxZ);
	}

	for (float y = minY; y <= maxY; y += spacingY)
	{
		inBetweenOpacity = ((1-(y / maxY))*(floorOpacity - ceilingOpacity)) + ceilingOpacity;
		glColor4f(r, g, b, inBetweenOpacity);
		glVertex3f(minX, y, minZ);
		glVertex3f(maxX, y, minZ);

		glVertex3f(minX, y, maxZ);
		glVertex3f(maxX, y, maxZ);
		
		glVertex3f(minX, y, minZ);
		glVertex3f(minX, y, maxZ);

		glVertex3f(maxX, y, minZ);
		glVertex3f(maxX, y, maxZ);
	}

	for (float z = minZ; z <= maxZ; z += spacingZ)
	{
		glColor4f(r, g, b, floorOpacity);
		glVertex3f(minX, minY, z);
		glVertex3f(maxX, minY, z);

		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(minX, maxY, z);
		glVertex3f(maxX, maxY, z);

		glColor4f(r, g, b, floorOpacity);
		glVertex3f(minX, minY, z);
		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(minX, maxY, z);

		glColor4f(r, g, b, floorOpacity);
		glVertex3f(maxX, minY, z);
		glColor4f(r, g, b, ceilingOpacity);
		glVertex3f(maxX, maxY, z);
	}

	glEnd();

	glColor4f(1.0, 1.0, 1.0, 1.0);

	//std::vector<float> vertdataarray;

	//
	//vertdataarray.push_back(-0.5);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(1);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(0.5);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(1);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);


	//vertdataarray.push_back(0);
	//vertdataarray.push_back(-0.5);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(1);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0.5);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(1);
	//vertdataarray.push_back(0);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(-0.5);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(1);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0.5);

	//vertdataarray.push_back(0);
	//vertdataarray.push_back(0);
	//vertdataarray.push_back(1);

	//int vertcount = 6;

	//// Setup the VAO the first time through.
	//if (m_unControllerVAO == 0)
	//{
	//	glGenVertexArrays(1, &m_unControllerVAO);
	//	glBindVertexArray(m_unControllerVAO);

	//	glGenBuffers(1, &m_glControllerVertBuffer);
	//	glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	//	GLuint stride = 2 * 3 * sizeof(float);
	//	GLuint offset = 0;

	//	glEnableVertexAttribArray(0);
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	//	offset += (sizeof(float)*3);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	//	glBindVertexArray(0);
	//}

	//glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	//// set vertex data if we have some
	//if (vertdataarray.size() > 0)
	//{
	//	//$ TODO: Use glBufferSubData for this...
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	//}

	//// draw the controller axis lines
	////glUseProgram(m_unControllerTransformProgramID);
	////glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix(nEye).get());
	//glBindVertexArray(m_unControllerVAO);
	//glDrawArrays(GL_LINES, 0, vertcount);
	//glBindVertexArray(0);


}