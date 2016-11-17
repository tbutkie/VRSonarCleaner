#include "InfoBoxManager.h"
#include "ShaderUtils.h"
#include <shared\glm\gtc\type_ptr.hpp>
#include <shared\glm\gtc\matrix_transform.hpp>

InfoBoxManager & InfoBoxManager::getInstance()
{
	static InfoBoxManager instance;
	return instance;	
}

void InfoBoxManager::addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose)
{
	Texture* tex = m_mapTextureBank[pngFileName];
	if (!tex)
	{
		tex = new Texture(pngFileName);
		m_mapTextureBank[pngFileName] = tex;
	}
	float ar = static_cast<float>(tex->getWidth()) / static_cast<float>(tex->getHeight());
	m_mapInfoBoxes[name] = std::tuple<Texture*, float, glm::mat4>(tex, width, glm::mat4(pose * glm::scale(glm::mat4(), glm::vec3(width, width / ar, 1.f))));
}

InfoBoxManager::InfoBoxManager()
	: m_unTransformProgramID(0)
	, m_glVertBuffer(0)
	, m_nMatrixLocation(0)
	, m_unVAO(0)
{
	createGeometry();
	createShaders();
}

InfoBoxManager::~InfoBoxManager()
{
}

void InfoBoxManager::update(TrackedDevice* device, const int event)
{
	switch (event)
	{
	case EVENT_EDIT_TRIGGER_PRESSED:
		updateInfoBoxSize("Test 1", 0.f);
		break;
	default:
		;
	}
	
}

void InfoBoxManager::render(const float *matVP)
{
	glm::mat4 VP = glm::make_mat4(matVP);
	glUseProgram(m_unTransformProgramID);
	glBindVertexArray(m_unVAO);
	for (auto const& ib : m_mapInfoBoxes)
	{
		glUniformMatrix4fv(m_nMatrixLocation, 1, GL_FALSE, glm::value_ptr(VP * std::get<2>(ib.second)));
		std::get<0>(ib.second)->activate();
		glDrawArrays(GL_QUADS, 0, 4); 
		std::get<0>(ib.second)->deactivate();
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

bool InfoBoxManager::updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose)
{
	float width = static_cast<float>(std::get<0>(m_mapInfoBoxes[infoBoxName])->getWidth());
	float height = static_cast<float>(std::get<0>(m_mapInfoBoxes[infoBoxName])->getHeight());
	float ar = width / height;
	std::get<2>(m_mapInfoBoxes[infoBoxName]) = glm::mat4(pose * glm::scale(glm::mat4(), glm::vec3(std::get<1>(m_mapInfoBoxes[infoBoxName]), std::get<1>(m_mapInfoBoxes[infoBoxName]) / ar, 1.f)));
	return true;
}

bool InfoBoxManager::updateInfoBoxSize(std::string infoBoxName, float size)
{
	std::get<1>(m_mapInfoBoxes[infoBoxName]) = size;
	float width = static_cast<float>(std::get<0>(m_mapInfoBoxes[infoBoxName])->getWidth());
	float height = static_cast<float>(std::get<0>(m_mapInfoBoxes[infoBoxName])->getHeight());
	float ar = width / height;
	std::get<2>(m_mapInfoBoxes[infoBoxName]) = glm::mat4(std::get<2>(m_mapInfoBoxes[infoBoxName]) * glm::scale(glm::mat4(), glm::vec3(std::get<1>(m_mapInfoBoxes[infoBoxName]), std::get<1>(m_mapInfoBoxes[infoBoxName]) / ar, 1.f)));
	return false;
}

void InfoBoxManager::createGeometry()
{
	std::vector<float> vertdataarray;
		
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(0.f);

	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(1.f);
	vertdataarray.push_back(0.f);

	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(1.f);
	vertdataarray.push_back(1.f);

	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(1.f);

	// Setup the VAO the first time through.
	if (m_unVAO == 0)
	{
		glGenVertexArrays(1, &m_unVAO);
		glBindVertexArray(m_unVAO);

		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

		GLuint stride = 3 * sizeof(float) + 2 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += 3 * sizeof(float);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
}

bool InfoBoxManager::createShaders()
{
	m_unTransformProgramID = CompileGLShader(
		"InfoBox",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 v2TexCoordIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = vec2(v2TexCoordIn.x, 1.f - v2TexCoordIn.y);\n"
		"	gl_Position = matrix * vec4(position, 1.f);\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"uniform sampler2D texSampler;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture(texSampler, v2TexCoord);\n"
		"}\n"
	);

	m_nMatrixLocation = glGetUniformLocation(m_unTransformProgramID, "matrix");
	if (m_nMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in InfoBox shader\n");
		return false;
	}

	return m_unTransformProgramID != 0;
}