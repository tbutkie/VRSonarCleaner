#include "RunStudyBehavior.h"

#include <iostream>
#include <random>
#include <algorithm>

using namespace std::experimental::filesystem::v1;


RunStudyBehavior::RunStudyBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_bTrialsLoaded(false)
{
}


RunStudyBehavior::~RunStudyBehavior()
{
}

void RunStudyBehavior::init()
{
	auto basePath = current_path().append(path("data/study"));

	for (directory_iterator it(basePath); it != directory_iterator(); ++it)
	{
		if (is_regular_file(*it))
		{
			std::cout << __FUNCTION__ << ": Found study file " << *it << std::endl;

			m_vStudyDatasets.push_back(*it);
		}
	}

	std::shuffle(m_vStudyDatasets.begin(), m_vStudyDatasets.end(), std::mt19937_64(std::random_device()()));

	//m_Future = std::async(std::launch::async, [&] {
		for (auto const &ds : m_vStudyDatasets)
			m_qTrials.push(StudyTrialBehavior(m_pTDM, ds.string()));

		m_bTrialsLoaded = true;
		m_qTrials.front().init();
	//});
}

void RunStudyBehavior::update()
{
	if (!m_bTrialsLoaded && m_Future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
	{
	}
	
	if (!m_bTrialsLoaded || m_qTrials.size() == 0u)
		return;

	if (!m_qTrials.front().isActive())
	{
		m_qTrials.pop();

		if (m_qTrials.size() > 0u)
			m_qTrials.front().init();
	}
	else
		m_qTrials.front().update();
}

void RunStudyBehavior::draw()
{
	if (m_bTrialsLoaded && m_qTrials.size() > 0u)
		m_qTrials.front().draw();
}
