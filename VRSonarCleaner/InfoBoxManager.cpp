#include "InfoBoxManager.h"
#include <gtc\type_ptr.hpp>
#include <gtc\matrix_transform.hpp>

#include "GLSLpreamble.h"
#include "utilities.h"
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


}

InfoBoxManager::~InfoBoxManager()
{
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
			infoBoxMat = utils::getBillBoardTransform(glm::vec3(infoBoxMat[3]), glm::vec3(HMDXform[3]), glm::vec3(0.f, 1.f, 0.f), true);

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
