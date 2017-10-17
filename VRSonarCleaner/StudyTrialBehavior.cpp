#include "StudyTrialBehavior.h"
#include "BehaviorManager.h"
#include "GrabDataVolumeBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include "Renderer.h"


StudyTrialBehavior::StudyTrialBehavior(TrackedDeviceManager* pTDM, std::string fileName, DataLogger::LogHandle log)
	: m_pTDM(pTDM)
	, m_Log(log)
{
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pPointCloud = new SonarPointCloud(m_pColorScaler, fileName, SonarPointCloud::SONAR_FILETYPE::XYZF);

	m_pDataVolume = new DataVolume(glm::vec3(0.f, 1.f, 0.f), glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.f));

	m_pDataVolume->add(m_pPointCloud);

	m_pColorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pPointCloud->getMinDepthTPU(),
		m_pPointCloud->getMaxDepthTPU(),
		m_pPointCloud->getMinPositionalTPU(),
		m_pPointCloud->getMaxPositionalTPU()
	);
}


StudyTrialBehavior::~StudyTrialBehavior()
{
	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pDataVolume)
		delete m_pDataVolume;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTrialBehavior::init()
{
	std::cout << "Starting trials for " << m_pPointCloud->getName() << std::endl;
	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM, m_pDataVolume, vr::VRSystem()));
	BehaviorManager::getInstance().addBehavior("grab", new GrabDataVolumeBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));
}

void StudyTrialBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	bool allDone = true;

	for (int i = 0; i < m_pPointCloud->getPointCount(); ++i)
	{
		if (m_pPointCloud->getPointDepthTPU(i) == 1.f && m_pPointCloud->getPointMark(i) != 1)
		{
			allDone = false;
			break;
		}
	}
	if (allDone || (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedGrip()))
	{
		BehaviorManager::getInstance().removeBehavior("scale");
		BehaviorManager::getInstance().removeBehavior("grab");
		BehaviorManager::getInstance().removeBehavior("edit");

		m_bActive = false;
	}
}

void StudyTrialBehavior::draw()
{
	m_pDataVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 2.f);
	m_pDataVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_POINTS;
	rs.shaderName = "flat";
	rs.VAO = m_pPointCloud->getVAO();
	rs.vertCount = m_pPointCloud->getPointCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.modelToWorldTransform = m_pDataVolume->getCurrentDataTransform(m_pPointCloud);

	Renderer::getInstance().addToDynamicRenderQueue(rs);
}
