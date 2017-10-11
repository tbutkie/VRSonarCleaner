#include "CurateStudyDataBehavior.h"
#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "Renderer.h"

#include "SelectAreaBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include "ScaleDataVolumeBehavior.h"

#include <fstream>
#include <sstream>

using namespace std::experimental::filesystem::v1;


CurateStudyDataBehavior::CurateStudyDataBehavior(TrackedDeviceManager* pTDM, DataVolume* tableVolume, DataVolume* wallVolume)
	: m_pTDM(pTDM)
	, m_pTableVolume(tableVolume)
	, m_pWallVolume(wallVolume)
{
	if (!BehaviorManager::getInstance().getBehavior("select"))
		BehaviorManager::getInstance().addBehavior("select", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
	if (!BehaviorManager::getInstance().getBehavior("grab"))
		BehaviorManager::getInstance().addBehavior("grab", new GrabDataVolumeBehavior(m_pTDM, m_pTableVolume));
	if (!BehaviorManager::getInstance().getBehavior("scale"))
		BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));

	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);
	

	m_vDataPaths.push_back(path("south_santa_rosa"));
	m_vDataPaths.push_back(path("santa_cruz_south"));
	m_vDataPaths.push_back(path("santa_cruz_basin"));
	m_vDataPaths.push_back(path("davidson_seamount"));
	m_vDataPaths.push_back(path("pilgrim_bank"));
	m_vDataPaths.push_back(path("south_san_miguel"));

	m_pathCurrentDataArea = m_vDataPaths.front();

	m_pathBase = current_path().append(path("data"));
	std::cout << "Base study data directory: " << m_pathBase << std::endl;

	m_pathAccept = path(m_pathBase).append(path("accept"));
	m_pathReject = path(m_pathBase).append(path("reject"));	
}


CurateStudyDataBehavior::~CurateStudyDataBehavior()
{

}



void CurateStudyDataBehavior::update()
{
	if (!m_pTDM->getSecondaryController())
		return;

	if (m_pTDM->getSecondaryController()->justPressedTouchpad())
	{
		glm::vec2 touchPadPos = m_pTDM->getSecondaryController()->getCurrentTouchpadTouchPoint();
		if (touchPadPos.y > 0.f)
			changeDataset();
		else
			loadDataset();
	}

	if (m_pTDM->getSecondaryController()->justPressedMenu())
		savePoints();

	m_pTableVolume->update();
	m_pWallVolume->update();

	for (auto &cloud : m_mapDatasetClouds[m_pathCurrentDataArea])
		cloud->update();
}

void CurateStudyDataBehavior::draw()
{
}


bool fileExists(const std::string &fname)
{
	struct stat buffer;
	return (stat(fname.c_str(), &buffer) == 0);
}

std::string intToString(int i, unsigned int pad_to_magnitude)
{
	if (pad_to_magnitude < 1)
		return std::to_string(i);

	std::string ret;

	int mag = i == 0 ? 0 : (int)log10(i);

	for (int j = pad_to_magnitude - mag; j > 0; --j)
		ret += std::to_string(0);

	ret += std::to_string(i);

	return ret;
}

std::string getTimeString()
{
	time_t t = time(0);   // get time now
	struct tm *now = localtime(&t);

	return 	  /*** DATE ***/
		intToString(now->tm_year + 1900, 3) + // year
		"-" + intToString(now->tm_mon + 1, 1) +     // month
		"-" + intToString(now->tm_mday, 1) +        // day
													/*** TIME ***/
		"_" + intToString(now->tm_hour, 1) +        // hour
		"-" + intToString(now->tm_min, 1) +         // minute
		"-" + intToString(now->tm_sec, 1);          // second
}

void CurateStudyDataBehavior::savePoints()
{
	// construct filename
	std::string outFileName("data/study/saved_points_" + m_pathCurrentDataArea.string() + "_" +
		std::to_string(m_pTableVolume->getCustomMinBound().x) + "_" + std::to_string(m_pTableVolume->getCustomMinBound().y) +
		"_" +
		std::to_string(m_pTableVolume->getCustomMaxBound().x) + "_" + std::to_string(m_pTableVolume->getCustomMaxBound().y) +
		".csv");

	// if file exists, keep trying until we find a filename that doesn't already exist
	for (int i = 0; fileExists(outFileName); ++i)
		outFileName = std::string("data/study/saved_points_" + m_pathCurrentDataArea.string() + "_" +
			std::to_string(m_pTableVolume->getCustomMinBound().x) + "_" + std::to_string(m_pTableVolume->getCustomMinBound().y) +
			"_" +
			std::to_string(m_pTableVolume->getCustomMaxBound().x) + "_" + std::to_string(m_pTableVolume->getCustomMaxBound().y) +
			"(" + std::to_string(i + 1) + ")" +
			".csv");

	std::ofstream outFile;
	outFile.open(outFileName);

	if (outFile.is_open())
	{
		std::cout << "Opened file " << outFileName << " for writing output" << std::endl;
		outFile << "x,y,z,flag" << std::endl;
	}
	else
		std::cout << "Error opening file " << outFileName << " for writing output" << std::endl;

	outFile.precision(std::numeric_limits<double>::max_digits10);

	for (auto &ds : m_pTableVolume->getDatasets())
	{
		SonarPointCloud* cloud = static_cast<SonarPointCloud*>(ds);

		for (unsigned int i = 0u; i < cloud->getPointCount(); ++i)
		{
			if (cloud->getPointMark(i) == 1u)
				continue;

			outFile << cloud->getRawPointPosition(i).x << "," << cloud->getRawPointPosition(i).y << "," << cloud->getRawPointPosition(i).z << "," << (cloud->getPointPositionTPU(i) == 1.f ? "1" : "0") << std::endl;
		}
	}

	outFile.close();

	std::cout << "File " << outFileName << " successfully written" << std::endl;
}

void CurateStudyDataBehavior::loadDataset()
{
	for (directory_iterator it(path(m_pathAccept).append(m_pathCurrentDataArea)); it != directory_iterator(); ++it)
	{
		if (is_regular_file(*it))
		{
			if (std::find_if(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_mapDatasetClouds[m_pathCurrentDataArea].end())
			{
				SonarPointCloud* tmp = new SonarPointCloud(m_pColorScaler, (*it).path().string(), SonarPointCloud::QIMERA);
				m_mapDatasetClouds[m_pathCurrentDataArea].push_back(tmp);
				m_pTableVolume->add(tmp);
				m_pWallVolume->add(tmp);
				break;
			}
		}
	}

	for (directory_iterator it(path(m_pathReject).append(m_pathCurrentDataArea)); it != directory_iterator(); ++it)
	{
		if (is_regular_file(*it))
		{
			if (std::find_if(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_mapDatasetClouds[m_pathCurrentDataArea].end())
			{
				SonarPointCloud* tmp = new SonarPointCloud(m_pColorScaler, (*it).path().string(), SonarPointCloud::QIMERA);
				m_mapDatasetClouds[m_pathCurrentDataArea].push_back(tmp);
				m_pTableVolume->add(tmp);
				m_pWallVolume->add(tmp);
				break;
			}
		}
	}

	refreshColorScale();
}

void CurateStudyDataBehavior::changeDataset()
{
	for (auto &ds : m_pTableVolume->getDatasets())
		m_pTableVolume->remove(ds);

	for (auto &ds : m_pWallVolume->getDatasets())
		m_pWallVolume->remove(ds);

	auto it = std::find(m_vDataPaths.begin(), m_vDataPaths.end(), m_pathCurrentDataArea) + 1;

	if (it == m_vDataPaths.end())
		it = m_vDataPaths.begin();

	m_pathCurrentDataArea = *it;

	for (auto &cloud : m_mapDatasetClouds[m_pathCurrentDataArea])
	{
		m_pTableVolume->add(cloud);
		m_pWallVolume->add(cloud);
	}
}

void CurateStudyDataBehavior::refreshColorScale()
{
	if (m_mapDatasetClouds[m_pathCurrentDataArea].size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(m_mapDatasetClouds[m_pathCurrentDataArea].begin(), m_mapDatasetClouds[m_pathCurrentDataArea].end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	m_pColorScaler->resetMinMaxForColorScale(m_pTableVolume->getMinDataBound().z, m_pTableVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : m_mapDatasetClouds[m_pathCurrentDataArea])
		cloud->resetAllMarks();
}