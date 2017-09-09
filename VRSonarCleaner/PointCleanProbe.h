#pragma once
#include "ProbeBehavior.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "openvr.h"

#define POINT_CLOUD_CLEAN_PROBE_ROTATION_RATE std::chrono::duration<float, std::milli>(2000)
#define POINT_CLOUD_HIGHLIGHT_BLINK_RATE std::chrono::duration<float, std::milli>(250)

class PointCleanProbe :
	public ProbeBehavior
{
public:
	PointCleanProbe(ViveController* controller, DataVolume* pointCloudVolume, vr::IVRSystem *pHMD);
	~PointCleanProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	float m_fPtHighlightAmt;

	vr::IVRSystem *m_pHMD;
	
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTime;
	std::chrono::duration<float, std::milli> m_msElapsedTime;

	float m_fCursorHoopAngle;

private:
	void activateProbe();
	void deactivateProbe();

	void checkPoints();
};

