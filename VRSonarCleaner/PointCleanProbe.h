#pragma once
#include "ProbeBehavior.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "openvr.h"

#define POINT_CLOUD_CLEAN_PROBE_ROTATION_RATE 2000ms
#define POINT_CLOUD_HIGHLIGHT_BLINK_RATE 250ms

class PointCleanProbe :
	public ProbeBehavior
{
public:
	PointCleanProbe(ViveController* controller, DataVolume* pointCloudVolume, SonarPointCloud *pCloud, vr::IVRSystem *pHMD);
	~PointCleanProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;
	
	SonarPointCloud* m_pPointCloud;

	float m_fPtHighlightAmt;

	vr::IVRSystem *m_pHMD;
	
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTime;
	std::chrono::milliseconds m_msElapsedTime;

	float m_fCursorHoopAngle;

private:
	void activateProbe();
	void deactivateProbe();

	void checkPoints();
};

