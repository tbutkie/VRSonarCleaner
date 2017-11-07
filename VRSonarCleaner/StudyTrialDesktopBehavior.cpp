#include "StudyTrialDesktopBehavior.h"
#include "BehaviorManager.h"
#include "Renderer.h"
#include "DataLogger.h"
#include "DesktopCleanBehavior.h"

#include <filesystem>
#include <sstream>

#include <gtc/random.hpp>

using namespace std::chrono;

StudyTrialDesktopBehavior::StudyTrialDesktopBehavior(Renderer::SceneViewInfo *sceneinfo, glm::ivec4 &viewport, Renderer::Camera *cam, std::string fileName, std::string category)
	: m_pDesktop3DViewInfo(sceneinfo)
	, m_ivec4Viewport(viewport)
	, m_pCamera(cam)
	, m_strFileName(fileName)
	, m_strCategory(category)
	, m_nPointsLeft(0u)
	, m_bPointsCleaned(false)
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

	m_tpLastUpdate = high_resolution_clock::now();
}


StudyTrialDesktopBehavior::~StudyTrialDesktopBehavior()
{
	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pDataVolume)
		delete m_pDataVolume;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTrialDesktopBehavior::init()
{
	using namespace std::experimental::filesystem::v1;
	std::cout << "Starting trial: " << path(m_strFileName).filename() << std::endl;
	DesktopCleanBehavior *dcb = new DesktopCleanBehavior(m_pDataVolume, m_pDesktop3DViewInfo, m_ivec4Viewport);
	BehaviorManager::getInstance().addBehavior("desktop_edit", dcb);
	dcb->init();

	std::stringstream ss;

	ss << "Trial Begin" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
	ss << "\t";
	ss << "trial-type:\"desktop\"";
	ss << ";";
	ss << "file-name:\"" << path(m_strFileName).filename() << "\"";
	ss << ";";
	ss << "file-category:\"" << m_strCategory << "\"";
	ss << ";";
	ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
	ss << ";";
	ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
	ss << ";";
	ss << "vol-dims:\"" << m_pDataVolume->getDimensions().x << "," << m_pDataVolume->getDimensions().y << "," << m_pDataVolume->getDimensions().z << "\"";
	ss << ";";
	ss << "cam-pos:\"" << m_pCamera->pos.x << "," << m_pCamera->pos.y << "," << m_pCamera->pos.z << "\"";
	ss << ";";
	ss << "cam-lookat:\"" << m_pCamera->lookat.x << "," << m_pCamera->lookat.y << "," << m_pCamera->lookat.z << "\"";

	DataLogger::getInstance().logMessage(ss.str());
}

void StudyTrialDesktopBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	if (!m_bPointsCleaned)
	{
		unsigned int prevPointCount = m_nPointsLeft;
		m_nPointsLeft = m_nCleanedGoodPoints = m_nPointsCleaned = 0u;

		for (int i = 0; i < m_pPointCloud->getPointCount(); ++i)
		{
			if (m_pPointCloud->getPointDepthTPU(i) == 1.f)
			{
				if (m_pPointCloud->getPointMark(i) == 1)
					m_nPointsCleaned++;
				else
					m_nPointsLeft++;
			}
			else if (m_pPointCloud->getPointMark(i) == 1)
				m_nCleanedGoodPoints++;
		}

		//if (m_nPointsLeft != prevPointCount)
		//	m_vPointUpdateAnimations.push_back(std::make_pair(m_nPointsLeft, high_resolution_clock::now()));

		if (m_nPointsLeft == 0u)
		{
			BehaviorManager::getInstance().removeBehavior("desktop_edit");

			m_bPointsCleaned = true;

			std::stringstream ss;

			ss << "Trial End" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
			ss << "\t";
			ss << "trial-type:\"desktop\"";
			ss << ";";
			ss << "file-name:\"" << std::experimental::filesystem::v1::path(m_strFileName).filename() << "\"";
			ss << ";";
			ss << "file-category:\"" << m_strCategory << "\"";
			ss << ";";
			ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
			ss << ";";
			ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
			ss << ";";
			ss << "vol-dims:\"" << m_pDataVolume->getDimensions().x << "," << m_pDataVolume->getDimensions().y << "," << m_pDataVolume->getDimensions().z << "\"";
			ss << ";";
			ss << "cam-pos:\"" << m_pCamera->pos.x << "," << m_pCamera->pos.y << "," << m_pCamera->pos.z << "\"";
			ss << ";";
			ss << "cam-lookat:\"" << m_pCamera->lookat.x << "," << m_pCamera->lookat.y << "," << m_pCamera->lookat.z << "\"";

			DataLogger::getInstance().logMessage(ss.str());
		}
	}
}

void StudyTrialDesktopBehavior::draw()
{
	m_pDataVolume->drawVolumeBacking(glm::inverse(m_pDesktop3DViewInfo->view), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 2.f);
	m_pDataVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_POINTS;
	rs.shaderName = "flat";
	rs.VAO = m_pPointCloud->getVAO();
	rs.vertCount = m_pPointCloud->getPointCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.modelToWorldTransform = m_pDataVolume->getCurrentDataTransform(m_pPointCloud);

	Renderer::getInstance().addToDynamicRenderQueue(rs);

	float accuracy = static_cast<float>(m_nPointsCleaned) / static_cast<float>(m_nCleanedGoodPoints + m_nPointsCleaned);
	float accuracyPct = accuracy * 100.f;
	std::stringstream accuracyStr;
	glm::vec4 accColor;

	if (std::isnan(accuracy))
	{
		accuracyStr << "n/a";
		accColor = glm::vec4(glm::vec3(0.7f), 1.f);
	}
	else
	{
		accuracyStr.precision(2);
		accuracyStr << std::fixed << accuracyPct << "%";
		accColor = glm::mix(glm::vec4(0.8f, 0.1f, 0.2f, 1.f), glm::vec4(0.1f, 0.8f, 0.2f, 1.f), accuracy);
	}

	if (!m_bPointsCleaned)
	{
		std::stringstream ss;

		// Point Count Label
		ss << "Bad Points Remaining: " << m_nPointsLeft;

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
			glm::vec3(0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::LEFT,
			Renderer::TextAnchor::BOTTOM_LEFT
		);

		ss = std::stringstream();
		ss << "Accuracy: " << accuracyStr.str();

		Renderer::getInstance().drawUIText(
			ss.str(),
			accColor,
			glm::vec3(0.f, 25.f, 0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::LEFT,
			Renderer::TextAnchor::BOTTOM_LEFT
		);
	}
	else
	{
		Renderer::getInstance().drawUIText(
			"Trial Complete!",
			glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
			glm::vec3(m_ivec4Viewport[2] *0.5f, m_ivec4Viewport[3], 0.f),
			glm::quat(),
			100,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			"Press the <Enter> key to continue...",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(m_ivec4Viewport[2] * 0.5f, m_ivec4Viewport[3] - 110, 0.f),
			glm::quat(),
			50,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			accuracyStr.str(),
			accColor,
			glm::vec3(m_ivec4Viewport[2] * 0.5f, 30.f, 0.f),
			glm::quat(),
			75,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		Renderer::getInstance().drawUIText(
			"Accuracy",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(m_ivec4Viewport[2] * 0.5f, 0.f, 0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);
	}
}

void StudyTrialDesktopBehavior::finish()
{
	if (m_bPointsCleaned)
		m_bActive = false;
}
