#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include "LightingSystem.h"

#include "GLSLpreamble.h"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif // !GLEW_STATIC
#include <GL/glew.h>

#include <shared/glm/gtc/matrix_transform.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include <string>

LightingSystem::LightingSystem() 
	: m_bRefreshShader(true)
	, m_bDrawLightBulbs(true)
{
}

LightingSystem::~LightingSystem()
{
	dLights.clear();
	pLights.clear();
	sLights.clear();
}

// Uses the current shader
void LightingSystem::update(glm::mat4 view)
{
	if (m_bRefreshShader)
		generateLightingShader();

	glUseProgram(m_glProgramID);

	updateLightingUniforms();

	// Directional light
	for (int i = 0; i < dLights.size(); ++i)
	{
		std::string name = "dirLights[" + std::to_string(i);
		name += "]";

		glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(glm::mat3(view) * dLights[i].direction)));
	}

	// Point light
	for (int i = 0; i < pLights.size(); ++i)
	{
		std::string name = "pointLights[" + std::to_string(i);
		name += "]";

		glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(pLights[i].position, 1.f))));

	}

	// SpotLight
	for (int i = 0; i < sLights.size(); ++i)
	{
		std::string name = "spotLights[" + std::to_string(i);
		name += "]";

		if (sLights[i].attachedToCamera)
		{
			sLights[i].position = glm::vec3(0.f);
			sLights[i].direction = glm::vec3(0.f, 0.f, -1.f);
		}

		if (sLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(sLights[i].position, 1.f))));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(glm::normalize(glm::vec3(view * glm::vec4(sLights[i].direction, 0.f)))));
		}
	}	
}


void LightingSystem::updateLightingUniforms()
{
	glm::vec3 black(0.f);

	// Directional light
	for (int i = 0; i < dLights.size(); ++i)
	{
		std::string name = "dirLights[" + std::to_string(i);
		name += "]";

		if (dLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(dLights[i].ambient));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(dLights[i].diffuse));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(dLights[i].specular));
		}
		else
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(black));
		}
	}

	// Point light
	for (int i = 0; i < pLights.size(); ++i)
	{
		std::string name = "pointLights[" + std::to_string(i);
		name += "]";

		if (pLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(pLights[i].ambient));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(pLights[i].diffuse));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(pLights[i].specular));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".constant").c_str()), pLights[i].constant);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".linear").c_str()), pLights[i].linear);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".quadratic").c_str()), pLights[i].quadratic);
		}
		else
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(black));
		}
	}

	// SpotLight
	for (int i = 0; i < sLights.size(); ++i)
	{
		std::string name = "spotLights[" + std::to_string(i);
		name += "]";

		if (sLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(sLights[i].ambient));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(sLights[i].diffuse));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(sLights[i].specular));
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".constant").c_str()), sLights[i].constant);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".linear").c_str()), sLights[i].linear);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".quadratic").c_str()), sLights[i].quadratic);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".cutOff").c_str()), sLights[i].cutOff);
			glUniform1f(glGetUniformLocation(m_glProgramID, (name + ".outerCutOff").c_str()), sLights[i].outerCutOff);
		}
		else
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".ambient").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".diffuse").c_str()), 1, glm::value_ptr(black));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".specular").c_str()), 1, glm::value_ptr(black));
		}
	}
}

LightingSystem::DLight* LightingSystem::addDirectLight(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
	DLight dl;

	dl.direction = direction;
	dl.ambient = ambient;
	dl.diffuse = diffuse;
	dl.specular = specular;

	dl.on = true;

	dLights.push_back(dl);

	m_bRefreshShader = true;

	return &dLights.back();
}

LightingSystem::PLight* LightingSystem::addPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
{
	PLight pl;
	pl.position = position;
	pl.ambient = ambient;
	pl.diffuse = diffuse;
	pl.specular = specular;
	pl.constant = constant;
	pl.linear = linear;
	pl.quadratic = quadratic;

	pl.on = true;

	pLights.push_back(pl);

	m_bRefreshShader = true;

	return &pLights.back();
}

LightingSystem::SLight* LightingSystem::addSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float cutOffDeg, float outerCutOffDeg, bool attachToCamera)
{
	SLight sl;
	sl.position = position;
	sl.direction = direction;
	sl.ambient = ambient;
	sl.diffuse = diffuse;
	sl.specular = specular;
	sl.constant = constant;
	sl.linear = linear;
	sl.quadratic = quadratic;
	sl.cutOff = glm::cos(glm::radians(cutOffDeg));
	sl.outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
	sl.attachedToCamera = attachToCamera;

	sl.on = true;

	sLights.push_back(sl);

	m_bRefreshShader = true;

	return &sLights.back();
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

void LightingSystem::activateShader()
{
	glUseProgram(m_glProgramID);
}

void LightingSystem::deactivateShader()
{
	glUseProgram(0);
}

void LightingSystem::generateLightingShader()
{
	std::string vBuffer, vInstancedBuffer, fBuffer;

	// VERTEX SHADER
	{
		vBuffer.append("#version 450 core\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(POSITION_ATTRIB_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	in vec3 v3Position;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(NORMAL_ATTRIB_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	in vec3 v3NormalIn;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(TEXCOORD_ATTRIB_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	in vec2 v2TexCoordsIn;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MVP_UNIFORM_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	uniform mat4 m4MVP;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_UNIFORM_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	uniform mat4 m4MV;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_INV_TRANS_UNIFORM_LOCATION)); vBuffer.append(")\n");
		vBuffer.append("	uniform mat3 m3MVInvTrans;\n");
		vBuffer.append("out vec3 v3Normal;\n");
		vBuffer.append("out vec3 v3FragPos;\n");
		vBuffer.append("out vec2 v2TexCoords;\n");
		vBuffer.append("void main()\n");
		vBuffer.append("{\n");
		vBuffer.append("	gl_Position = m4MVP * vec4(v3Position, 1.0f);\n");
		vBuffer.append("	v3FragPos = vec3(m4MV * vec4(v3Position, 1.0f));\n");
		vBuffer.append("	v3Normal =  m3MVInvTrans * v3NormalIn;\n"); // this preserves correct normals under nonuniform scaling by using the normal matrix
		vBuffer.append("	v2TexCoords = v2TexCoordsIn;\n");
		vBuffer.append("}\n");
	} // VERTEX SHADER

	// INSTANCED VERTEX SHADER
	{
		vInstancedBuffer.append("#version 450 core\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(POSITION_ATTRIB_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	in vec3 v3Position;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(NORMAL_ATTRIB_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	in vec3 v3NormalIn;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(TEXCOORD_ATTRIB_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	in vec2 v2TexCoordsIn;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(INSTANCE_POSITION_ATTRIB_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	in vec3 v3InstancePosition;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(INSTANCE_FORWARD_ATTRIB_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	in vec3 v3InstanceForward;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(MVP_UNIFORM_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	uniform mat4 m4MVP;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(MV_UNIFORM_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	uniform mat4 m4MV;\n");
		vInstancedBuffer.append("layout(location = "); vInstancedBuffer.append(std::to_string(MV_INV_TRANS_UNIFORM_LOCATION)); vInstancedBuffer.append(")\n");
		vInstancedBuffer.append("	uniform mat3 m3MVInvTrans;\n");
		vInstancedBuffer.append("out vec3 v3Normal;\n");
		vInstancedBuffer.append("out vec3 v3FragPos;\n");
		vInstancedBuffer.append("out vec2 v2TexCoords;\n");
		vInstancedBuffer.append("void main()\n");
		vInstancedBuffer.append("{\n");
		vInstancedBuffer.append("	gl_Position = m4MVP * vec4(v3Position, 1.0f);\n");
		vInstancedBuffer.append("	v3FragPos = vec3(m4MV * vec4(v3Position, 1.0f));\n");
		vInstancedBuffer.append("	v3Normal = m3MVInvTrans * v3NormalIn;\n"); // this preserves correct normals under nonuniform scaling by using the normal matrix
		vInstancedBuffer.append("	v2TexCoords = v2TexCoordsIn;\n");
		vInstancedBuffer.append("}\n");
	}

	// FRAGMENT SHADER
	{
		fBuffer.append("#version 450 core\n");
		fBuffer.append("layout(binding = "); fBuffer.append(std::to_string(DIFFUSE_TEXTURE_BINDING)); fBuffer.append(")\n");
		fBuffer.append("	uniform sampler2D diffuseTex;\n");
		fBuffer.append("layout(binding = "); fBuffer.append(std::to_string(SPECULAR_TEXTURE_BINDING)); fBuffer.append(")\n");
		fBuffer.append("	uniform sampler2D specularTex;\n");
		fBuffer.append("layout(binding = "); fBuffer.append(std::to_string(EMISSIVE_TEXTURE_BINDING)); fBuffer.append(")\n");
		fBuffer.append("	uniform sampler2D emissiveTex;\n");
		fBuffer.append("layout(location = "); fBuffer.append(std::to_string(MATERIAL_SHININESS_UNIFORM_LOCATION)); fBuffer.append(")\n");
		fBuffer.append("	uniform float shininess;\n");

		fBuffer.append("in vec3 v3Normal;\n");
		fBuffer.append("in vec3 v3FragPos;\n");
		fBuffer.append("in vec2 v2TexCoords;\n");

		fBuffer.append("out vec4 color;\n");

		if (dLights.size() > 0)
		{
			fBuffer.append("#define N_DIR_LIGHTS "); fBuffer.append(std::to_string(dLights.size())); fBuffer.append("\n");
			fBuffer.append("struct DirLight {\n");
			fBuffer.append("    vec3 direction;\n");
			fBuffer.append("    vec3 ambient;\n");
			fBuffer.append("    vec3 diffuse;\n");
			fBuffer.append("    vec3 specular;\n");
			fBuffer.append("};\n");
			fBuffer.append("uniform DirLight dirLights[N_DIR_LIGHTS];\n");
			fBuffer.append("vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)\n");
			fBuffer.append("{\n");
			fBuffer.append("    float diff = max(dot(normal, -light.direction), 0.0);\n");
			fBuffer.append("    vec3 reflectDir = reflect(light.direction, normal);\n");
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.f), shininess);\n");
			fBuffer.append("    vec3 ambient = light.ambient * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 specular = light.specular * spec * vec3(texture(specularTex, v2TexCoords));\n");
			fBuffer.append("    return (ambient + diffuse + specular);\n");
			fBuffer.append("}\n");
		}
		if (pLights.size() > 0)
		{
			fBuffer.append("#define N_POINT_LIGHTS "); fBuffer.append(std::to_string(pLights.size())); fBuffer.append("\n");
			fBuffer.append("struct PointLight {\n");
			fBuffer.append("    vec3 position;\n");
			fBuffer.append("    float constant;\n");
			fBuffer.append("    float linear;\n");
			fBuffer.append("    float quadratic;\n");
			fBuffer.append("    vec3 ambient;\n");
			fBuffer.append("    vec3 diffuse;\n");
			fBuffer.append("    vec3 specular;\n");
			fBuffer.append("};\n");
			fBuffer.append("uniform PointLight pointLights[N_POINT_LIGHTS];\n");
			fBuffer.append("vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n");
			fBuffer.append("{\n");
			fBuffer.append("    vec3 lightDir = normalize(light.position - fragPos);\n");
			fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
			fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n");
			fBuffer.append("    float distance = length(light.position - fragPos);\n");
			fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
			fBuffer.append("    vec3 ambient = light.ambient * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 specular = light.specular * spec * vec3(texture(specularTex, v2TexCoords));\n");
			fBuffer.append("    ambient *= attenuation;\n");
			fBuffer.append("    diffuse *= attenuation;\n");
			fBuffer.append("    specular *= attenuation;\n");
			fBuffer.append("    return (ambient + diffuse + specular);\n");
			fBuffer.append("}\n");
		}
		if (sLights.size() > 0)
		{
			fBuffer.append("#define N_SPOT_LIGHTS "); fBuffer.append(std::to_string(sLights.size())); fBuffer.append("\n");
			fBuffer.append("struct SpotLight {\n");
			fBuffer.append("    vec3 position;\n");
			fBuffer.append("    vec3 direction;\n");
			fBuffer.append("    float cutOff;\n");
			fBuffer.append("    float outerCutOff;\n");
			fBuffer.append("    float constant;\n");
			fBuffer.append("    float linear;\n");
			fBuffer.append("    float quadratic;\n");
			fBuffer.append("    vec3 ambient;\n");
			fBuffer.append("    vec3 diffuse;\n");
			fBuffer.append("    vec3 specular;\n");
			fBuffer.append("};\n");
			fBuffer.append("uniform SpotLight spotLights[N_SPOT_LIGHTS];\n");
			fBuffer.append("vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n");
			fBuffer.append("{\n");
			fBuffer.append("    vec3 lightDir = normalize(light.position - fragPos);\n");
			fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
			fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n");
			fBuffer.append("    float distance = length(light.position - fragPos);\n");
			fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
			fBuffer.append("    float theta = dot(lightDir, -light.direction);\n");
			fBuffer.append("    float epsilon = light.cutOff - light.outerCutOff;\n");
			fBuffer.append("    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n");
			fBuffer.append("    vec3 ambient = light.ambient * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTex, v2TexCoords));\n");
			fBuffer.append("    vec3 specular = light.specular * spec * vec3(texture(specularTex, v2TexCoords));\n");
			fBuffer.append("    ambient *= attenuation * intensity;\n");
			fBuffer.append("    diffuse *= attenuation * intensity;\n");
			fBuffer.append("    specular *= attenuation * intensity;\n");
			fBuffer.append("    return (ambient + diffuse + specular);\n");
			fBuffer.append("}\n");
		}

		fBuffer.append("void main()\n");
		fBuffer.append("{\n");
		fBuffer.append("    vec3 norm = normalize(v3Normal);\n");
		fBuffer.append("    vec3 viewDirection = normalize(-v3FragPos);\n");
		fBuffer.append("    vec3 result = vec3(0.f);\n");
		if (dLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_DIR_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcDirLight(dirLights[i], norm, viewDirection);\n");
		}
		if (pLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_POINT_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcPointLight(pointLights[i], norm, v3FragPos, viewDirection);\n");
		}
		if (sLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_SPOT_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcSpotLight(spotLights[i], norm, v3FragPos, viewDirection);\n");
		}
		//fBuffer.append("    result += vec3(texture(emissiveTex, v2TexCoords));\n");
		//fBuffer.append("    vec3 gammaCorrection = vec3(1.f/2.2f);\n");
		//fBuffer.append("    color = vec4(pow(result, gammaCorrection), 1.0);\n");
		fBuffer.append("    color = vec4(result, 1.0);\n");
       
		fBuffer.append("}\n");
	} // FRAGMENT SHADER

	//std::cout << fBuffer << std::endl;

	m_bRefreshShader = false;

	m_glProgramID = CompileGLShader("Lighting Shader", vBuffer.c_str(), fBuffer.c_str());
	m_glInstancedProgramID = CompileGLShader("Instanced Lighting Shader", vInstancedBuffer.c_str(), fBuffer.c_str());
}

#endif