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

void LightingSystem::updateLightingUniforms()
{
	if (m_bRefreshShader)
		generateLightingShader();

	glUseProgram(m_glProgramID);

	glm::vec3 black(0.f);

	// Directional light
	for (int i = 0; i < dLights.size(); ++i)
	{
		std::string name = "dirLights[" + std::to_string(i);
		name += "]";

		if (dLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(dLights[i].position));
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
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(pLights[i].position));
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

// Uses the current shader
void LightingSystem::updateView(glm::mat4 view)
{
	if (m_bRefreshShader)
		generateLightingShader();

	glUseProgram(m_glProgramID);

	glm::mat4 invView = glm::inverse(view);
	glm::vec3 camPos(invView[3].x, invView[3].y, invView[3].z);
	glm::vec3 camFwd(invView[2].x, invView[2].y, invView[2].z);

	glUniform3fv(glGetUniformLocation(m_glProgramID, "viewPos"), 1, glm::value_ptr(camPos));

	// SpotLight
	for (int i = 0; i < sLights.size(); ++i)
	{
		std::string name = "spotLights[" + std::to_string(i);
		name += "]";

		if (sLights[i].attachedToCamera)
		{
			sLights[i].position = camPos;
			sLights[i].direction = camFwd;
		}

		if (sLights[i].on)
		{
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".position").c_str()), 1, glm::value_ptr(sLights[i].position));
			glUniform3fv(glGetUniformLocation(m_glProgramID, (name + ".direction").c_str()), 1, glm::value_ptr(sLights[i].direction));
		}
	}	
}

bool LightingSystem::addDirectLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
	DLight dl;

	dl.position = position;
	dl.ambient = ambient;
	dl.diffuse = diffuse;
	dl.specular = specular;

	dl.on = true;

	dLights.push_back(dl);

	m_bRefreshShader = true;

	return true;
}

bool LightingSystem::addPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
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

	return true;
}

bool LightingSystem::addSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float cutOffDeg, float outerCutOffDeg, bool attachToCamera)
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

	return true;
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
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(POSITION_ATTRIB_LOCATION)); vBuffer.append(")");
		vBuffer.append("	in vec3 v3Pposition;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(NORMAL_ATTRIB_LOCATION)); vBuffer.append(")");
		vBuffer.append("	in vec3 v3NormalIn;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MVP_UNIFORM_LOCATION)); vBuffer.append(")");
		vBuffer.append("	uniform mat4 m4MVP;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_UNIFORM_LOCATION)); vBuffer.append(")");
		vBuffer.append("	uniform mat4 m4MV;\n");
		vBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_INV_TRANS_UNIFORM_LOCATION)); vBuffer.append(")");
		vBuffer.append("	uniform mat3 m3MVInvTrans;\n");
		vBuffer.append("out vec3 v3Normal;\n");
		vBuffer.append("out vec3 v3FragPos;\n");
		vBuffer.append("void main()\n");
		vBuffer.append("{\n");
		vBuffer.append("	gl_Position = m4MVP * vec4(v3Position, 1.0f);\n");
		vBuffer.append("	v3FragPos = vec3(m4MV * vec4(v3Position, 1.0f));\n");
		vBuffer.append("	v3Normal = normalize(m3MVInvTrans * v3NormalIn);\n"); // this preserves correct normals under nonuniform scaling by using the normal matrix
		vBuffer.append("}\n");
	} // VERTEX SHADER

	// INSTANCED VERTEX SHADER
	{
		vInstancedBuffer.append("#version 450 core\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(POSITION_ATTRIB_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	in vec3 v3Position;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(NORMAL_ATTRIB_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	in vec3 v3NormalIn;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(INSTANCE_POSITION_ATTRIB_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	in vec3 v3InstancePosition;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(INSTANCE_FORWARD_ATTRIB_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	in vec3 v3InstanceForward;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(MVP_UNIFORM_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	uniform mat4 m4MVP;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_UNIFORM_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	uniform mat4 m4MV;\n");
		vInstancedBuffer.append("layout(location = "); vBuffer.append(std::to_string(MV_INV_TRANS_UNIFORM_LOCATION)); vBuffer.append(")");
		vInstancedBuffer.append("	uniform mat3 m3MVInvTrans;\n");
		vInstancedBuffer.append("out vec3 v3Normal;\n");
		vInstancedBuffer.append("out vec3 v3FragPos;\n");
		vInstancedBuffer.append("void main()\n");
		vInstancedBuffer.append("{\n");
		vInstancedBuffer.append("	gl_Position = m4MVP * vec4(v3Position, 1.0f);\n");
		vInstancedBuffer.append("	v3FragPos = vec3(m4MV * vec4(v3Position, 1.0f));\n");
		vInstancedBuffer.append("	v3Normal = normalize(m3MVInvTrans * v3NormalIn);\n"); // this preserves correct normals under nonuniform scaling by using the normal matrix
		vInstancedBuffer.append("}\n");
	}

	// FRAGMENT SHADER
	{
		fBuffer.append("#version 450 core\n");
		
		fBuffer.append("struct Material {\n");
		fBuffer.append("    vec3 diffuse;\n");
		fBuffer.append("    vec3 specular;\n");
		fBuffer.append("    vec3 emissive;\n");
		fBuffer.append("    float shininess;\n");
		fBuffer.append("};\n");
		fBuffer.append("uniform Material material;\n");

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
			fBuffer.append("    vec3 lightDir = normalize(light.direction);\n");
			fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
			fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
			fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
			fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
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
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
			fBuffer.append("    float distance = length(light.position - fragPos);\n");
			fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
			fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
			fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
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
			fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
			fBuffer.append("    float distance = length(light.position - fragPos);\n");
			fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
			fBuffer.append("    float theta = dot(lightDir, normalize(-light.direction));\n");
			fBuffer.append("    float epsilon = light.cutOff - light.outerCutOff;\n");
			fBuffer.append("    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n");
			fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
			fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
			fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
			fBuffer.append("    ambient *= attenuation * intensity;\n");
			fBuffer.append("    diffuse *= attenuation * intensity;\n");
			fBuffer.append("    specular *= attenuation * intensity;\n");
			fBuffer.append("    return (ambient + diffuse + specular);\n");
			fBuffer.append("}\n");
		}

		fBuffer.append("in vec3 FragPos;\n");
		fBuffer.append("in vec3 Normal;\n");
		fBuffer.append("out vec4 color;\n");
		fBuffer.append("uniform vec3 viewPos;\n");

		fBuffer.append("void main()\n");
		fBuffer.append("{\n");
		fBuffer.append("    vec3 viewDirection = normalize(viewPos - FragPos);\n");
		fBuffer.append("    vec3 norm = Normal;\n");
		fBuffer.append("    if(!gl_FrontFacing)\n");
		fBuffer.append("		norm = -norm;\n");
		fBuffer.append("    vec3 result = vec3(0.f);\n");
		if (dLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_DIR_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcDirLight(dirLights[i], norm, viewDirection);\n");
		}
		if (pLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_POINT_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcPointLight(pointLights[i], norm, FragPos, viewDirection);\n");
		}
		if (sLights.size() > 0)
		{
			fBuffer.append("    for(int i = 0; i < N_SPOT_LIGHTS; i++)\n");
			fBuffer.append("        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDirection);\n");
		}
		fBuffer.append("    result += material.emissive;\n");
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