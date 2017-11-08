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
		std::stringstream ss;

		ss << "Lasso Activate" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();

		DataLogger::getInstance().logMessage(ss.str());

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

				if (DataLogger::getInstance().logging())
				{
					std::stringstream ss;

					ss << (cloud->getPointDepthTPU(i) == 1.f) ? "Bad Point Cleaned" : "Good Point Cleaned";
					ss << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
					ss << "\t";
					ss << "point-id:\"" << i << "\"";
					ss << ";";
					ss << "point-pos:\"" << in.x << "," << in.y << "," << in.z << "\"";
					ss << ";";
					ss << "point-pos-screen:\"" << out.x << "," << out.y << "\"";
					ss << ";";
					ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
					ss << ";";
					ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
					ss << ";";
					ss << "vol-dims:\"" << m_pDataVolume->getDimensions().x << "," << m_pDataVolume->getDimensions().y << "," << m_pDataVolume->getDimensions().z << "\"";

					DataLogger::getInstance().logMessage(ss.str());
				}
			}
		}
		if (hit)
			cloud->setRefreshNeeded();
	}

	return hit;
}