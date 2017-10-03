#include "CloudEditControllerTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "PointCleanProbe.h"
#include <shared/glm/gtc/matrix_transform.hpp>
#include "Renderer.h"

CloudEditControllerTutorial::CloudEditControllerTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
{
}


CloudEditControllerTutorial::~CloudEditControllerTutorial()
{
	for (auto &cloud : m_vpClouds)
		delete cloud;

	delete m_pDemoVolume;
	delete m_pColorScaler;

	BehaviorManager::getInstance().removeBehavior("Editing");
	InfoBoxManager::getInstance().removeInfoBox("Cloud Editing Tutorial");
	InfoBoxManager::getInstance().removeInfoBox("Activate Label");
	InfoBoxManager::getInstance().removeInfoBox("Reset Label");
}

void CloudEditControllerTutorial::init()
{
	glm::vec3 tablePosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(2.25f, 2.25f, 0.75f);

	m_pDemoVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);
	//m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale);
	//m_pColorScaler->setColorMap(ColorScaler::ColorMap::Rainbow);

	m_vpClouds.push_back(new SonarPointCloud(m_pColorScaler, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt"));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt"));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt"));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt"));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt"));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt"));

	for (auto const &cloud : m_vpClouds)
		m_pDemoVolume->add(cloud);
	
	refreshColorScale();

	InfoBoxManager::getInstance().addInfoBox(
		"Cloud Editing Tutorial",
		"editpointstut.png",
		3.5f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 2.f, -2.9f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Activate Label",
		"activaterightlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(-0.05f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Reset Label",
		"resetleftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.052f, 0.008f, 0.052f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);
	
	BehaviorManager::getInstance().addBehavior("Editing", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pDemoVolume, vr::VRSystem()));
	static_cast<PointCleanProbe*>(BehaviorManager::getInstance().getBehavior("Editing"))->activateDemoMode();
}

void CloudEditControllerTutorial::update()
{
	static_cast<PointCleanProbe*>(BehaviorManager::getInstance().getBehavior("Editing"))->update();

	for (auto &cloud : m_vpClouds)
	{
		if (m_pTDM->getPrimaryController()->justPressedTouchpad())
			cloud->resetAllMarks();

		cloud->update();
	}
}

void CloudEditControllerTutorial::draw()
{
	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_POINTS;
	rs.shaderName = "flat";
	rs.indexType = GL_UNSIGNED_INT;

	for (auto &cloud : m_vpClouds)
	{
		rs.VAO = static_cast<SonarPointCloud*>(cloud)->getVAO();
		rs.vertCount = static_cast<SonarPointCloud*>(cloud)->getPointCount();
		rs.modelToWorldTransform = m_pDemoVolume->getCurrentDataTransform(cloud);
		Renderer::getInstance().addToDynamicRenderQueue(rs);
	}
}

void CloudEditControllerTutorial::refreshColorScale()
{
	if (m_vpClouds.size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(m_vpClouds.begin(), m_vpClouds.end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(m_vpClouds.begin(), m_vpClouds.end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(m_vpClouds.begin(), m_vpClouds.end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(m_vpClouds.begin(), m_vpClouds.end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	m_pColorScaler->resetMinMaxForColorScale(m_pDemoVolume->getMinDataBound().z, m_pDemoVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : m_vpClouds)
		cloud->resetAllMarks();
}