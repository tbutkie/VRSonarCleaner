#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include "BroadcastSystem.h"
#include "ShaderUtils.h"
#include "Icosphere.h"
#include "GLSLpreamble.h"

#include <shared/glm/glm.hpp>

class LightingSystem
{
public:
	struct BasicLight {
		bool on;
		glm::vec3 color;
		float ambientCoefficient;

		BasicLight()
			: on(false)
			, color(glm::vec3(0.f))
			, ambientCoefficient(0.f)
		{}
	};

	struct DLight : BasicLight {
		glm::vec3 direction;

		DLight()
			: direction(glm::vec3(0.f))
		{}
	};

	struct PLight : BasicLight {
		glm::vec3 position;
		float constant;
		float linear;
		float quadratic;

		PLight()
			: position(glm::vec3(0.f))
			, constant(0.f)
			, linear(0.f)
			, quadratic(0.f)
		{}
	};

	struct SLight : PLight {
		glm::vec3 direction;
		float cutOff;
		float outerCutOff;
		bool attachedToCamera;

		SLight()
			: direction(glm::vec3(0.f))
			, cutOff(0.f)
			, outerCutOff(0.f)
			, attachedToCamera(false)
		{}
	};

	GLuint m_glProgramID;

public:
	LightingSystem();
	~LightingSystem();

	void update(glm::mat4 view);

	DLight* addDirectLight(glm::vec3 direction = glm::vec3(-1.f)
		, glm::vec3 color = glm::vec3(1.f)
		, float ambientCoeff = 0.1f
		);

	PLight* addPointLight(glm::vec3 position = glm::vec3(1.f)
		, glm::vec3 color = glm::vec3(1.f)
		, float ambientCoeff = 0.1f
		, float constant = 1.f
		, float linear = 0.f
		, float quadratic = 1.f
		);

	SLight* addSpotLight(glm::vec3 position = glm::vec3(1.f)
		, glm::vec3 direction = glm::vec3(0.f)
		, glm::vec3 color = glm::vec3(1.f)
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
	DLight m_arrDLights[MAX_DIRECTIONAL_LIGHTS];
	PLight m_arrPLights[MAX_POINT_LIGHTS];
	SLight m_arrSLights[MAX_SPOT_LIGHTS];
	bool m_bDrawLightBulbs;

	int m_nDLights;
	int m_nPLights;
	int m_nSLights;
};

#endif