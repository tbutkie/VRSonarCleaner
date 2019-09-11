#include "StudyTrialFishtankBehavior.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include "Renderer.h"
#include "DataLogger.h"

#include <filesystem>
#include <sstream>

#include <gtc/random.hpp>

using namespace std::chrono;

StudyTrialFishtankBehavior::StudyTrialFishtankBehavior(TrackedDeviceManager* pTDM, std::string fileName, std::string category, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_strFileName(fileName)
	, m_strCategory(category)
	, m_nPointsLeft(0u)
	, m_bPointsCleaned(false)
	, m_pDataVolume(dataVolume)
	, m_pColorScaler(NULL)
{
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pPointCloud = new SonarPointCloud(m_pColorScaler, fileName, SonarPointCloud::SONAR_FILETYPE::XYZF);

	m_tpLastUpdate = high_resolution_clock::now();

	GLuint* shaderHandle = Renderer::getInstance().getShader("instanced");
	if (shaderHandle)
	{
		glUseProgram(*shaderHandle);
		glUniform1f(glGetUniformLocation(*shaderHandle, "size"), 0.001f);
	}
}


StudyTrialFishtankBehavior::~StudyTrialFishtankBehavior()
{
	m_pDataVolume->remove(m_pPointCloud);

	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTrialFishtankBehavior::init()
{
	m_pDataVolume->resetPositionAndOrientation();
	m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());

	while (!m_pPointCloud->ready())
	{
		Sleep(10);
	}

	m_pDataVolume->add(m_pPointCloud);
	
	m_pColorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pPointCloud->getMinDepthTPU(),
		m_pPointCloud->getMaxDepthTPU(),
		m_pPointCloud->getMinPositionalTPU(),
		m_pPointCloud->getMaxPositionalTPU()
	);

	// apply new color scale
	m_pPointCloud->resetAllMarks();
	

	using namespace std::experimental::filesystem::v1;
	std::cout << "Starting trial: " << path(m_strFileName).filename() << std::endl;
	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));

	auto cam = Renderer::getInstance().getCamera();
	glm::vec3 COPPos = cam->pos;
	glm::quat COPQuat = glm::inverse(glm::lookAt(cam->pos, cam->lookat, cam->up));

	std::stringstream ss;

	ss << "Trial Begin" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
	ss << "\t";
	ss << "trial-type:\"fishtank\"";
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
	ss << "hmd-pos:\"" << COPPos.x << "," << COPPos.y << "," << COPPos.z << "\"";
	ss << ";";
	ss << "hmd-quat:\"" << COPQuat.x << "," << COPQuat.y << "," << COPQuat.z << "," << COPQuat.w << "\"";

	if (m_pTDM->getPrimaryController())
	{
		glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
		glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

		ss << ";";
		ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
		ss << ";";
		ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
	}

	if (m_pTDM->getSecondaryController())
	{
		glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
		glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

		ss << ";";
		ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
		ss << ";";
		ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
	}

	DataLogger::getInstance().logMessage(ss.str());
}

void StudyTrialFishtankBehavior::processEvent(SDL_Event & ev)
{
}

void StudyTrialFishtankBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	if (m_pTDM && m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->justUnpressedMenu())
	{
		m_pDataVolume->resetPositionAndOrientation();
		m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());

		if (DataLogger::getInstance().logging())
		{
			std::stringstream ss;

			ss << "View Reset" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();

			DataLogger::getInstance().logMessage(ss.str());
		}
	}

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
	
	if (m_nPointsLeft == 0u && !m_bPointsCleaned)// || (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedGrip()))
	{
		BehaviorManager::getInstance().removeBehavior("scale");
		BehaviorManager::getInstance().removeBehavior("grab");
		BehaviorManager::getInstance().removeBehavior("edit");

		m_bPointsCleaned = true;

		m_pDataVolume->resetPositionAndOrientation();
		m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());

		auto cam = Renderer::getInstance().getCamera();
		glm::vec3 COPPos = cam->pos;
		glm::quat COPQuat = glm::inverse(glm::lookAt(cam->pos, cam->lookat, cam->up));

		std::stringstream ss;

		ss << "Trial End" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "trial-type:\"fishtank\"";
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
		ss << "hmd-pos:\"" << COPPos.x << "," << COPPos.y << "," << COPPos.z << "\"";
		ss << ";";
		ss << "hmd-quat:\"" << COPQuat.x << "," << COPQuat.y << "," << COPQuat.z << "," << COPQuat.w << "\"";

		if (m_pTDM->getPrimaryController())
		{
			glm::vec3 primCtrlrPos = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3];
			glm::quat primCtrlrQuat = glm::quat_cast(m_pTDM->getPrimaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			ss << ";";
			ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		}

		if (m_pTDM->getSecondaryController())
		{
			glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());

			ss << ";";
			ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			ss << ";";
			ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		}

		ss << ";";
		ss << "total-cleaned:\"" << m_nPointsCleaned << "\"";
		ss << ";";
		ss << "total-mistakes:\"" << m_nCleanedGoodPoints << "\"";

		DataLogger::getInstance().logMessage(ss.str());
	}
}

void StudyTrialFishtankBehavior::draw()
{
	if (!m_pTDM->getPrimaryController())
		return;

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
		Renderer::RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "instanced";
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("disc");
		rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("disc");
		rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("disc");
		rs.instanced = true;
		rs.specularExponent = 0.f;

		rs.VAO = static_cast<SonarPointCloud*>(m_pPointCloud)->getVAO();
		rs.modelToWorldTransform = m_pDataVolume->getTransformDataset(m_pPointCloud);
		rs.instanceCount = static_cast<SonarPointCloud*>(m_pPointCloud)->getPointCount();
		Renderer::getInstance().addToDynamicRenderQueue(rs);

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
			glm::vec4(0.2f, 0.2f, 0.8f, 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, Renderer::getInstance().getMonoInfo()->viewport[3], 0.f),
			glm::quat(),
			100,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			"Press both thumbpads to continue...",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, Renderer::getInstance().getMonoInfo()->viewport[3] - 110, 0.f),
			glm::quat(),
			50,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			accuracyStr.str(),
			accColor,
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, 30.f, 0.f),
			glm::quat(),
			75,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		Renderer::getInstance().drawUIText(
			"Accuracy",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, 0.f, 0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);
	}
}

void StudyTrialFishtankBehavior::finish()
{
	if (m_bPointsCleaned)
		m_bActive = false;
}