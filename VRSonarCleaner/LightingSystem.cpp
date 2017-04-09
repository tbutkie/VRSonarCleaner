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
	, m_nLights(0)
{
	//glCreateBuffers(1, &m_glLightingUBO);
	//glNamedBufferData(m_glLightingUBO, sizeof(FrameUniforms), NULL, GL_STATIC_DRAW); // allocate memory
	//glBindBufferRange(GL_UNIFORM_BUFFER, SCENE_UNIFORM_BUFFER_LOCATION, m_glLightingUBO, 0, sizeof(FrameUniforms));
}

LightingSystem::~LightingSystem()
{
}

// Uses the current shader
void LightingSystem::update(glm::mat4 view)
{
	glm::vec3 black(0.f);

	glUniform1i(LIGHT_COUNT_UNIFORM_LOCATION, m_nLights);

	// Directional light
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		std::string name = "lights[" + std::to_string(i);
		name += "]";

		{
			glUniform4fv(glGetUniformLocation(m_glProgramID, (name + ".color").c_str()), 1, glm::value_ptr(m_arrLights[i].color));
			glUniform4fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(view * m_arrLights[i].position));
			glUniform4fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(view * m_arrLights[i].direction)));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".ambientCoeff").c_str()), m_arrLights[i].ambientCoefficient);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".constant").c_str()), m_arrLights[i].constant);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".linear").c_str()), m_arrLights[i].linear);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".quadratic").c_str()), m_arrLights[i].quadratic);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".cutOff").c_str()), m_arrLights[i].cutOff);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".outerCutOff").c_str()), m_arrLights[i].outerCutOff);
		}
	}
}

LightingSystem::Light* LightingSystem::addDirectLight(glm::vec4 direction, glm::vec4 color, float ambientCoeff)
{
	if (m_nLights == MAX_LIGHTS)
		return nullptr;

	m_arrLights[m_nLights].direction = direction;
	m_arrLights[m_nLights].color = color;
	m_arrLights[m_nLights].ambientCoefficient = ambientCoeff;

	return &m_arrLights[m_nLights++];
}

LightingSystem::Light* LightingSystem::addPointLight(glm::vec4 position, glm::vec4 color, float ambientCoeff, float constant, float linear, float quadratic)
{
	if (m_nLights == MAX_LIGHTS)
		return nullptr;

	m_arrLights[m_nLights].position = position;
	m_arrLights[m_nLights].color = color;
	m_arrLights[m_nLights].ambientCoefficient = ambientCoeff;
	m_arrLights[m_nLights].constant = constant;
	m_arrLights[m_nLights].linear = linear;
	m_arrLights[m_nLights].quadratic = quadratic;

	return &m_arrLights[m_nLights++];
}

LightingSystem::Light* LightingSystem::addSpotLight(glm::vec4 position, glm::vec4 direction, glm::vec4 color, float ambientCoeff, float constant, float linear, float quadratic, float cutOffDeg, float outerCutOffDeg, bool attachToCamera)
{
	if (m_nLights == MAX_LIGHTS)
		return nullptr;

	m_arrLights[m_nLights].position = position;
	m_arrLights[m_nLights].direction = direction;
	m_arrLights[m_nLights].color = color;
	m_arrLights[m_nLights].ambientCoefficient = ambientCoeff;
	m_arrLights[m_nLights].constant = constant;
	m_arrLights[m_nLights].linear = linear;
	m_arrLights[m_nLights].quadratic = quadratic;
	m_arrLights[m_nLights].cutOff = glm::cos(glm::radians(cutOffDeg));
	m_arrLights[m_nLights].outerCutOff = glm::cos(glm::radians(outerCutOffDeg));

	return &m_arrLights[m_nLights++];
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