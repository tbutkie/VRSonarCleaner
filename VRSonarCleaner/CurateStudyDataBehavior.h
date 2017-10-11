#pragma once

#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"

#include <filesystem>

class CurateStudyDataBehavior :
	public BehaviorBase
{
public:
	CurateStudyDataBehavior(TrackedDeviceManager* m_pTDM, DataVolume* tableVolume, DataVolume* wallVolume);
	virtual ~CurateStudyDataBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	ColorScaler* m_pColorScaler;
	DataVolume* m_pTableVolume, *m_pWallVolume;
	std::map<std::experimental::filesystem::v1::path, std::vector<SonarPointCloud*>> m_mapDatasetClouds;

	std::vector<std::experimental::filesystem::v1::path> m_vDataPaths;

	std::experimental::filesystem::v1::path m_pathBase;
	std::experimental::filesystem::v1::path m_pathAccept;
	std::experimental::filesystem::v1::path m_pathReject;
	std::experimental::filesystem::v1::path m_pathCurrentDataArea;
	
private:
	void refreshColorScale();
	void savePoints();
	void loadDataset();
	void changeDataset();
};

