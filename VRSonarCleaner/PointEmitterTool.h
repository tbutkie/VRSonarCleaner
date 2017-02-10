#ifndef __PointEmitterTool_h__
#define __PointEmitterTool_h__

#include <windows.h>
#include <GL/glew.h>
#include <vector>

#include "IllustrativeParticleEmitter.h"
#include "CoordinateScaler.h"
#include "ColorsAndSizes.h"

#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/quaternion.hpp>

class PointEmitterTool
{
public:
	PointEmitterTool(glm::vec3 position, CoordinateScaler *scaler);
	virtual ~PointEmitterTool();
	
	void changeColor(int color);
	void changeSpread(float radius);
	void changeRate(float msBetweenParticles);
	void changeLifetime(float lifetime);
	void changeTrailtime(float trailTime);
	void drawSmall3D();

	float getTopActualDepth();
	float getBottomActualDepth();

	void kill();
	bool shouldBeDeleted();

private:
	CoordinateScaler *m_pCoordScaler;
	glm::vec3 m_vec3Position;
	glm::quat m_quatOrientation;

	IllustrativeParticleEmitter* m_pParticleEmitter;

	float m_fVerticalHalfLength;

	bool m_bDeleteMe;
};

#endif