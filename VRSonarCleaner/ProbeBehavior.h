#pragma once
#include "SingleControllerBehavior.h"

#include "DataVolume.h"

#include <chrono>

class ProbeBehavior :
	public SingleControllerBehavior
{
public:
	ProbeBehavior(ViveController* controller, DataVolume* dataVolume);
	virtual ~ProbeBehavior();

	void activateDemoMode();
	void deactivateDemoMode();

	glm::vec3 getPosition();
	glm::vec3 getLastPosition();
	glm::mat4 getProbeToWorldTransform();
	glm::mat4 getLastProbeToWorldTransform();

	virtual void update();

	void draw() = 0;

protected:
	DataVolume* m_pDataVolume;

	bool m_bShowProbe;

	const float c_fTouchDeltaThreshold;
	glm::vec2 m_vec2InitialMeasurementPoint; // The touch point from which we will take displacement measurements

	bool m_bVerticalSwipeMode;
	bool m_bHorizontalSwipeMode;

	float m_fProbeOffset;
	float m_fProbeOffsetMin;
	float m_fProbeOffsetMax;
	float m_fProbeInitialOffset;
	glm::vec3 m_vec3ProbeOffsetDirection;

	float m_fProbeRadius; 
	float m_fProbeRadiusMin;
	float m_fProbeRadiusMax;
	float m_fProbeRadiusInitial;

	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fCursorHoopAngle;

	unsigned int m_glProbeVAO, m_glProbeVBO, m_glProbeEBO, m_glProbeDiffTex, m_glProbeSpecTex;

	GLsizei m_nProbeVertices;

protected:
	virtual void receiveEvent(const int event, void* payloadData);
	virtual void activateProbe() = 0;
	virtual void deactivateProbe() = 0;

	virtual void drawProbe(float length);
	
	void generateCylinder(int numSegments);

	std::vector<glm::vec3> makeCircle(int numSegments);
	std::vector<glm::vec3> m_vvec3Circle;
};

