#include "RunStudyBehavior.h"

#include "BehaviorManager.h"

#include "StudyTrialFishtankBehavior.h"
#include "StudyTrialHMDBehavior.h"
#include "StudyTrialDesktopBehavior.h"

#include <iostream>
#include <random>
#include <algorithm>

#include <gtc/random.hpp>

using namespace std::experimental::filesystem::v1;


RunStudyBehavior::RunStudyBehavior(TrackedDeviceManager* pTDM, EStudyType mode, DataVolume* fishtankVolume)
	: m_eStudyType(mode)
	, m_pTDM(pTDM)
	, m_bTrialsLoaded(false)
	, m_pFishtankVolume(fishtankVolume)
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

	DataLogger::getInstance().setLogDirectory("logs/");
	DataLogger::getInstance().openLog("study", true);
	DataLogger::getInstance().start();

	std::shuffle(m_vStudyDatasets.begin(), m_vStudyDatasets.end(), std::mt19937_64(std::random_device()()));

	for (auto const &ds : m_vStudyDatasets)
		switch (m_eStudyType)
		{
		case RunStudyBehavior::FISHTANK:
			m_qTrials.push(new StudyTrialFishtankBehavior(m_pTDM, ds.first.string(), ds.second, m_pFishtankVolume));
			break;
		case RunStudyBehavior::VR:
			m_qTrials.push(new StudyTrialHMDBehavior(m_pTDM, ds.first.string(), ds.second));
			break;
		case RunStudyBehavior::DESKTOP:
			m_qTrials.push(new StudyTrialDesktopBehavior(ds.first.string(), ds.second));
			break;
		default:
			break;
		}
		

	m_bTrialsLoaded = true;
	m_qTrials.front()->init();
}

void RunStudyBehavior::processSDLEvent(SDL_Event & ev)
{
	if (!m_bTrialsLoaded || m_qTrials.size() == 0u)
		return;

	if (ev.type == SDL_KEYDOWN)
	{
		if (ev.key.keysym.sym == SDLK_RETURN)
		{
			next();
		}
	}

	m_qTrials.front()->processEvent(ev);
}

void RunStudyBehavior::update()
{	
	if (!m_bTrialsLoaded || m_qTrials.size() == 0u)
		return;

	if (m_pTDM && m_pTDM->getPrimaryController() && m_pTDM->getSecondaryController())
	{
		if (m_pTDM->getPrimaryController()->isTouchpadClicked() && m_pTDM->getSecondaryController()->isTouchpadClicked())
			next();
	}

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
	if (m_bTrialsLoaded)
	{
		if (m_qTrials.size() > 0u)
		{
			m_qTrials.front()->draw();
		}
		else
		{
			Renderer::getInstance().drawUIText(
				"COMPLETE!",
				glm::vec4(1.f),
				glm::vec3(glm::vec2(Renderer::getInstance().getUIRenderSize()) * 0.5f, 0.f),
				glm::quat(),
				Renderer::getInstance().getUIRenderSize().x / 2.f,
				Renderer::WIDTH,
				Renderer::CENTER,
				Renderer::CENTER_MIDDLE
			);

			if (m_eStudyType == VR)
			{
				glm::mat4 hmdXform = m_pTDM->getHMDToWorldTransform();
				glm::vec3 hmdForward = -hmdXform[2];
				glm::vec3 hmdUp = hmdXform[1];
				glm::vec3 hmdPos = hmdXform[3];

				Renderer::getInstance().drawText(
					"COMPLETE!",
					glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
					glm::vec3(hmdPos + hmdForward * 1.f),
					glm::quat(hmdXform),
					0.2f,
					Renderer::TextSizeDim::HEIGHT,
					Renderer::TextAlignment::CENTER,
					Renderer::TextAnchor::CENTER_MIDDLE
				);
			}
		}
	}
}

void RunStudyBehavior::next()
{
	// this is so stupid and hacky but it works
	if (m_eStudyType == DESKTOP && m_qTrials.size() > 0u)
		static_cast<StudyTrialDesktopBehavior*>(m_qTrials.front())->finish();
	else if (m_eStudyType == FISHTANK && m_qTrials.size() > 0u)
		static_cast<StudyTrialFishtankBehavior*>(m_qTrials.front())->finish();
	else if (m_eStudyType == VR && m_qTrials.size() > 0u)
		static_cast<StudyTrialHMDBehavior*>(m_qTrials.front())->finish();
}