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
	PointCleanProbe(ViveController* pController, DataVolume* pointCloudVolume, vr::IVRSystem *pHMD);
	virtual ~PointCleanProbe();

	virtual void update();

	virtual void draw();

	bool isProbeActive();
	bool anyHits();

private:
	bool m_bProbeActive;
	bool m_bWaitForTriggerRelease;
	bool m_bAnyHits;
	float m_fPtHighlightAmt;
	unsigned int m_nPointsSelected;

	vr::IVRSystem *m_pHMD;
	
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTime;
	std::chrono::duration<float, std::milli> m_msElapsedTime;

	float m_fCursorHoopAngle;

private:
	void activateProbe();
	void deactivateProbe();

	unsigned int checkPoints();
};

