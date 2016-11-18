#include "InfoBoxManager.h"
#include "ShaderUtils.h"
#include <shared\glm\gtc\type_ptr.hpp>
#include <shared\glm\gtc\matrix_transform.hpp>

InfoBoxManager & InfoBoxManager::getInstance()
{
	static InfoBoxManager instance;
	return instance;	
}

bool InfoBoxManager::BInit(TrackedDeviceManager * tdm)
{
	m_pTDM = tdm;
	return true;
}

void InfoBoxManager::addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose, RELATIVE_TO what)
{
	Texture* tex = m_mapTextureBank[pngFileName];
	if (!tex)
	{
		tex = new Texture(pngFileName);
		m_mapTextureBank[pngFileName] = tex;
	}
	float ar = static_cast<float>(tex->getWidth()) / static_cast<float>(tex->getHeight());
	m_mapInfoBoxes[name] = InfoBoxT(tex, width, glm::mat4(pose * glm::scale(glm::mat4(), glm::vec3(width, width / ar, 1.f))), what);
}

bool InfoBoxManager::removeInfoBox(std::string name)
{		
	return m_mapInfoBoxes.erase(name);
}

InfoBoxManager::InfoBoxManager()
	: m_unTransformProgramID(0)
	, m_glVertBuffer(0)
	, m_nMatrixLocation(0)
	, m_unVAO(0)
	, m_pTDM(NULL)
{
	createGeometry();
	createShaders();
	
	addInfoBox(
		"Test 1",
		"cube_texture.png",
		1.f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -2.f)),
		RELATIVE_TO::HMD);
	addInfoBox(
		"Test 2", 
		"test.png", 
		10.f, 
		glm::translate(glm::mat4(), glm::vec3(1.f, 2.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)), 
		RELATIVE_TO::WORLD);
	addInfoBox(
		"Test 3",
		"test.png",
		0.5f,
		glm::translate(glm::mat4(), glm::vec3(-0.3f, 0.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::EDIT_CONTROLLER);
	addInfoBox(
		"Editing Label",
		"editctrlrlabel.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.3f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::EDIT_CONTROLLER);
	addInfoBox(
		"Manipulation Label",
		"manipctrlrlabel.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.3f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::MANIP_CONTROLLER);
}

InfoBoxManager::~InfoBoxManager()
{
}

void InfoBoxManager::receiveEvent(TrackedDevice* device, const int event)
{
	switch (event)
	{
	case EDIT_TRIGGER_CLICKED:
		updateInfoBoxSize("Test 1", 0.1f);
		removeInfoBox("Test 2");
		break;
	default:
		break;
	}
	
}

void InfoBoxManager::render(const float *matVP)
{
	glm::mat4 VP = glm::make_mat4(matVP);

	glm::mat4 HMDXform = glm::inverse(glm::make_mat4(m_pTDM->getHMDPose().get()));
	glm::mat4 EditCtrlrXform = glm::make_mat4(m_pTDM->getEditControllerPose().get());
	glm::mat4 ManipCtrlrXform = glm::make_mat4(m_pTDM->getManipControllerPose().get());

	glUseProgram(m_unTransformProgramID);
	glBindVertexArray(m_unVAO);
	for (auto const& ib : m_mapInfoBoxes)
	{
		RELATIVE_TO relToWhat = std::get<IBIndex::TRANSFORM_RELATION>(ib.second); // get fourth element of infobox tuple

		glm::mat4 relXform;
		if (relToWhat == WORLD) relXform = glm::mat4();
		if (relToWhat == HMD) relXform = HMDXform;
		if (relToWhat == EDIT_CONTROLLER) relXform = EditCtrlrXform;
		if (relToWhat == MANIP_CONTROLLER) relXform = ManipCtrlrXform;

		// short-circuit if controller is not active
		if (relXform == glm::mat4() && (relToWhat == EDIT_CONTROLLER || relToWhat == MANIP_CONTROLLER))
			continue;

		glm::mat4 infoBoxMat = std::get<IBIndex::TRANSFORM_MATRIX>(ib.second);

		glUniformMatrix4fv(m_nMatrixLocation, 1, GL_FALSE, glm::value_ptr(VP * relXform * infoBoxMat));
		std::get<IBIndex::TEXTURE>(ib.second)->activate();
		glDrawArrays(GL_QUADS, 0, 4); 
		std::get<IBIndex::TEXTURE>(ib.second)->deactivate();
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

bool InfoBoxManager::updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose)
{
	float width  = static_cast<float>(std::get<IBIndex::TEXTURE>(m_mapInfoBoxes[infoBoxName])->getWidth());
	float height = static_cast<float>(std::get<IBIndex::TEXTURE>(m_mapInfoBoxes[infoBoxName])->getHeight());
	float ar = width / height;
	std::get<IBIndex::TRANSFORM_MATRIX>(m_mapInfoBoxes[infoBoxName]) = glm::mat4(pose * glm::scale(glm::mat4(), glm::vec3(std::get<IBIndex::SIZE_METERS>(m_mapInfoBoxes[infoBoxName]), std::get<IBIndex::SIZE_METERS>(m_mapInfoBoxes[infoBoxName]) / ar, 1.f)));
	return true;
}

bool InfoBoxManager::updateInfoBoxSize(std::string infoBoxName, float size)
{
	float oldSize = std::get<IBIndex::SIZE_METERS>(m_mapInfoBoxes[infoBoxName]);
	std::get<IBIndex::SIZE_METERS>(m_mapInfoBoxes[infoBoxName]) = size;

	float sizeDelta = size / oldSize;

	float width  = static_cast<float>(std::get<IBIndex::TEXTURE>(m_mapInfoBoxes[infoBoxName])->getWidth());
	float height = static_cast<float>(std::get<IBIndex::TEXTURE>(m_mapInfoBoxes[infoBoxName])->getHeight());
	float ar = width / height;
	std::get<IBIndex::TRANSFORM_MATRIX>(m_mapInfoBoxes[infoBoxName]) = glm::mat4(std::get<IBIndex::TRANSFORM_MATRIX>(m_mapInfoBoxes[infoBoxName]) * glm::scale(glm::mat4(), glm::vec3(sizeDelta, sizeDelta / ar, 1.f)));
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
		"   vec4 col = texture(texSampler, v2TexCoord);\n"
		"   if (col.a < 0.2f)\n"
		"      discard;\n"
		"   outputColor = col;\n"
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