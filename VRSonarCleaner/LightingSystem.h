#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include "BroadcastSystem.h"
#include "Icosphere.h"
#include "GLSLpreamble.h"

#include <shared/glm/glm.hpp>
#include <GL\glew.h>

class LightingSystem
{
public:
	struct Light {
		glm::vec4 color;
		glm::vec4 position;
		glm::vec4 direction;
		float ambientCoefficient;
		float constant;
		float linear;
		float quadratic;
		float cutOff;
		float outerCutOff;

		Light()
			: color(glm::vec4(0.f))
			, position(glm::vec4(0.f))
			, direction(glm::vec4(0.f))
			, ambientCoefficient(0.f)
			, constant(0.f)
			, linear(0.f)
			, quadratic(0.f)
			, cutOff(0.f)
			, outerCutOff(0.f)
		{}
	};

	GLuint m_glProgramID;

public:
	LightingSystem();
	~LightingSystem();

	void update(glm::mat4 view);

	Light* addDirectLight(glm::vec4 direction = glm::vec4(-1.f, -1.f, -1.f, 0.f)
		, glm::vec4 color = glm::vec4(1.f)
		, float ambientCoeff = 0.1f
		);

	Light* addPointLight(glm::vec4 position = glm::vec4(1.f)
		, glm::vec4 color = glm::vec4(1.f)
		, float ambientCoeff = 0.1f
		, float constant = 1.f
		, float linear = 0.f
		, float quadratic = 1.f
		);

	Light* addSpotLight(glm::vec4 position = glm::vec4(1.f)
		, glm::vec4 direction = glm::vec4(-1.f, -1.f, -1.f, 0.f)
		, glm::vec4 color = glm::vec4(1.f)
		, float ambientCoeff = 0.1f
		, float constant = 1.f
		, float linear = 0.f
		, float quadratic = 1.f
		, float cutOffDeg = 12.5f
		, float outerCutOffDeg = 15.f
		, bool attachToCamera = false
		);
	
	void showPointLights(bool yesno);
	bool toggleShowPointLights();

private:
	Light m_arrLights[MAX_LIGHTS];
	bool m_bDrawLightBulbs;

	GLuint m_glLightingUBO;

	int m_nLights;
};

#endif