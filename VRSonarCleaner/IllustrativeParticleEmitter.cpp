#include "IllustrativeParticleEmitter.h"

#include "DebugDrawer.h"

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
}

void IllustrativeParticleEmitter::drawRoundedGlyph(float X, float Y, float Width, float Height, float roundingHeight, bool selected)
{
}

glm::vec3 IllustrativeParticleEmitter::getColor()
{
	if (color == 0)
		return glm::vec3(COLOR_0_R, COLOR_0_G, COLOR_0_B);
	else if (color == 1)
		return glm::vec3(COLOR_1_R, COLOR_1_G, COLOR_1_B);
	else if (color == 2)
		return glm::vec3(COLOR_2_R, COLOR_2_G, COLOR_2_B);
	else if (color == 3)
		return glm::vec3(COLOR_3_R, COLOR_3_G, COLOR_3_B);
	else if (color == 4)
		return glm::vec3(COLOR_4_R, COLOR_4_G, COLOR_4_B);
	else if (color == 5)
		return glm::vec3(COLOR_5_R, COLOR_5_G, COLOR_5_B);
	else if (color == 6)
		return glm::vec3(COLOR_6_R, COLOR_6_G, COLOR_6_B);
	else if (color == 7)
		return glm::vec3(COLOR_7_R, COLOR_7_G, COLOR_7_B);
	else if (color == 8)
		return glm::vec3(COLOR_8_R, COLOR_8_G, COLOR_8_B);
}

glm::vec3 IllustrativeParticleEmitter::getMutedColor()
{
	if (color == 0)
		return glm::vec3(COLOR_0_R, COLOR_0_G, COLOR_0_B) - 0.35f;
	else if (color == 1)
		return glm::vec3(COLOR_1_R, COLOR_1_G, COLOR_1_B) - 0.35f;
	else if (color == 2)
		return glm::vec3(COLOR_2_R, COLOR_2_G, COLOR_2_B) - 0.35f;
	else if (color == 3)
		return glm::vec3(COLOR_3_R, COLOR_3_G, COLOR_3_B) - 0.35f;
	else if (color == 4)
		return glm::vec3(COLOR_4_R, COLOR_4_G, COLOR_4_B) - 0.35f;
	else if (color == 5)
		return glm::vec3(COLOR_5_R, COLOR_5_G, COLOR_5_B) - 0.35f;
	else if (color == 6)
		return glm::vec3(COLOR_6_R, COLOR_6_G, COLOR_6_B) - 0.35f;
	else if (color == 7)
		return glm::vec3(COLOR_7_R, COLOR_7_G, COLOR_7_B) - 0.35f;
	else if (color == 8)
		return glm::vec3(COLOR_8_R, COLOR_8_G, COLOR_8_B) - 0.35f;
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
	DebugDrawer::getInstance().drawLine(
		glm::vec3(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthTop)),
		glm::vec3(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthBottom)),
		glm::vec4(getColor(), 1.f)
	);
	glEnd();

	int nSegments = 16;

	for (int i = 0; i < nSegments; ++i)
	{
		float theta0 = glm::two_pi<float>() * static_cast<float>(i) / (static_cast<float>(nSegments - 1));
		float theta1 = glm::two_pi<float>() * static_cast<float>((i + 1) % nSegments) / (static_cast<float>(nSegments - 1));

		DebugDrawer::getInstance().drawLine(
			glm::vec3(scaler->getScaledLonX(x) + cos(theta0)*scaler->getScaledLength(radius), scaler->getScaledLatY(y) + sin(theta0)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthBottom)),
			glm::vec3(scaler->getScaledLonX(x) + cos(theta1)*scaler->getScaledLength(radius), scaler->getScaledLatY(y) + sin(theta1)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthBottom)),
			glm::vec4(getColor(), 1.f)
		);
	}

	for (int i = 0; i < nSegments; ++i)
	{
		float theta0 = glm::two_pi<float>() * static_cast<float>(i) / (static_cast<float>(nSegments - 1));
		float theta1 = glm::two_pi<float>() * static_cast<float>((i + 1) % nSegments) / (static_cast<float>(nSegments - 1));
		
		DebugDrawer::getInstance().drawLine(
			glm::vec3(scaler->getScaledLonX(x) + cos(theta0)*scaler->getScaledLength(radius), scaler->getScaledLatY(y) + sin(theta0)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthTop)),
			glm::vec3(scaler->getScaledLonX(x) + cos(theta1)*scaler->getScaledLength(radius), scaler->getScaledLatY(y) + sin(theta1)*scaler->getScaledLength(radius), scaler->getScaledDepth(depthTop)),
			glm::vec4(getColor(), 1.f)
		);
	}

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