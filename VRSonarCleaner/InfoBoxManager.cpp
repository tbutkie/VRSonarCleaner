#include "InfoBoxManager.h"
#include <shared\glm\gtc\type_ptr.hpp>
#include <shared\glm\gtc\matrix_transform.hpp>

#include "GLSLpreamble.h"

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

void InfoBoxManager::addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose, RELATIVE_TO what, bool billboarded)
{
	Texture* tex = m_mapTextureBank[pngFileName];
	if (!tex)
	{
		tex = new Texture(pngFileName);
		m_mapTextureBank[pngFileName] = tex;
	}

	m_mapInfoBoxes[name] = InfoBoxT(tex, width, pose, what, billboarded);
}

bool InfoBoxManager::removeInfoBox(std::string name)
{		
	return m_mapInfoBoxes.erase(name) > 0u;
}

InfoBoxManager::InfoBoxManager()
	: m_glVertBuffer(0)
	, m_unVAO(0)
	, m_pTDM(NULL)
{
	createGeometry();

	createTutorial();
	
	addInfoBox(
		"Editing Label",
		"editctrlrlabel.png",
		0.05f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::PRIMARY_CONTROLLER,
		false);                                                                                                   
	addInfoBox(
		"Manipulation Label",
		"manipctrlrlabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::SECONDARY_CONTROLLER,
		false);
}

InfoBoxManager::~InfoBoxManager()
{
}

void InfoBoxManager::receiveEvent(const int event, void* data)
{
	switch (event)
	{
	//case BroadcastSystem::EVENT::EDIT_TRIGGER_CLICKED:
	//	updateInfoBoxSize("Test 1", 0.1f);
	//	removeInfoBox("Test 2");
	//	break;
	case BroadcastSystem::EVENT::EXIT_PLAY_AREA:
		updateInfoBoxSize("Test 3", 1.0f);
		break;
	case BroadcastSystem::EVENT::ENTER_PLAY_AREA:
		updateInfoBoxSize("Test 3", 0.5f);
		break;
	default:
		break;
	}
	
}

void InfoBoxManager::render(const float *matVP)
{
	glm::mat4 VP = glm::make_mat4(matVP);

	glm::mat4 HMDXform = glm::inverse(m_pTDM->getHMDPose());

	glBindVertexArray(m_unVAO);
	for (auto const& ib : m_mapInfoBoxes)
	{
		RELATIVE_TO relToWhat = std::get<IBIndex::TRANSFORM_RELATION>(ib.second); // get fourth element of infobox tuple

		glm::mat4 relXform;
		if (relToWhat == WORLD) relXform = glm::mat4();
		if (relToWhat == HMD) relXform = HMDXform;
		if (relToWhat == PRIMARY_CONTROLLER) relXform = m_pTDM->getPrimaryControllerPose();
		if (relToWhat == SECONDARY_CONTROLLER) relXform = m_pTDM->getSecondaryControllerPose();

		// short-circuit if controller is not active
		if ((relToWhat == PRIMARY_CONTROLLER && m_pTDM->getPrimaryController() && !m_pTDM->getPrimaryController()->readyToRender()) ||
			(relToWhat == SECONDARY_CONTROLLER && m_pTDM->getSecondaryController() && !m_pTDM->getSecondaryController()->readyToRender()))
			continue;

		glm::mat4 infoBoxMat = std::get<IBIndex::TRANSFORM_MATRIX>(ib.second);

		float widthPx = static_cast<float>(std::get<IBIndex::TEXTURE>(ib.second)->getWidth());
		float heightPx = static_cast<float>(std::get<IBIndex::TEXTURE>(ib.second)->getHeight());
		float ar = widthPx / heightPx;
		float sizeM = std::get<IBIndex::SIZE_METERS>(ib.second);
		glm::mat4 scaleMat = glm::scale(glm::mat4(), glm::vec3(sizeM, sizeM / ar, 1.f));

		if (std::get<IBIndex::BILLBOARDED>(ib.second))
		{
			infoBoxMat[0] = HMDXform[0];
			infoBoxMat[1] = HMDXform[1];
			infoBoxMat[2] = HMDXform[2];
		}

		glUniformMatrix4fv(MVP_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(VP * relXform * infoBoxMat * scaleMat));
		std::get<IBIndex::TEXTURE>(ib.second)->activate();
		glDrawElements(GL_TRIANGLES, m_uiIndexSize, GL_UNSIGNED_SHORT, 0); 
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

bool InfoBoxManager::updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose)
{
	if (m_mapInfoBoxes.count(infoBoxName) == 0) return false;

	std::get<IBIndex::TRANSFORM_MATRIX>(m_mapInfoBoxes[infoBoxName]) = pose;
	return true;
}

bool InfoBoxManager::updateInfoBoxSize(std::string infoBoxName, float size)
{
	if (m_mapInfoBoxes.count(infoBoxName) == 0) return false;

	std::get<IBIndex::SIZE_METERS>(m_mapInfoBoxes[infoBoxName]) = size;
	return true;
}

void InfoBoxManager::createGeometry()
{
	std::vector<float> vertdataarray;
		
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(0.f);

	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(1.f);
	vertdataarray.push_back(0.f);

	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(1.f);
	vertdataarray.push_back(1.f);

	vertdataarray.push_back(-0.5f);
	vertdataarray.push_back(0.5f);
	vertdataarray.push_back(0.f);
	vertdataarray.push_back(1.f);

	GLushort vIndices[] = { 0, 1, 2,   2, 3, 0 };
	m_uiIndexSize = 6;


	// Setup the VAO the first time through.
	if (m_unVAO == 0)
	{
		glGenVertexArrays(1, &m_unVAO);
		glBindVertexArray(m_unVAO);

		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);	
	
		glGenBuffers(1, &m_glIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

		GLuint stride = 2 * sizeof(float) + 2 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += 2 * sizeof(float);
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}
}

void InfoBoxManager::createTutorial()
{
}
