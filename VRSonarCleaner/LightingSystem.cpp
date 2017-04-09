#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include "LightingSystem.h"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif // !GLEW_STATIC
#include <GL/glew.h>

#include <shared/glm/gtc/matrix_transform.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include <string>

LightingSystem::LightingSystem()
	: m_bDrawLightBulbs(true)
	, m_glLightingUBO(0)
	, m_nDLights(0)
	, m_nPLights(0)
	, m_nSLights(0)
{
}

LightingSystem::~LightingSystem()
{
}

// Uses the current shader
void LightingSystem::update(glm::mat4 view)
{
	glm::vec3 black(0.f);

	glUniform1i(DIR_LIGHTS_COUNT_UNIFORM_LOCATION, m_nDLights);
	glUniform1i(POINT_LIGHTS_COUNT_UNIFORM_LOCATION, m_nPLights);
	glUniform1i(SPOT_LIGHTS_COUNT_UNIFORM_LOCATION, m_nSLights);

	// Directional light
	for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		std::string name = "dirLights[" + std::to_string(i);
		name += "]";

		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(glm::mat3(view) * m_arrDLights[i].direction)));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".color").c_str()), 1, glm::value_ptr(m_arrDLights[i].color));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".ambientCoeff").c_str()), m_arrDLights[i].ambientCoefficient);
		}
	}

	// Point light
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		std::string name = "pointLights[" + std::to_string(i);
		name += "]";

		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(m_arrPLights[i].position, 1.f))));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".color").c_str()), 1, glm::value_ptr(m_arrPLights[i].color));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".ambientCoeff").c_str()), m_arrPLights[i].ambientCoefficient);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".constant").c_str()), m_arrPLights[i].constant);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".linear").c_str()), m_arrPLights[i].linear);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".quadratic").c_str()), m_arrPLights[i].quadratic);
		}
	}

	// SpotLight
	for (int i = 0; i < MAX_SPOT_LIGHTS; ++i)
	{
		std::string name = "spotLights[" + std::to_string(i);
		name += "]";

		{
			if (m_arrSLights[i].attachedToCamera)
			{
				m_arrSLights[i].position = glm::vec3(0.f);
				m_arrSLights[i].direction = glm::vec3(0.f, 0.f, -1.f);
			}
			else
			{
				glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(m_arrSLights[i].position, 1.f))));
				glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(glm::vec3(view * glm::vec4(m_arrSLights[i].direction, 0.f)))));
			}

			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".color").c_str()), 1, glm::value_ptr(m_arrSLights[i].color));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".ambientCoeff").c_str()), m_arrSLights[i].ambientCoefficient);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".constant").c_str()), m_arrSLights[i].constant);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".linear").c_str()), m_arrSLights[i].linear);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".quadratic").c_str()), m_arrSLights[i].quadratic);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".cutOff").c_str()), m_arrSLights[i].cutOff);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".outerCutOff").c_str()), m_arrSLights[i].outerCutOff);
		}
	}
}

LightingSystem::DLight* LightingSystem::addDirectLight(glm::vec3 direction, glm::vec3 color, float ambientCoeff)
{
	if (m_nDLights == MAX_DIRECTIONAL_LIGHTS)
		return nullptr;

	m_arrDLights[m_nDLights].direction = direction;
	m_arrDLights[m_nDLights].color = color;
	m_arrDLights[m_nDLights].ambientCoefficient = ambientCoeff;
	m_arrDLights[m_nDLights].on = true;

	return &m_arrDLights[m_nDLights++];
}

LightingSystem::PLight* LightingSystem::addPointLight(glm::vec3 position, glm::vec3 color, float ambientCoeff, float constant, float linear, float quadratic)
{
	if (m_nPLights == MAX_POINT_LIGHTS)
		return nullptr;

	m_arrPLights[m_nPLights].position = position;
	m_arrPLights[m_nPLights].color = color;
	m_arrPLights[m_nPLights].ambientCoefficient = ambientCoeff;
	m_arrPLights[m_nPLights].constant = constant;
	m_arrPLights[m_nPLights].linear = linear;
	m_arrPLights[m_nPLights].quadratic = quadratic;
	m_arrPLights[m_nPLights].on = true;

	return &m_arrPLights[m_nPLights++];
}

LightingSystem::SLight* LightingSystem::addSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float ambientCoeff, float constant, float linear, float quadratic, float cutOffDeg, float outerCutOffDeg, bool attachToCamera)
{
	if (m_nSLights == MAX_SPOT_LIGHTS)
		return nullptr;

	m_arrSLights[m_nSLights].position = position;
	m_arrSLights[m_nSLights].direction = direction;
	m_arrSLights[m_nSLights].color = color;
	m_arrSLights[m_nSLights].ambientCoefficient = ambientCoeff;
	m_arrSLights[m_nSLights].constant = constant;
	m_arrSLights[m_nSLights].linear = linear;
	m_arrSLights[m_nSLights].quadratic = quadratic;
	m_arrSLights[m_nSLights].cutOff = glm::cos(glm::radians(cutOffDeg));
	m_arrSLights[m_nSLights].outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
	m_arrSLights[m_nSLights].attachedToCamera = attachToCamera;
	m_arrSLights[m_nSLights].on = true;

	return &m_arrSLights[m_nSLights++];
}

void LightingSystem::showPointLights(bool yesno)
{
	m_bDrawLightBulbs = yesno;
}

bool LightingSystem::toggleShowPointLights()
{
	m_bDrawLightBulbs = !m_bDrawLightBulbs;
	return m_bDrawLightBulbs;
}

#endif