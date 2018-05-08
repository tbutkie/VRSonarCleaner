#include "StudyEditTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "ScaleDataVolumeBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"
#include "utilities.h"

#include <sstream>

StudyEditTutorial::StudyEditTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_bProbeActivated(false)
	, m_bProbeMinned(false)
	, m_bProbeMaxed(false)
	, m_bProbeExtended(false)
	, m_bProbeRetracted(false)
{
	m_tpTimestamp = std::chrono::high_resolution_clock::now();
}	

StudyEditTutorial::~StudyEditTutorial()
{
	cleanup();
}

void StudyEditTutorial::init()
{
	glm::vec3 tablePosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(1.f, 1.f, 0.5f);

	m_pDemoVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pDemoCloud = new SonarPointCloud(m_pColorScaler, "tutorial_points.csv", SonarPointCloud::XYZF);

	m_pDemoVolume->add(m_pDemoCloud);
	
	refreshColorScale();

	BehaviorManager::getInstance().addBehavior("Scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	BehaviorManager::getInstance().addBehavior("Grab", new GrabDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	m_pProbe = new PointCleanProbe(m_pTDM, m_pDemoVolume, vr::VRSystem());
	BehaviorManager::getInstance().addBehavior("Editing", m_pProbe);

	m_pProbe->lockProbeLength();
	m_pProbe->lockProbeSize();

	m_bInitialized = true;
}

void StudyEditTutorial::update()
{
	if (!m_pTDM->getPrimaryController())
		return;
	
	m_pDemoVolume->update();
	m_pDemoCloud->update();

	if (!m_bProbeActivated)
	{
		m_bProbeActivated = m_pProbe->isProbeActive() && m_nPointsCleaned > 0u;

		if (m_nPointsCleaned > 0u)
		{
			m_pDemoVolume->setVisible(false);
			m_pProbe->unlockProbeSize();
			m_pProbe->disable();
		}
	}
	else if (!m_bProbeMaxed)
	{
		m_bProbeMaxed = m_pProbe->getProbeRadius() == m_pProbe->getProbeRadiusMax();
	}
	else if (!m_bProbeMinned)
	{
		m_bProbeMinned = m_pProbe->getProbeRadius() == m_pProbe->getProbeRadiusMin();

		if (m_bProbeMinned)
		{
			m_pProbe->lockProbeSize();
			m_pProbe->unlockProbeLength();
		}
	}
	else if (!m_bProbeExtended)
	{
		m_bProbeExtended = m_pProbe->getProbeOffset() == m_pProbe->getProbeOffsetMax();
	}
	else if (!m_bProbeRetracted)
	{
		m_bProbeRetracted = m_pProbe->getProbeOffset() == m_pProbe->getProbeOffsetMin();

		if (m_bProbeRetracted)
		{
			m_pProbe->unlockProbeSize();
			m_pProbe->enable();
			m_pDemoVolume->setVisible(true);
		}
	}

	BehaviorBase* done = BehaviorManager::getInstance().getBehavior("Done");
	if (done)
	{
		done->update();
		if (!done->isActive())
			m_bActive = false;
		if (static_cast<TaskCompleteBehavior*>(done)->restartRequested())
		{
			cleanup();
			init();
		}
	}
	else //if (m_pTDM->getPrimaryController()->isTriggerClicked() || m_pTDM->getPrimaryController()->justUnclickedTrigger())
	{
		bool allDone = true;
		m_vvec3BadPoints.clear();

		unsigned int prevPointCount = m_nPointsLeft;
		m_nPointsLeft = m_nCleanedGoodPoints = m_nPointsCleaned = 0u;

		for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
		{
			if (m_pDemoCloud->getPointDepthTPU(i) == 1.f)
			{
				if (m_pDemoCloud->getPointMark(i) == 1)
				{
					m_nPointsCleaned++;
				}
				else
				{
					m_nPointsLeft++;

					allDone = false;
					if (m_vvec3BadPoints.size() <= 5u)
						m_vvec3BadPoints.push_back(m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud) * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(i), 1.f));
				}
			}
			else if (m_pDemoCloud->getPointMark(i) == 1)
				m_nCleanedGoodPoints++;
		}

		if (m_nPointsLeft != prevPointCount)// && std::find_if(m_vPointUpdateAnimations.begin(), m_vPointUpdateAnimations.end(), [&](const std::pair<unsigned int, std::chrono::time_point<high_resolution_clock>> &obj) -> bool { return obj.first == m_nPointsLeft; }) == m_vPointUpdateAnimations.end())
			m_vPointUpdateAnimations.push_back(std::make_pair(m_nPointsLeft, std::chrono::high_resolution_clock::now()));

		if (allDone)
		{
			BehaviorManager::getInstance().removeBehavior("Scale");
			BehaviorManager::getInstance().removeBehavior("Grab");
			BehaviorManager::getInstance().removeBehavior("Editing");

			m_pDemoVolume->resetPositionAndOrientation();

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}
}

void StudyEditTutorial::draw()
{
	if (m_pDemoVolume->isVisible())
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
	
	std::chrono::duration<float> elapsedTime(std::chrono::high_resolution_clock::now() - m_tpTimestamp);
	float cycleTime = 1.f;
	float amt = (sinf(glm::two_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime) + 1.f) * 0.5f;
	glm::vec4 hiliteColor = glm::mix(glm::vec4(0.7f, 0.7f, 0.7f, 1.f), glm::vec4(0.85f, 0.81f, 0.31f, 1.f), amt);

	float dvMaxSide = std::max(std::max(m_pDemoVolume->getDimensions().x, m_pDemoVolume->getDimensions().y), m_pDemoVolume->getDimensions().z);
	float tmp = std::sqrt(dvMaxSide * dvMaxSide * 2.f);
	float dvOffset = std::sqrt(tmp * tmp + dvMaxSide * dvMaxSide) * 0.5f;
	glm::mat4 dvPromptTrans = ccomutils::getBillBoardTransform(m_pDemoVolume->getPosition() + dvOffset * glm::vec3(0.f, 1.f, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

	if (BehaviorManager::getInstance().getBehavior("Done") == nullptr)
	{
		if (m_pDemoVolume->isVisible())
		{
			Renderer::getInstance().drawText(
				"Bad Data Points",
				glm::vec4(0.7f, 0.1f, 0.1f, 1.f),
				dvPromptTrans[3],
				glm::quat(dvPromptTrans),
				dvOffset * 2.f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_BOTTOM
			);

			for (auto &pt : m_vvec3BadPoints)
			{
				Renderer::getInstance().drawConnector(
					dvPromptTrans[3],
					glm::vec3(dvPromptTrans[3]) + (pt - glm::vec3(dvPromptTrans[3])) * 0.999f,
					0.001f,
					glm::vec4(0.7f, 0.7f, 0.7f, 1.f)
				);
			}
		}

		bool rightHanded = m_pTDM->isPrimaryControllerInRighthandPosition();
		glm::mat4 probeEdgeTrans = glm::translate(glm::mat4(), (rightHanded ? -glm::vec3(m_pTDM->getHMDToWorldTransform()[0]) : glm::vec3(m_pTDM->getHMDToWorldTransform()[0])) * m_pProbe->getProbeRadius()) * m_pProbe->getProbeToWorldTransform();
		glm::mat4 probeLabelTrans = glm::translate(glm::mat4(), (rightHanded ? -glm::vec3(m_pTDM->getHMDToWorldTransform()[0]) : glm::vec3(m_pTDM->getHMDToWorldTransform()[0])) * 0.025f + glm::vec3(m_pTDM->getHMDToWorldTransform()[2]) * 0.05f) * probeEdgeTrans;

		glm::mat4 ctrlrLabelTrans = ccomutils::getBillBoardTransform(
			m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3] + 
			(rightHanded ? -m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[0] : m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[0]) * 0.05f +
			m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[1] * 0.025f,
			m_pTDM->getHMDToWorldTransform()[3],
			m_pTDM->getHMDToWorldTransform()[1],
			false
		);

		glm::vec3 triggerPos = m_pTDM->getPrimaryController()->getTriggerPoint();

		if (!m_bProbeActivated)
		{
			std::string grabInitialText("The trigger activates the editing sphere.\nPoints inside the sphere will be cleaned away.\nClean a bad data point using the tool!");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(grabInitialText, 0.5f, Renderer::WIDTH);
			Renderer::getInstance().drawText(
				grabInitialText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				probeLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(probeLabelTrans[3] + (rightHanded ? -probeLabelTrans[0] : probeLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.5f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				triggerPos,
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				probeEdgeTrans[3],
				0.001f,
				hiliteColor
			);
		}
		else if (!m_bProbeMaxed)
		{
			// the size of the tool can also be increased by swiping right on the touchpad
			std::string maxText("The size of the tool can be increased\nby swiping right on the touchpad");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(maxText, 0.05f, Renderer::HEIGHT);
			float osc = sinf(glm::half_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime);
			Renderer::getInstance().drawText(
				maxText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				ctrlrLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(ctrlrLabelTrans[3] + (rightHanded ? -ctrlrLabelTrans[0] : ctrlrLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.05f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::mix(glm::vec3(-0.02023f, 0.00495f, 0.04934f), glm::vec3(0.02023f, 0.00495f, 0.04934f), osc)))[3],
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				m_pProbe->getProbeToWorldTransform()[3] + m_pProbe->getProbeToWorldTransform()[2] * m_pProbe->getProbeRadius(),
				0.001f,
				hiliteColor
			);
		}
		else if (!m_bProbeMinned)
		{
			// the size of the tool can be decreased by swiping left on the touchpad
			std::string minText("... and can be decreased\nby swiping left on the touchpad...");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(minText, 0.05f, Renderer::HEIGHT);
			float osc = sinf(glm::half_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime);
			Renderer::getInstance().drawText(
				minText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				ctrlrLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(ctrlrLabelTrans[3] + (rightHanded ? -ctrlrLabelTrans[0] : ctrlrLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.05f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::mix(glm::vec3(0.02023f, 0.00495f, 0.04934f), glm::vec3(-0.02023f, 0.00495f, 0.04934f) , osc)))[3],
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				m_pProbe->getProbeToWorldTransform()[3] + m_pProbe->getProbeToWorldTransform()[2] * m_pProbe->getProbeRadius(),
				0.001f,
				hiliteColor
			);
		}
		else if (!m_bProbeExtended)
		{
			// the tool can be extended by swiping upwards on the touchpad
			std::string extText("The tool can be extended\nby swiping up on the touchpad");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(extText, 0.05f, Renderer::HEIGHT);
			float osc = sinf(glm::half_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime);
			Renderer::getInstance().drawText(
				extText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				probeLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(probeLabelTrans[3] + (rightHanded ? -probeLabelTrans[0] : probeLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.05f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::mix(glm::vec3(0.f, 0.00265f, 0.06943f), glm::vec3(0.f, 0.00725f, 0.02924f), osc)))[3],
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				probeEdgeTrans[3],
				0.001f,
				hiliteColor
			);
		}
		else if (!m_bProbeRetracted)
		{
			// the tool can also be retracted by swiping downwards on the touchpad
			std::string retText("...and can be retracted\nby swiping down on the touchpad");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(retText, 0.1f, Renderer::HEIGHT);
			float osc = sinf(glm::half_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime);
			Renderer::getInstance().drawText(
				retText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				probeLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(probeLabelTrans[3] + (rightHanded ? -probeLabelTrans[0] : probeLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.1f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::mix(glm::vec3(0.f, 0.00725f, 0.02924f), glm::vec3(0.f, 0.00265f, 0.06943f), osc)))[3],
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				probeLabelTrans[3],
				probeEdgeTrans[3],
				0.001f,
				hiliteColor
			);
		}
		else
		{
			// now clean the rest of the bad data points from the sample set
			float labelSize = 0.025f;
			glm::mat4 statusTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.15f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
			glm::mat4 accuracyTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.175f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

			glm::mat4 instTrans = ccomutils::getBillBoardTransform(glm::vec3(1.f, 1.f, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);
			Renderer::getInstance().drawText(
				"Bad data is colored red\nand should be cleaned away!",
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				instTrans[3],
				glm::quat(instTrans),
				1.f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_MIDDLE
			);

			// the tool can also be retracted by swiping downwards on the touchpad
			std::string retText("Clean the remaining bad data points.\nTry to keep your accuracy high\nby only cleaning red points!\n\nRemember: you can grab and scale\nthe data volume while cleaning!");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(retText, 0.05f, Renderer::HEIGHT);
			Renderer::getInstance().drawText(
				retText,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				ctrlrLabelTrans[3],
				glm::quat(ccomutils::getBillBoardTransform(ctrlrLabelTrans[3] + (rightHanded ? -ctrlrLabelTrans[0] : ctrlrLabelTrans[0]) * dims.x * 0.5f, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false)),
				0.05f,
				Renderer::TextSizeDim::HEIGHT,
				Renderer::TextAlignment::CENTER,
				rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				statusTextAnchorTrans[3],
				0.001f,
				hiliteColor
			);

			Renderer::getInstance().drawConnector(
				ctrlrLabelTrans[3],
				dvPromptTrans[3],
				0.001f,
				hiliteColor
			);

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
				float timeElapsed = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - anim.second).count();
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
				accuracyStr << std::fixed << accuracyPct << "%";
				accColor = glm::mix(glm::vec4(0.8f, 0.1f, 0.2f, 1.f), glm::vec4(0.1f, 0.8f, 0.2f, 1.f), accuracy);
			}

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
	}
	else
	{
		float accuracy = static_cast<float>(m_nPointsCleaned) / static_cast<float>(m_nCleanedGoodPoints + m_nPointsCleaned);
		float accuracyPct = accuracy * 100.f;
		std::stringstream accuracyStr;
		glm::vec4 accColor;

		glm::mat4 accuracyTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.175f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

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

		Renderer::getInstance().drawText(
			"Accuracy: ",
			glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
			dvPromptTrans[3],
			glm::quat(dvPromptTrans),
			0.1f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::BOTTOM_RIGHT
		);
		Renderer::getInstance().drawText(
			accuracyStr.str(),
			accColor,
			dvPromptTrans[3],
			glm::quat(dvPromptTrans),
			0.1f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::BOTTOM_LEFT
		);
	}
}

void StudyEditTutorial::cleanup()
{
	if (m_bInitialized)
	{
		delete m_pDemoCloud;

		delete m_pDemoVolume;

		delete m_pColorScaler;

		BehaviorManager::getInstance().removeBehavior("Scale");
		BehaviorManager::getInstance().removeBehavior("Grab");
		BehaviorManager::getInstance().removeBehavior("Editing");
		BehaviorManager::getInstance().removeBehavior("Done");

		m_bProbeActivated = m_bProbeMinned = m_bProbeMaxed = m_bProbeExtended = m_bProbeRetracted = false;

		m_bInitialized = false;
	}
}

void StudyEditTutorial::refreshColorScale()
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