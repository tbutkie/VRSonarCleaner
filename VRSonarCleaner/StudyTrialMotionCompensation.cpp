#include "StudyTrialMotionCompensation.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include "Renderer.h"
#include "DataLogger.h"

#include <filesystem>
#include <sstream>

using namespace std::chrono;

StudyTrialMotionCompensation::StudyTrialMotionCompensation(TrackedDeviceManager* pTDM, std::string fileName, std::string category)
	: m_pTDM(pTDM)
	, m_bInitialColorRefresh(false)
	, m_bCompensation(false)
	, m_strFileName(fileName)
	, m_strCategory(category)
	, m_nPointsLeft(0u)
{
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pPointCloud = new SonarPointCloud(m_pColorScaler, fileName, SonarPointCloud::SONAR_FILETYPE::XYZF);

	glm::vec3 up(0.f, 1.f, 0.f);
	glm::vec3 hmdForward = -m_pTDM->getHMDToWorldTransform()[2];
	glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];

	glm::vec3 forward = glm::normalize(glm::cross(glm::normalize(glm::cross(up, hmdForward)), up));

	m_pDataVolume = new DataVolume(hmdPos + forward * 1.5f, glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.f));

	m_pDataVolume->add(m_pPointCloud);

	m_pPointCloud->setRefreshNeeded();

	m_pColorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pPointCloud->getMinDepthTPU(),
		m_pPointCloud->getMaxDepthTPU(),
		m_pPointCloud->getMinPositionalTPU(),
		m_pPointCloud->getMaxPositionalTPU()
	);

	m_tpLastUpdate = high_resolution_clock::now();
}


StudyTrialMotionCompensation::~StudyTrialMotionCompensation()
{
	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pDataVolume)
		delete m_pDataVolume;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTrialMotionCompensation::init()
{
	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));
}

void StudyTrialMotionCompensation::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	unsigned int prevPointCount = m_nPointsLeft;
	m_nPointsLeft = m_nCleanedGoodPoints = m_nPointsCleaned = 0u;

	for (unsigned int i = 0; i < m_pPointCloud->getPointCount(); ++i)
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

	if (m_pTDM->getTracker())
	{
		glm::mat4 matTracker = m_pTDM->getTracker()->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		GrabObjectBehavior* grab = static_cast<GrabObjectBehavior*>(BehaviorManager::getInstance().getBehavior("grab"));
		if (grab && !grab->isBeingRotated())
		{
			m_mat4TrackerToVolumeOffset = glm::inverse(m_pTDM->getTracker()->getDeviceToWorldTransform()) * m_pDataVolume->getTransformVolume();
		}

		glm::mat4 matTrackerToVolume = matTracker * glm::translate(glm::mat4(), glm::vec3(0.f, 0.5f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		m_pDataVolume->setPosition(matTrackerToVolume[3]);
		m_pDataVolume->setOrientation(matTrackerToVolume);

		if (m_bCompensation)
			Renderer::getInstance().setSkyboxTransform(glm::mat4());
		else
			Renderer::getInstance().setSkyboxTransform(matTracker * glm::translate(glm::mat4(), glm::vec3(0.f, -1.f, 0.f)));
	}
}

void StudyTrialMotionCompensation::draw()
{
	glm::mat4 matTracker, matTrackerToPlatform, matTrackerToVolume;
	
	if (m_pTDM->getTracker())
	{
		matTracker = m_pTDM->getTracker()->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		matTrackerToVolume = matTracker * glm::translate(glm::mat4(), glm::vec3(0.f, 1.f, 0.f));
	}
	// Platform
	Renderer::getInstance().drawPrimitive("plane", matTracker * glm::translate(glm::mat4(), glm::vec3(0.f, -1.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(1.5f, 3.f, 1.f)), glm::vec4(0.2f, 0.2f, 0.2f, 1.f), glm::vec4(1.f), 132.f);

	//m_pDataVolume->setBackingColor(glm::vec4(0.15f, 0.21f, 0.31f, 1.f));
	//m_pDataVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 2.f);
	m_pDataVolume->drawBBox(0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "instanced";
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("disc");
	rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("disc");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("disc");
	rs.instanced = true;
	rs.specularExponent = 0.1f;

	bool unloadedData = false;
	std::vector<SonarPointCloud*> clouds;
	for (auto &cloud : m_pDataVolume->getDatasets())
	{
		if (!static_cast<SonarPointCloud*>(cloud)->ready())
		{
			unloadedData = true;
			continue;
		}
		clouds.push_back(static_cast<SonarPointCloud*>(cloud));
		rs.VAO = static_cast<SonarPointCloud*>(cloud)->getVAO();
		rs.modelToWorldTransform = m_pDataVolume->getTransformDataset(cloud);
		rs.instanceCount = static_cast<SonarPointCloud*>(cloud)->getPointCount();
		Renderer::getInstance().addToDynamicRenderQueue(rs);
	}


	if (!unloadedData && !m_bInitialColorRefresh)
	{
		refreshColorScale(m_pColorScaler, clouds);
		m_bInitialColorRefresh = true;
	}

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

void StudyTrialMotionCompensation::toggleCompensation()
{
	m_bCompensation = !m_bCompensation;

	std::stringstream ss;
	ss << "Motion Compensation = " << m_bCompensation;
	Renderer::getInstance().drawUIText(
		ss.str(),
		glm::vec4(1.f),
		glm::vec3(0.f),
		glm::quat(),
		10.f,
		Renderer::TextSizeDim::HEIGHT
	);
}



void StudyTrialMotionCompensation::refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds)
{
	if (clouds.size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	colorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	colorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : clouds)
		cloud->resetAllMarks();
}