#include "RunStudyBehavior.h"

#include "StudyTrialStandingBehavior.h"
#include "StudyTrialSittingBehavior.h"
#include "StudyTrialDesktopBehavior.h"

#include <iostream>
#include <random>
#include <algorithm>

using namespace std::experimental::filesystem::v1;


RunStudyBehavior::RunStudyBehavior(TrackedDeviceManager* pTDM, bool sitting)
	: m_eStudyType(sitting ? VR_SITTING : VR_STANDING)
	, m_pTDM(pTDM)
	, m_pDesktop3DViewInfo(NULL)
	, m_pCamera(NULL)
	, m_bTrialsLoaded(false)
{
}

RunStudyBehavior::RunStudyBehavior(Renderer::SceneViewInfo * pSceneInfo, glm::ivec4 & viewport, Renderer::Camera * pCamera)
	: m_eStudyType(DESKTOP)
	, m_pTDM(NULL)
	, m_pDesktop3DViewInfo(pSceneInfo)
	, m_ivec4Viewport(viewport)
	, m_pCamera(pCamera)
	, m_bTrialsLoaded(false)
{
}


RunStudyBehavior::~RunStudyBehavior()
{
	DataLogger::getInstance().closeLog();
}

void RunStudyBehavior::init()
{
	auto basePath = current_path().append(path("resources/data/sonar/study"));

	for (directory_iterator it(basePath); it != directory_iterator(); ++it)
	{
		std::string fileCategory("none");

		if (is_regular_file(*it))
		{
			std::cout << __FUNCTION__ << ": Found study file " << (*it).path().filename() << std::endl;

			m_vStudyDatasets.push_back(std::make_pair(*it, fileCategory));
		}
		else if (is_directory(*it))
		{
			fileCategory = (*it).path().filename().string();

			for (directory_iterator subIt(*it); subIt != directory_iterator(); ++subIt)
			{
				if (is_regular_file(*subIt))
				{
					std::cout << __FUNCTION__ << ": Found " << fileCategory << " study file " << (*subIt).path().filename() << std::endl;

					m_vStudyDatasets.push_back(std::make_pair(*subIt, fileCategory));
				}
			}
		}
	}

	if (m_vStudyDatasets.size() == 0u)
	{
		std::cout << __FUNCTION__ << ": No study files found in " << basePath << std::endl;
		return;
	}

	DataLogger::getInstance().setLogDirectory("logs");
	DataLogger::getInstance().openLog("study", true);
	DataLogger::getInstance().start();

	std::shuffle(m_vStudyDatasets.begin(), m_vStudyDatasets.end(), std::mt19937_64(std::random_device()()));

	for (auto const &ds : m_vStudyDatasets)
		switch (m_eStudyType)
		{
		case RunStudyBehavior::VR_STANDING:
			m_qTrials.push(new StudyTrialStandingBehavior(m_pTDM, ds.first.string(), ds.second));
			break;
		case RunStudyBehavior::VR_SITTING:
			m_qTrials.push(new StudyTrialSittingBehavior(m_pTDM, ds.first.string(), ds.second));
			break;
		case RunStudyBehavior::DESKTOP:
			m_qTrials.push(new StudyTrialDesktopBehavior(m_pDesktop3DViewInfo, m_ivec4Viewport, m_pCamera, ds.first.string(), ds.second));
			break;
		default:
			break;
		}
		

	m_bTrialsLoaded = true;
	m_qTrials.front()->init();
}

void RunStudyBehavior::update()
{	
	if (!m_bTrialsLoaded || m_qTrials.size() == 0u)
		return;

	if (!m_qTrials.front()->isActive())
	{
		delete m_qTrials.front();
		m_qTrials.pop();

		if (m_qTrials.size() > 0u)
			m_qTrials.front()->init();
		else
			DataLogger::getInstance().closeLog();
	}
	else
		m_qTrials.front()->update();
}

void RunStudyBehavior::draw()
{
	if (m_bTrialsLoaded && m_qTrials.size() > 0u)
		m_qTrials.front()->draw();
}

void RunStudyBehavior::next()
{
	if (m_eStudyType == DESKTOP && m_qTrials.size() > 0u)
		static_cast<StudyTrialDesktopBehavior*>(m_qTrials.front())->finish();
}
