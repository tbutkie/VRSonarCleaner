#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include "BroadcastSystem.h"
#include "ShaderUtils.h"
#include "Icosphere.h"

#include <shared/glm/glm.hpp>

class LightingSystem
{
public:
	struct BasicLight {
		bool on;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};

	struct DLight : BasicLight {
		glm::vec3 direction;
	};

	struct PLight : BasicLight {
		glm::vec3 position;
		float constant;
		float linear;
		float quadratic;
	};

	struct SLight : PLight {
		glm::vec3 direction;
		float cutOff;
		float outerCutOff;
		bool attachedToCamera;
	};

private:
	std::vector<DLight> dLights;
	std::vector<PLight> pLights;
	std::vector<SLight> sLights;

public:
	LightingSystem();
	~LightingSystem();

	void update(glm::mat4 view);

	DLight* addDirectLight(glm::vec3 direction = glm::vec3(-1.f)
		, glm::vec3 ambient = glm::vec3(0.5f)
		, glm::vec3 diffuse = glm::vec3(1.f)
		, glm::vec3 specular = glm::vec3(0.25f)
		);

	PLight* addPointLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 ambient = glm::vec3(0.05f)
		, glm::vec3 diffuse = glm::vec3(1.f)
		, glm::vec3 specular = glm::vec3(0.25f)
		, float constant = 1.f
		, float linear = 0.09f
		, float quadratic = 0.032f
		);

	SLight* addSpotLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 direction = glm::vec3(0.0f)
		, glm::vec3 ambient = glm::vec3(0.0f)
		, glm::vec3 diffuse = glm::vec3(1.0f)
		, glm::vec3 specular = glm::vec3(0.25f)
		, float constant = 1.0f
		, float linear = 0.09f
		, float quadratic = 0.032f
		, float cutOffDeg = 12.5f
		, float outerCutOffDeg = 15.0f
		, bool attachToCamera = false
		);

	void generateLightingShader();
	
	void showPointLights(bool yesno);
	bool toggleShowPointLights();

	void activateShader();
	void deactivateShader();

private:
	void updateLightingUniforms();

private:
	GLuint m_glProgramID, m_glInstancedProgramID;
	bool m_bRefreshShader, m_bDrawLightBulbs;
};

#endif