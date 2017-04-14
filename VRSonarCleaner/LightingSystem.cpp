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
	: m_glLightingUBO(0)
	, m_nLights(0)
{
	//glCreateBuffers(1, &m_glLightingUBO);
	//glNamedBufferData(m_glLightingUBO, sizeof(FrameUniforms), NULL, GL_STATIC_DRAW); // allocate memory
	//glBindBufferRange(GL_UNIFORM_BUFFER, SCENE_UNIFORM_BUFFER_LOCATION, m_glLightingUBO, 0, sizeof(FrameUniforms));
}

LightingSystem::~LightingSystem()
{
}

void LightingSystem::addShaderToUpdate(GLuint * shader)
{
	m_vpShadersWithLighting.push_back(shader);
}

// Uses the current shader
void LightingSystem::update(glm::mat4 view)
{
	glm::vec3 black(0.f);

	for (auto pShader : m_vpShadersWithLighting)
	{
		if (*pShader)
		{
			glUseProgram(*pShader);

			glUniform1i(LIGHT_COUNT_UNIFORM_LOCATION, m_nLights);

			for (int i = 0; i < m_nLights; ++i)
			{
				std::string name = "lights[" + std::to_string(i);
				name += "]";

				glUniform4fv(glGetUniformLocation(*pShader, (name + ".color").c_str()), 1, glm::value_ptr(m_arrLights[i].color));
				glUniform4fv(glGetUniformLocation(*pShader, (name + ".position").c_str()), 1, glm::value_ptr(view * m_arrLights[i].position));
				glUniform4fv(glGetUniformLocation(*pShader, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(view * m_arrLights[i].direction)));
				glUniform1f(glGetUniformLocation(*pShader, (name + ".ambientCoeff").c_str()), m_arrLights[i].ambientCoefficient);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".constant").c_str()), m_arrLights[i].constant);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".linear").c_str()), m_arrLights[i].linear);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".quadratic").c_str()), m_arrLights[i].quadratic);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".cutOff").c_str()), m_arrLights[i].cutOff);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".outerCutOff").c_str()), m_arrLights[i].outerCutOff);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".isOn").c_str()), m_arrLights[i].isOn);
				glUniform1f(glGetUniformLocation(*pShader, (name + ".isSpotLight").c_str()), m_arrLights[i].isSpotLight);
			}
		}
	}

	glUseProgram(0);
}

LightingSystem::Light* LightingSystem::addDirectLight(glm::vec4 direction, glm::vec4 color, float ambientCoeff)
{
	if (m_nLights == MAX_LIGHTS)
		return nullptr;

	m_arrLights[m_nLights].direction = direction;
	m_arrLights[m_nLights].color = color;
	m_arrLights[m_nLights].ambientCoefficient = ambientCoeff;
	m_arrLights[m_nLights].isOn = 1.f;

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
	m_arrLights[m_nLights].isOn = 1.f;

	return &m_arrLights[m_nLights++];
}

LightingSystem::Light* LightingSystem::addSpotLight(glm::vec4 position, glm::vec4 direction, glm::vec4 color, float ambientCoeff, float constant, float linear, float quadratic, float cutOffDeg, float outerCutOffDeg)
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
	m_arrLights[m_nLights].isOn = 1.f;
	m_arrLights[m_nLights].isSpotLight = 1.f;

	return &m_arrLights[m_nLights++];
}

#endif