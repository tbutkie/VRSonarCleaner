#pragma once
#include "ProbeBehavior.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include "openvr.h"

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

	unsigned int m_glTorusVAO, m_glTorusVBO, m_glTorusEBO, m_glTorusDiffTex, m_glTorusSpecTex;

	GLsizei m_nTorusVertices;

private:
	void activateProbe();
	void deactivateProbe();

	void checkPoints();

	void generateGeometry();

	void generateTorus();
};

