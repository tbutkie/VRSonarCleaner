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

StudyEditTutorial::StudyEditTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_uiBadPoint1(62474u)
	, m_uiBadPoint2(14274u)
	, m_uiBadPoint3(1540u)
	, m_uiBadPoint4(38694u)
{
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
	
	InfoBoxManager::getInstance().addInfoBox(
		"Edit Tool Info",
		"studyedittoolinstructions.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.1f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Cloud Editing Tutorial",
		"studyeditpointstut.png",
		1.5f,
		glm::translate(glm::mat4(), glm::vec3(2.f, m_pTDM->getHMDToWorldTransform()[3].y, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		true);

	//makeBadDataLabels(0.25f);

	BehaviorManager::getInstance().addBehavior("Scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	BehaviorManager::getInstance().addBehavior("Grab", new GrabDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	BehaviorManager::getInstance().addBehavior("Editing", new PointCleanProbe(m_pTDM, m_pDemoVolume, vr::VRSystem()));
	//static_cast<PointCleanProbe*>(BehaviorManager::getInstance().getBehavior("Editing"))->activateDemoMode();

	m_bInitialized = true;
}

void StudyEditTutorial::update()
{
	if (!m_pTDM->getPrimaryController())
		return;
	
	m_pDemoVolume->update();
	m_pDemoCloud->update();

	//updateBadDataLabels(0.25f);

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
	else if (m_pTDM->getPrimaryController()->isTriggerClicked() || m_pTDM->getPrimaryController()->justUnclickedTrigger())
	{
		bool allDone = true;

		for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
		{
			if (m_pDemoCloud->getPointDepthTPU(i) == 1.f && m_pDemoCloud->getPointMark(i) != 1)
			{
				allDone = false;
				break;
			}
		}
		if (allDone)
		{
			BehaviorManager::getInstance().removeBehavior("Scale");
			BehaviorManager::getInstance().removeBehavior("Grab");
			BehaviorManager::getInstance().removeBehavior("Editing");

			cleanupBadDataLabels();
			InfoBoxManager::getInstance().removeInfoBox("Cloud Editing Tutorial");
			InfoBoxManager::getInstance().removeInfoBox("Edit Tool Info");

			m_pDemoVolume->resetPositionAndOrientation();

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}

	//std::cout.precision(std::numeric_limits<double>::max_digits10);
	//
	//if (m_pTDM->getSecondaryController()->isTouchpadClicked())
	//	for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
	//		if (m_pDemoCloud->getPointMark(i) >= 100)
	//			std::cout << i << ": (" << m_pDemoCloud->getRawPointPosition(i).x << ", " << m_pDemoCloud->getRawPointPosition(i).y << ", " << m_pDemoCloud->getRawPointPosition(i).z << ")" << std::endl;
}

void StudyEditTutorial::draw()
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

		cleanupBadDataLabels();
		InfoBoxManager::getInstance().removeInfoBox("Cloud Editing Tutorial");
		InfoBoxManager::getInstance().removeInfoBox("Edit Tool Info");

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

void StudyEditTutorial::makeBadDataLabels(float width)
{
	float offset = width * 0.5f;

	glm::dmat4 dataXform = m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud);
		
	glm::vec3 pos1 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint1), 1.f));
	glm::vec3 pos2 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint2), 1.f));
	glm::vec3 pos3 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint3), 1.f));
	glm::vec3 pos4 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint4), 1.f));
	
	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 1",
		"baddataleftlabel.png",
		width,
		glm::translate(glm::mat4(), pos1) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 2",
		"baddatarightlabel.png",
		width,
		glm::translate(glm::mat4(), pos1) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 3",
		"baddatarightlabel.png",
		width,
		glm::translate(glm::mat4(), pos2) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);
	
	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 4",
		"baddataleftlabel.png",
		width,
		glm::translate(glm::mat4(), pos2) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 5",
		"baddatarightlabel.png",
		width,
		glm::translate(glm::mat4(), pos3) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 6",
		"baddataleftlabel.png",
		width,
		glm::translate(glm::mat4(), pos3) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 7",
		"baddataleftlabel.png",
		width,
		glm::translate(glm::mat4(), pos4) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Bad Data Label 8",
		"baddatarightlabel.png",
		width,
		glm::translate(glm::mat4(), pos4) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		false);
}

void StudyEditTutorial::updateBadDataLabels(float width)
{
	float offset = width * 0.5f;

	glm::dmat4 dataXform = m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud);

	glm::vec3 pos1 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint1), 1.f));
	glm::vec3 pos2 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint2), 1.f));
	glm::vec3 pos3 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint3), 1.f));
	glm::vec3 pos4 = glm::vec3(dataXform * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(m_uiBadPoint4), 1.f));

	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 1", glm::translate(glm::mat4(), pos1) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 2", glm::translate(glm::mat4(), pos1) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 3", glm::translate(glm::mat4(), pos2) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 4", glm::translate(glm::mat4(), pos2) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 5", glm::translate(glm::mat4(), pos3) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 6", glm::translate(glm::mat4(), pos3) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 7", glm::translate(glm::mat4(), pos4) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(offset, 0.f, 0.f)));
	InfoBoxManager::getInstance().updateInfoBoxPose("Bad Data Label 8", glm::translate(glm::mat4(), pos4) * glm::mat4_cast(m_pDemoVolume->getOrientation()) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::translate(glm::mat4(), glm::vec3(-offset, 0.f, 0.f)));
}

void StudyEditTutorial::cleanupBadDataLabels()
{
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 1");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 2");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 3");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 4");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 5");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 6");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 7");
	InfoBoxManager::getInstance().removeInfoBox("Bad Data Label 8");
}