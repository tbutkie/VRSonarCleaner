#include "CloudEditControllerTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "PointCleanProbe.h"
#include <shared/glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"

CloudEditControllerTutorial::CloudEditControllerTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_dvec4BadPointPos1(glm::dvec4(62474))
	, m_dvec4BadPointPos2(glm::dvec4(14274))
	, m_dvec4BadPointPos3(glm::dvec4(1540))
{
}


CloudEditControllerTutorial::~CloudEditControllerTutorial()
{
	if (m_bInitialized)
	{
		delete m_pDemoCloud;

		delete m_pDemoVolume;
		
		delete m_pColorScaler;

		BehaviorManager::getInstance().removeBehavior("Editing");
		BehaviorManager::getInstance().removeBehavior("Done");

		cleanupBadDataLabels();
	}
}

void CloudEditControllerTutorial::init()
{
	glm::vec3 tablePosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(1.f, 1.f, 0.5f);

	m_pDemoVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pDemoCloud = new SonarPointCloud(m_pColorScaler, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt");

	m_pDemoVolume->add(m_pDemoCloud);
	
	refreshColorScale();

	InfoBoxManager::getInstance().addInfoBox(
		"Cloud Editing Tutorial",
		"editpointstut.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.1f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Reset Label",
		"resetleftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.052f, 0.008f, 0.052f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);

	makeBadDataLabels();
	
	BehaviorManager::getInstance().addBehavior("Editing", new PointCleanProbe(m_pTDM, m_pDemoVolume, vr::VRSystem()));
	static_cast<PointCleanProbe*>(BehaviorManager::getInstance().getBehavior("Editing"))->activateDemoMode();

	m_bInitialized = true;
}

void CloudEditControllerTutorial::update()
{
	if (!m_pTDM->getPrimaryController())
		return;

	if (m_pTDM->getPrimaryController()->justPressedTouchpad())
		m_pDemoCloud->resetAllMarks();

	m_pDemoCloud->update();

	BehaviorBase* done = BehaviorManager::getInstance().getBehavior("Done");
	if (done)
	{
		done->update();
		if (!done->isActive())
			m_bActive = false;
	}
	else if (m_pTDM->getPrimaryController()->isTriggerClicked() || m_pTDM->getPrimaryController()->justUnclickedTrigger())
	{
		double minZ = std::numeric_limits<double>::max();
		double maxZ = -std::numeric_limits<double>::max();

		for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
		{
			if (m_pDemoCloud->getPointMark(i) != 1)
			{
				minZ = m_pDemoCloud->getRawPointPosition(i).z < minZ ? m_pDemoCloud->getRawPointPosition(i).z : minZ;
				maxZ = m_pDemoCloud->getRawPointPosition(i).z > maxZ ? m_pDemoCloud->getRawPointPosition(i).z : maxZ;
			}
		}
		if (minZ >= 19.731 && maxZ <= 22.89)
		{
			BehaviorManager::getInstance().removeBehavior("Editing");

			cleanupBadDataLabels();
			InfoBoxManager::getInstance().removeInfoBox("Cloud Editing Tutorial");
			InfoBoxManager::getInstance().removeInfoBox("Reset Label");

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}

	std::cout.precision(std::numeric_limits<double>::max_digits10);

	if (m_pTDM->getSecondaryController()->isTouchpadClicked())
		for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
			if (m_pDemoCloud->getPointMark(i) >= 100)
				std::cout << i << ": (" << m_pDemoCloud->getRawPointPosition(i).x << ", " << m_pDemoCloud->getRawPointPosition(i).y << ", " << m_pDemoCloud->getRawPointPosition(i).z << ")" << std::endl;
}

void CloudEditControllerTutorial::draw()
{
	m_pDemoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 2.f);
	m_pDemoVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_POINTS;
	rs.shaderName = "flat";
	rs.VAO = m_pDemoCloud->getVAO();
	rs.vertCount = m_pDemoCloud->getPointCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.modelToWorldTransform = m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud);

	Renderer::getInstance().addToDynamicRenderQueue(rs);
	
}

void CloudEditControllerTutorial::refreshColorScale()
{
	if (!m_pDemoCloud)
		return;

	m_pColorScaler->resetMinMaxForColorScale(m_pDemoVolume->getMinDataBound().z, m_pDemoVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pDemoCloud->getMinDepthTPU(),
		m_pDemoCloud->getMaxDepthTPU(),
		m_pDemoCloud->getMinPositionalTPU(),
		m_pDemoCloud->getMaxPositionalTPU()
	);

	// apply new color scale
	m_pDemoCloud->resetAllMarks();
}

void CloudEditControllerTutorial::makeBadDataLabels()
{
	glm::dmat4 dataXform = m_pDemoVolume->getRawDomainToVolumeTransform();

	glm::vec3 tmp(dataXform * m_dvec4BadPointPos1);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 1",
		"baddataleftlabel.png",
		0.25f,
		glm::translate(glm::mat4(), tmp),// *glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 2",
		"baddatarightlabel.png",
		0.25f,
		glm::translate(glm::mat4(), tmp) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	//InfoBoxManager::getInstance().addInfoBox(
	//	"Bad Data Label 3",
	//	"baddatarightlabel.png",
	//	0.25f,
	//	glm::translate(glm::mat4(), glm::vec3(-0.825f, 1.475f, 0.525f)),
	//	InfoBoxManager::RELATIVE_TO::WORLD,
	//	false);
	//
	//InfoBoxManager::getInstance().addInfoBox(
	//	"Bad Data Label 4",
	//	"baddataleftlabel.png",
	//	0.25f,
	//	glm::translate(glm::mat4(), glm::vec3(-0.825f, 1.475f, 0.525f)) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)),
	//	InfoBoxManager::RELATIVE_TO::WORLD,
	//	false);
}

void CloudEditControllerTutorial::cleanupBadDataLabels()
{
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 1");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 2");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 3");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 4");
}
