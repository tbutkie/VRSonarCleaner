#include "MotionCompensationScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "StudyTrialMotionCompensation.h"
#include "StudyTrialNoMotionCompensation.h"
#include <gtc/quaternion.hpp>
#include <experimental/filesystem>
#include <random>
#include "utilities.h"


MotionCompensationScene::MotionCompensationScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_bMotionCompensation(false)
{
}


MotionCompensationScene::~MotionCompensationScene()
{
}

void MotionCompensationScene::init()
{
	using namespace std::experimental::filesystem;

	for (auto &t : m_vTrials)
		delete t;
	m_vTrials.clear();

	auto basePath = current_path().append(path("resources/data/sonar/study"));

	for (directory_iterator it(basePath); it != directory_iterator(); ++it)
	{
		std::string fileCategory("none");

		if (is_regular_file(*it))
		{
			std::cout << __FUNCTION__ << ": Found study file " << (*it).path().filename() << std::endl;

			if (m_bMotionCompensation)
				m_vTrials.push_back(new StudyTrialMotionCompensation(m_pTDM, it->path().string(), fileCategory));
			else
				m_vTrials.push_back(new StudyTrialNoMotionCompensation(m_pTDM, it->path().string(), fileCategory));
		}
		else if (is_directory(*it))
		{
			fileCategory = (*it).path().filename().string();

			for (directory_iterator subIt(*it); subIt != directory_iterator(); ++subIt)
			{
				if (is_regular_file(*subIt))
				{
					std::cout << __FUNCTION__ << ": Found " << fileCategory << " study file " << (*subIt).path().filename() << std::endl;

					if (m_bMotionCompensation)
						m_vTrials.push_back(new StudyTrialMotionCompensation(m_pTDM, subIt->path().string(), fileCategory));
					else
						m_vTrials.push_back(new StudyTrialNoMotionCompensation(m_pTDM, subIt->path().string(), fileCategory));
				}
			}
		}
	}

	std::shuffle(m_vTrials.begin(), m_vTrials.end(), std::mt19937_64(std::random_device()()));

	if (m_vTrials.size() == 0u)
	{
		std::cout << __FUNCTION__ << ": No study files found in " << basePath << std::endl;
		return;
	}

	m_vTrials.back()->init();
}

void MotionCompensationScene::processSDLEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_r)
		init();

	//if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_RETURN && m_vTrials.size() > 0u)
	//	m_vTrials.back()->init();

	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_m)
		m_bMotionCompensation = !m_bMotionCompensation;

	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_s)
		Renderer::getInstance().toggleSkybox();
}

void MotionCompensationScene::update()
{
	if (m_vTrials.size() == 0u)
		return;

	if (!m_vTrials.back()->isActive())
	{
		delete m_vTrials.back();
		m_vTrials.pop_back();

		if (m_vTrials.size() > 0u)
			m_vTrials.back()->init();
	}
	else
		m_vTrials.back()->update();
}

void MotionCompensationScene::draw()
{
	if (m_vTrials.size() > 0u)
		m_vTrials.back()->draw();
	
	//TrackedDevice* pTracker = m_pTDM->getTracker();
	//
	//if (pTracker)
	//{
	//	float size = 0.1f;
	//	glm::vec3 origin = pTracker->getDeviceToWorldTransform()[3];
	//	glm::vec3 u = pTracker->getDeviceToWorldTransform() * glm::vec4(size, 0.f, 0.f, 0.f);
	//	glm::vec3 v = pTracker->getDeviceToWorldTransform() * glm::vec4(0.f, size, 0.f, 0.f);
	//	glm::vec3 w = pTracker->getDeviceToWorldTransform() * glm::vec4(0.f, 0.f, size, 0.f);
	//	float thickness = 0.1f * (glm::length(u) + glm::length(v) + glm::length(w)) / 3.f;
	//
	//	Renderer::getInstance().drawPointerLit(origin, origin + u, thickness, glm::vec4(1.f), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
	//	Renderer::getInstance().drawPointerLit(origin, origin + v, thickness, glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
	//	Renderer::getInstance().drawPointerLit(origin, origin + w, thickness, glm::vec4(1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f));
	//}
}
