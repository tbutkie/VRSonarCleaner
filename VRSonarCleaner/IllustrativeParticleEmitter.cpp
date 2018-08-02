#include "IllustrativeParticleEmitter.h"
#include <gtc/random.hpp>

using namespace std::chrono_literals;

IllustrativeParticleEmitter::IllustrativeParticleEmitter(float xLoc, float yLoc, float zLoc)
{
	x = xLoc;
	y = yLoc;
	z = zLoc;
	
	color = 0;
	particlesPerSecond = DEFAULT_DYE_RATE;
	radius = DEFAULT_DYE_RADIUS;
	m_msLifetime = DEFAULT_DYE_LIFETIME;
	m_msTrailTime = DEFAULT_DYE_LENGTH;
	gravity = DEFAULT_GRAVITY;
	m_tpLastEmission = std::chrono::time_point<std::chrono::high_resolution_clock>();
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

void IllustrativeParticleEmitter::setLifetime(std::chrono::milliseconds time)
{
	m_msLifetime = time;
}

std::chrono::milliseconds IllustrativeParticleEmitter::getLifetime()
{
	return m_msLifetime;
}

void IllustrativeParticleEmitter::setRate(float ParticlesPerSecond)
{
	particlesPerSecond = ParticlesPerSecond;
}

void IllustrativeParticleEmitter::setTrailTime(std::chrono::milliseconds time)
{
	m_msTrailTime = time;
}

std::chrono::milliseconds IllustrativeParticleEmitter::getTrailTime()
{
	return m_msTrailTime;
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

int IllustrativeParticleEmitter::getNumParticlesToEmit(std::chrono::time_point<std::chrono::high_resolution_clock> tick)
{
	//if (m_tpLastEmission == std::chrono::time_point<std::chrono::high_resolution_clock>())
	//{
	//	m_tpLastEmission = tick - particlesPerSecond * 0.01f;
	//}

	std::chrono::milliseconds timeSinceLast = std::chrono::duration_cast<std::chrono::milliseconds>(tick - m_tpLastEmission);
	if (timeSinceLast > 0ms) //only spawn 10 times per second
	{
		int toEmit = static_cast<int>(floor(std::chrono::duration<float>(timeSinceLast).count() * particlesPerSecond / 1000.f));
		if (toEmit > 1000) //sanity check for times where there is too long between spawnings
		{
			m_tpLastEmission = tick;
			return 1000;
		}
		else
		{
			if (toEmit > 0)
				m_tpLastEmission = tick;

			return toEmit;
		}
	}
	else
	{
		return 0;
	}
}
std::vector<glm::vec3> IllustrativeParticleEmitter::getParticlesToEmit(int number) 
{
	std::vector<glm::vec3> verts;
	for (int i = 0; i < number; ++i)
	{
		verts.push_back(glm::vec3(x, y, z));

		if (i > 0)
		{
			verts.back() += glm::sphericalRand(radius);
		}
	}
	return verts;
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
	else
		return glm::vec3(1.f);
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