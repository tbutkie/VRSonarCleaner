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

	DataLogger::getInstance().setLogDirectory("data");
	m_DataLog = DataLogger::getInstance().openLog("study", true);

	//std::shuffle(m_vStudyDatasets.begin(), m_vStudyDatasets.end(), std::mt19937_64(std::random_device()()));

	for (auto const &ds : m_vStudyDatasets)
		m_qTrials.push(new StudyTrialBehavior(m_pTDM, ds.string(), m_DataLog));

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
			DataLogger::getInstance().closeLog(m_DataLog);
	}
	else
		m_qTrials.front()->update();
}

void RunStudyBehavior::draw()
{
	if (m_bTrialsLoaded && m_qTrials.size() > 0u)
		m_qTrials.front()->draw();
}
