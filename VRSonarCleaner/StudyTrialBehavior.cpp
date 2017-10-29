#include "StudyTrialBehavior.h"
#include "BehaviorManager.h"
#include "GrabDataVolumeBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include "Renderer.h"

#include <filesystem>
#include <sstream>

using namespace std::chrono;

StudyTrialBehavior::StudyTrialBehavior(TrackedDeviceManager* pTDM, std::string fileName, DataLogger::LogHandle log)
	: m_pTDM(pTDM)
	, m_Log(log)
	, m_nPointsLeft(0u)
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
	using namespace std::experimental::filesystem::v1;
	std::cout << "Starting trials for " << path(m_pPointCloud->getName()).filename() << std::endl;
	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM, m_pDataVolume, vr::VRSystem()));
	BehaviorManager::getInstance().addBehavior("grab", new GrabDataVolumeBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));
}

void StudyTrialBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

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

	if (m_nPointsLeft != prevPointCount)// && std::find_if(m_vPointUpdateAnimations.begin(), m_vPointUpdateAnimations.end(), [&](const std::pair<unsigned int, std::chrono::time_point<high_resolution_clock>> &obj) -> bool { return obj.first == m_nPointsLeft; }) == m_vPointUpdateAnimations.end())
		m_vPointUpdateAnimations.push_back(std::make_pair(m_nPointsLeft, high_resolution_clock::now()));

	if (m_nPointsLeft == 0u)// || (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedGrip()))
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

	if (!m_pTDM->getPrimaryController())
		return;

	// Point Count Label
	glm::mat4 statusTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.15f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	float labelSize = 0.025f;

	Renderer::getInstance().drawText(
		std::to_string(m_nPointsLeft),
		glm::vec4(0.8f, 0.1f, 0.2f, 1.f),
		statusTextAnchorTrans[3],
		glm::quat(statusTextAnchorTrans),
		labelSize,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_BOTTOM
	);

	Renderer::getInstance().drawText(
		std::string("Bad Points Left"),
		glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
		statusTextAnchorTrans[3],
		glm::quat(statusTextAnchorTrans),
		0.01f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_TOP
	);

	float animTime = 0.2f;

	for (auto &anim : m_vPointUpdateAnimations)
	{
		float timeElapsed = duration<float>(high_resolution_clock::now() - anim.second).count();
		float timeLeft = animTime - timeElapsed;

		if (timeLeft < 0.f)
			continue;

		float ratio = (animTime - timeLeft) / animTime;

		Renderer::getInstance().drawText(
			std::to_string(anim.first),
			glm::mix(glm::vec4(0.8f, 0.1f, 0.2f, 1.f), glm::vec4(0.1f, 0.8f, 0.2f, 0.f), ratio),
			statusTextAnchorTrans[3] + statusTextAnchorTrans[2] * ratio * 0.1f,
			glm::quat(statusTextAnchorTrans),
			labelSize * (1.f + 1.f * ratio),
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);
	}

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
		accuracyStr << std::fixed << accuracyPct;
		accColor = glm::mix(glm::vec4(0.8f, 0.1f, 0.2f, 1.f), glm::vec4(0.1f, 0.8f, 0.2f, 1.f), accuracy);
	}

	glm::mat4 accuracyTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.175f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

	Renderer::getInstance().drawText(
		"Accuracy: ",
		glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
		accuracyTextAnchorTrans[3],
		glm::quat(accuracyTextAnchorTrans),
		0.01f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_RIGHT
	);
	Renderer::getInstance().drawText(
		accuracyStr.str(),
		accColor,
		accuracyTextAnchorTrans[3],
		glm::quat(accuracyTextAnchorTrans),
		0.01f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_LEFT
	);
}
