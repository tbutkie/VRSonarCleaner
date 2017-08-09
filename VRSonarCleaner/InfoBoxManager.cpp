#include "InfoBoxManager.h"
#include <shared\glm\gtc\type_ptr.hpp>
#include <shared\glm\gtc\matrix_transform.hpp>

#include "GLSLpreamble.h"

#include "Renderer.h"

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
	GLTexture* tex = Renderer::getInstance().getTexture(pngFileName);
	if (tex == NULL)
	{
		tex = new GLTexture(pngFileName, true);
		Renderer::getInstance().addTexture(tex);
	}

	m_mapInfoBoxes[name] = InfoBoxT(tex, width, pose, what, billboarded);
}

bool InfoBoxManager::removeInfoBox(std::string name)
{		
	return m_mapInfoBoxes.erase(name) > 0u;
}

InfoBoxManager::InfoBoxManager()
	: m_pTDM(NULL)
{
	createTutorial();
	
	addInfoBox(
		"Editing Label",
		"editctrlrlabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::PRIMARY_CONTROLLER,
		false);
	addInfoBox(
		"Activate Label (Primary)",
		"activaterightlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(-0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::PRIMARY_CONTROLLER,
		false);
	addInfoBox(
		"Manipulation Label",
		"manipctrlrlabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::SECONDARY_CONTROLLER,
		false);
	addInfoBox(
		"Activate Label (Secondary)",
		"activateleftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	//addInfoBox(
	//	"Test 1",
	//	"cube_texture.png",
	//	1.f,
	//	glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -2.f)),
	//	RELATIVE_TO::HMD,
	//	false);
	//addInfoBox(
	//	"Test 2",
	//	"test.png",
	//	1.f,
	//	glm::translate(glm::mat4(), glm::vec3(1.f, 2.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)),
	//	RELATIVE_TO::WORLD,
	//	true);
	//addInfoBox(
	//	"Test 3",
	//	"test.png",
	//	2.f,
	//	glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -1.f)),
	//	RELATIVE_TO::HMD,
	//	false);
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
		updateInfoBoxSize("Test 3", 10.0f);
		break;
	case BroadcastSystem::EVENT::ENTER_PLAY_AREA:
		updateInfoBoxSize("Test 3", 10.f);
		break;
	default:
		break;
	}
	
}

void InfoBoxManager::draw()
{
	glm::mat4 HMDXform = m_pTDM->getHMDToWorldTransform();

	//glBindVertexArray(m_unVAO);
	for (auto const& ib : m_mapInfoBoxes)
	{
		RELATIVE_TO relToWhat = std::get<IBIndex::TRANSFORM_RELATION>(ib.second); // get fourth element of infobox tuple

		glm::mat4 relXform;
		if (relToWhat == WORLD) relXform = glm::mat4();
		if (relToWhat == HMD) relXform = HMDXform;
		if (relToWhat == PRIMARY_CONTROLLER) relXform = m_pTDM->getPrimaryControllerPose();
		if (relToWhat == SECONDARY_CONTROLLER) relXform = m_pTDM->getSecondaryControllerPose();

		// short-circuit if controller is not active
		if ((relToWhat == PRIMARY_CONTROLLER && !(m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->readyToRender())) ||
			(relToWhat == SECONDARY_CONTROLLER && !(m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->readyToRender())))
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

		glm::mat4 modelTransform = relXform * infoBoxMat * scaleMat;
		Renderer::getInstance().drawPrimitive("quad", modelTransform, std::get<IBIndex::TEXTURE>(ib.second)->getName(), "black", 0.f);
	}
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

void InfoBoxManager::createTutorial()
{
}
