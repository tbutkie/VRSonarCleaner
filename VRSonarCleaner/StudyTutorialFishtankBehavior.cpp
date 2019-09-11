#include "StudyTutorialFishtankBehavior.h"
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

StudyTutorialFishtankBehavior::StudyTutorialFishtankBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_nPointsLeft(0u)
	, m_bPointsCleaned(false)
	, m_pDataVolume(dataVolume)
	, m_pColorScaler(NULL)
{
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pPointCloud = new SonarPointCloud(m_pColorScaler, "resources/data/sonar/tutorial_points.csv", SonarPointCloud::SONAR_FILETYPE::XYZF);

	m_tpLastUpdate = high_resolution_clock::now();

	GLuint* shaderHandle = Renderer::getInstance().getShader("instanced");
	if (shaderHandle)
	{
		glUseProgram(*shaderHandle);
		glUniform1f(glGetUniformLocation(*shaderHandle, "size"), 0.0005f);
	}
}


StudyTutorialFishtankBehavior::~StudyTutorialFishtankBehavior()
{
	m_pDataVolume->remove(m_pPointCloud);

	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTutorialFishtankBehavior::init()
{
	m_pDataVolume->resetPositionAndOrientation();
	m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());

	while (!m_pPointCloud->ready())
		Sleep(10);

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

	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));

	std::cout << "Starting tutorial..." << std::endl;
}

void StudyTutorialFishtankBehavior::processEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN)
	{
		if (ev.key.keysym.sym == SDLK_BACKSPACE)
		{
			reset();
		}
	}
}

void StudyTutorialFishtankBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	if (m_pTDM && m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->justUnpressedMenu())
	{
		m_pDataVolume->resetPositionAndOrientation();
		m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());
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
	
	if (m_nPointsLeft == 0u)// || (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedGrip()))
	{
		BehaviorManager::getInstance().removeBehavior("scale");
		BehaviorManager::getInstance().removeBehavior("grab");
		BehaviorManager::getInstance().removeBehavior("edit");

		m_bPointsCleaned = true;

		m_pDataVolume->resetPositionAndOrientation();
		m_pDataVolume->setDimensions(m_pDataVolume->getOriginalDimensions());
	}
}

void StudyTutorialFishtankBehavior::draw()
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
			glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
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

void StudyTutorialFishtankBehavior::reset()
{
	m_pPointCloud->resetAllMarks();

	m_bPointsCleaned = false;

	m_nPointsLeft = m_pPointCloud->getPointCount();

	m_bActive = true;

	BehaviorManager::getInstance().addBehavior("edit", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pDataVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDataVolume));
}

void StudyTutorialFishtankBehavior::finish()
{
	if (m_bPointsCleaned)
		m_bActive = false;
}