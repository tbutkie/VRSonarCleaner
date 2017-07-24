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

	unsigned int m_glVAO, m_glVBO, m_glEBO;

	GLsizei m_nCylVertices;

private:
	void activateProbe();
	void deactivateProbe();

	void checkPoints();

	void generateGeometry();

	void generateCylinder(int numSegments);
	void generateTorus();
};

