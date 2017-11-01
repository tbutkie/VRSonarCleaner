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

	DataLogger::getInstance().setLogDirectory("data");
	DataLogger::getInstance().openLog("study", true);

	std::shuffle(m_vStudyDatasets.begin(), m_vStudyDatasets.end(), std::mt19937_64(std::random_device()()));

	for (auto const &ds : m_vStudyDatasets)
		m_qTrials.push(new StudyTrialBehavior(m_pTDM, ds.first.string(), ds.second));

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
