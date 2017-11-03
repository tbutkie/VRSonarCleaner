#include "DesktopCleanBehavior.h"

#include "BehaviorManager.h"
#include "Renderer.h"
#include "DataLogger.h"

using namespace std::chrono_literals;

DesktopCleanBehavior::DesktopCleanBehavior(DataVolume* pointCloudVolume, Renderer::SceneViewInfo *sceneinfo, glm::ivec4 &viewport)
	: m_pDataVolume(pointCloudVolume)
	, m_pDesktop3DViewInfo(sceneinfo)
	, m_ivec4Viewport(viewport)
{
}


DesktopCleanBehavior::~DesktopCleanBehavior()
{
}

void DesktopCleanBehavior::init()
{
	m_pArcball = new ArcBall(m_pDataVolume);
	m_pArcball->setProjection(&m_pDesktop3DViewInfo->projection);
	m_pArcball->setView(&m_pDesktop3DViewInfo->view);
	m_pArcball->setViewport(m_ivec4Viewport);
	BehaviorManager::getInstance().addBehavior("arcball", m_pArcball);

	m_pLasso = new LassoTool();
	BehaviorManager::getInstance().addBehavior("lasso", m_pLasso);
	m_pLasso->init();

	m_bInitialized = true;
}

void DesktopCleanBehavior::update()
{	
	
}

void DesktopCleanBehavior::draw()
{
	
}

void DesktopCleanBehavior::activate()
{
	if (!m_bInitialized)
		return;

	if (m_pLasso && m_pLasso->readyToCheck())
	{
		checkPoints();
		m_pLasso->reset();
	}
}

unsigned int DesktopCleanBehavior::checkPoints()
{
	bool hit = false;

	for (auto &ds : m_pDataVolume->getDatasets())
	{
		SonarPointCloud* cloud = static_cast<SonarPointCloud*>(ds);

		for (unsigned int i = 0u; i < cloud->getPointCount(); ++i)
		{
			glm::vec3 in = m_pDataVolume->convertToWorldCoords(cloud->getRawPointPosition(i));
			glm::vec3 out = glm::project(in, m_pDesktop3DViewInfo->view, m_pDesktop3DViewInfo->projection, m_ivec4Viewport);

			if (out.z > 1.f)
				continue;

			if (m_pLasso->checkPoint(glm::vec2(out)))
			{
				cloud->markPoint(i, 1);
				hit = true;
			}
		}
		if (hit)
			cloud->setRefreshNeeded();
	}

	return hit;
}