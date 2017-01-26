#include "IllustrativeParticleEmitter.h"

IllustrativeParticleEmitter::IllustrativeParticleEmitter(float xLoc, float yLoc, float DepthBottom, float DepthTop, CoordinateScaler *Scaler)
{
	x = xLoc;
	y = yLoc;
	depthBottom = DepthBottom;
	depthTop = DepthTop;
	
	color = 0;
	particlesPerSecond = DEFAULT_DYE_RATE;
	radius = DEFAULT_DYE_RADIUS;
	lifetime = DEFAULT_DYE_LIFETIME;
	trailTime = DEFAULT_DYE_LENGTH;
	gravity = DEFAULT_GRAVITY;
	lastEmission = 0;

	scaler = Scaler;
}

IllustrativeParticleEmitter::~IllustrativeParticleEmitter()
{

}

void IllustrativeParticleEmitter::changeColor(int Color)
{
	color = Color;
}
void IllustrativeParticleEmitter::incrementColor()
{
	color++;
	if (color > 8)
		color = 0;
}
void IllustrativeParticleEmitter::decrementColor()
{
	color--;
	if (color < 0)
		color = 8;
}

void IllustrativeParticleEmitter::changeSpread(float Radius)
{
	radius = Radius;
}

void IllustrativeParticleEmitter::setLifetime(float time)
{
	lifetime = time;
}

int IllustrativeParticleEmitter::getColor()
{
	return color;
}

float IllustrativeParticleEmitter::getLifetime()
{
	return lifetime;
}

void IllustrativeParticleEmitter::setRate(float ParticlesPerSecond)
{
	particlesPerSecond = ParticlesPerSecond;
}

void IllustrativeParticleEmitter::setTrailTime(float time)
{
	trailTime = time;
}

float IllustrativeParticleEmitter::getTrailTime()
{
	return trailTime;
}

float IllustrativeParticleEmitter::getGravity()
{
	return gravity;
}

void IllustrativeParticleEmitter::setGravity(float Gravity)
{
	gravity = Gravity;
}


float IllustrativeParticleEmitter::getRate()
{
	return particlesPerSecond;
}

float IllustrativeParticleEmitter::getRadius()
{
	return radius;
}
void IllustrativeParticleEmitter::setRadius(float rad)
{
	radius = rad;
}

int IllustrativeParticleEmitter::getNumParticlesToEmit(float tickCount)
{
	float timeSinceLast = tickCount-lastEmission;
	if (timeSinceLast > 100) //only spawn 10 times per second
	{
		int toEmit = (timeSinceLast/1000)*particlesPerSecond;
		lastEmission = tickCount;
		if (toEmit > 1000) //sanity check for times where there is too long between spawnings
			return 1000;
		else
			return toEmit;
	}
	else
	{
		return 0;
	}
}
float* IllustrativeParticleEmitter::getParticlesToEmit(int number) 
{
	float randAngle;
	float randDist;
	float randZ;
	float topScaledZ = fabsf(scaler->getScaledDepth(depthTop));
	float bottomScaledZ = fabsf(scaler->getScaledDepth(depthBottom));
	float scaledZRange = fabsf(bottomScaledZ-topScaledZ);
	//float zRange = depthBottom - depthTop;
	float* verts = new float[3*number];
	for (int i=0;i<number;i++)
	{
		randAngle = rand()%100;
		randAngle = (randAngle/100)*6.28318; //2pi
		randDist = rand()%100;
		randDist = (randDist/100)*radius;
		verts[i*3] = x + (randDist*cos(randAngle));
		verts[i*3+1] = y + (randDist*sin(randAngle));
		//randZ = rand()%100;
		//randZ = (randZ/100)*zRange;
		randZ = scaler->getUnscaledDepth( -1.0*  (((((float)(rand()%10000))/10000)*scaledZRange)+topScaledZ)  );
		verts[i*3+2] = randZ;
	}
	return verts;
}


void IllustrativeParticleEmitter::drawGlyph(float X, float Y, float Width, float Height)
{
/*
	//printf("dG %f, %f, %f, %f\n", X, Y, Width, Height);
	glColor3f(PANEL_EDGE_COLOR);
		
	glBegin(GL_QUADS);
		glVertex2f(X,Y);
		glVertex2f(X,Y+Height);
		glVertex2f(X+Width,Y+Height);
		glVertex2f(X+Width,Y);
	glEnd();

	if (color == 0)
		glColor3f(COLOR_0_R, COLOR_0_G, COLOR_0_B);
	else if (color == 1)
		glColor3f(COLOR_1_R, COLOR_1_G, COLOR_1_B);
	else if (color == 2)
		glColor3f(COLOR_2_R, COLOR_2_G, COLOR_2_B);
	else if (color == 3)
		glColor3f(COLOR_3_R, COLOR_3_G, COLOR_3_B);
	else if (color == 4)
		glColor3f(COLOR_4_R, COLOR_4_G, COLOR_4_B);
	else if (color == 5)
		glColor3f(COLOR_5_R, COLOR_5_G, COLOR_5_B);
	else if (color == 6)
		glColor3f(COLOR_6_R, COLOR_6_G, COLOR_6_B);
	else if (color == 7)
		glColor3f(COLOR_7_R, COLOR_7_G, COLOR_7_B);
	else if (color == 8)
		glColor3f(COLOR_8_R, COLOR_8_G, COLOR_8_B);

	glBegin(GL_QUADS);
		glVertex2f(X+1,Y+1);
		glVertex2f(X+1,Y+Height-1);
		glVertex2f(X+Width-1,Y+Height-1);
		glVertex2f(X+Width-1,Y+1);
	glEnd();

	lastGlyphX = X;
	lastGlyphY = Y;
	lastGlyphWidth = Width;
	lastGlyphHeight = Height;
*/
}

void IllustrativeParticleEmitter::drawRoundedGlyph(float X, float Y, float Width, float Height, float roundingHeight, bool selected)
{
	/*
	glColor3f(0,0,0);

	drawHalfEllipse(X,Y,Width, roundingHeight, false, 24);
	glBegin(GL_QUADS);
		glVertex2f(X,Y);
		glVertex2f(X,Y+Height);
		glVertex2f(X+Width,Y+Height);
		glVertex2f(X+Width,Y);
	glEnd();

	if (!selected)
		setMutedColor();
	else
		setColor();

	drawHalfEllipse(X+1,Y,Width-2, roundingHeight-1, false, 24);
	glBegin(GL_QUADS);
		glVertex2f(X+1,Y);
		glVertex2f(X+1,Y+Height);
		glVertex2f(X+Width-1,Y+Height);
		glVertex2f(X+Width-1,Y);
	glEnd();

	glColor3f(0,0,0);
	drawHalfEllipse(X,Y+Height,Width, roundingHeight, true, 24);
	drawHalfEllipse(X,Y+Height,Width, roundingHeight, false, 24);

	if (!selected)
		setMutedColor();
	else
		setColor();

	drawHalfEllipse(X+1,Y+Height,Width-2, roundingHeight-1, true, 24);
	drawHalfEllipse(X+1,Y+Height,Width-2, roundingHeight-1, false, 24);

	lastGlyphX = X;
	lastGlyphY = Y;
	lastGlyphWidth = Width;
	lastGlyphHeight = Height;
	*/
}

void IllustrativeParticleEmitter::setColor()
{
	if (color == 0)
		glColor3f(COLOR_0_R, COLOR_0_G, COLOR_0_B);
	else if (color == 1)
		glColor3f(COLOR_1_R, COLOR_1_G, COLOR_1_B);
	else if (color == 2)
		glColor3f(COLOR_2_R, COLOR_2_G, COLOR_2_B);
	else if (color == 3)
		glColor3f(COLOR_3_R, COLOR_3_G, COLOR_3_B);
	else if (color == 4)
		glColor3f(COLOR_4_R, COLOR_4_G, COLOR_4_B);
	else if (color == 5)
		glColor3f(COLOR_5_R, COLOR_5_G, COLOR_5_B);
	else if (color == 6)
		glColor3f(COLOR_6_R, COLOR_6_G, COLOR_6_B);
	else if (color == 7)
		glColor3f(COLOR_7_R, COLOR_7_G, COLOR_7_B);
	else if (color == 8)
		glColor3f(COLOR_8_R, COLOR_8_G, COLOR_8_B);
}

void IllustrativeParticleEmitter::setMutedColor()
{
	if (color == 0)
		glColor3f(COLOR_0_R-0.35, COLOR_0_G-0.35, COLOR_0_B-0.35);
	else if (color == 1)
		glColor3f(COLOR_1_R-0.35, COLOR_1_G-0.35, COLOR_1_B-0.35);
	else if (color == 2)
		glColor3f(COLOR_2_R-0.35, COLOR_2_G-0.35, COLOR_2_B-0.35);
	else if (color == 3)
		glColor3f(COLOR_3_R-0.35, COLOR_3_G-0.35, COLOR_3_B-0.35);
	else if (color == 4)
		glColor3f(COLOR_4_R-0.35, COLOR_4_G-0.35, COLOR_4_B-0.35);
	else if (color == 5)
		glColor3f(COLOR_5_R-0.35, COLOR_5_G-0.35, COLOR_5_B-0.35);
	else if (color == 6)
		glColor3f(COLOR_6_R-0.35, COLOR_6_G-0.35, COLOR_6_B-0.35);
	else if (color == 7)
		glColor3f(COLOR_7_R-0.35, COLOR_7_G-0.35, COLOR_7_B-0.35);
	else if (color == 8)
		glColor3f(COLOR_8_R-0.35, COLOR_8_G-0.35, COLOR_8_B-0.35);
}

bool IllustrativeParticleEmitter::isPointInGlyph(float X, float Y)
{
	if (X >= lastGlyphX && Y >= lastGlyphY && X <= lastGlyphX + lastGlyphWidth && Y <= lastGlyphY + lastGlyphHeight)
		return true;
	else
		return false;
}


void IllustrativeParticleEmitter::drawSmall3D() ///need to swap zy before using in vr
{
	glLineWidth(3);
	//glColor3f(DYE_POLE_COLOR);
	setColor();
	glBegin(GL_LINES);
		glVertex3f(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthTop));
		glVertex3f(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthBottom));
	glEnd();

	glBegin(GL_LINE_LOOP);
		for (float i=0;i<6.28318;i+=0.1)
		{
			glVertex3f(scaler->getScaledLonX(x)+cos(i)*scaler->getScaledLength(radius), scaler->getScaledLatY(y)+sin(i)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthBottom));
		}
	glEnd();

	glBegin(GL_LINE_LOOP);
		for (float i=0;i<6.28318;i+=0.1)
		{
			glVertex3f(scaler->getScaledLonX(x)+cos(i)*scaler->getScaledLength(radius), scaler->getScaledLatY(y)+sin(i)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthTop));
		}
	glEnd();

}


void IllustrativeParticleEmitter::setBottom(float DepthBottom)
{
	depthBottom = DepthBottom;
}

void IllustrativeParticleEmitter::setTop(float DepthTop)
{
	depthTop = DepthTop;
}

float IllustrativeParticleEmitter::getBottom()
{
	return depthBottom;
}

float IllustrativeParticleEmitter::getTop()
{
	return depthTop;
}