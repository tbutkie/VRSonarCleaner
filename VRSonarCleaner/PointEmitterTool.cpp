#include "PointEmitterTool.h"

PointEmitterTool::PointEmitterTool(glm::vec3 position, CoordinateScaler *scaler)
	: m_vec3Position(position)
	, m_pCoordScaler(scaler)
	, m_pParticleEmitter(NULL)
	, m_fVerticalHalfLength(0.1f)
	, m_bDeleteMe(false)
{
	m_pParticleEmitter = new IllustrativeParticleEmitter(
		m_vec3Position.x, 
		m_vec3Position.y,
		m_vec3Position.z - m_fVerticalHalfLength,
		m_vec3Position.z + m_fVerticalHalfLength,
		m_pCoordScaler);
}

PointEmitterTool::~PointEmitterTool()
{
	if (m_pParticleEmitter)
		delete m_pParticleEmitter;
}


void PointEmitterTool::changeColor(int color)
{
	m_pParticleEmitter->changeColor(color);
}

void PointEmitterTool::changeSpread(float radius)
{
	m_pParticleEmitter->changeSpread(radius);
}

void PointEmitterTool::changeRate(float msBetweenParticles)
{
	m_pParticleEmitter->setRate(msBetweenParticles);
}

void PointEmitterTool::changeLifetime(float lifetime)
{
	m_pParticleEmitter->setLifetime(lifetime);
}

void PointEmitterTool::changeTrailtime(float trailTime)
{
	m_pParticleEmitter->setTrailTime(trailTime);
}

void PointEmitterTool::drawSmall3D()
{
}

float PointEmitterTool::getBottomActualDepth()
{
	return m_vec3Position.z - m_fVerticalHalfLength;
}

float PointEmitterTool::getTopActualDepth()
{
	return m_vec3Position.z + m_fVerticalHalfLength;
}

void PointEmitterTool::kill()
{
	m_bDeleteMe = true;
}

bool PointEmitterTool::shouldBeDeleted()
{
	return m_bDeleteMe;
}