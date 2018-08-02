#pragma once

#include <GL/glew.h>

#include <math.h>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <future>

#include <glm.hpp>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "FlowGrid.h"
#include "LightingSystem.h"

class FlowVolume
	: public DataVolume
{
public:
	FlowVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth);
	virtual ~FlowVolume();

	void draw();

	void recalcVolumeBounds();

	void preRenderUpdates();

	void setParticleVelocityScale(float velocityScale);
	float getParticleVelocityScale();

	IllustrativeParticleEmitter* placeDyeEmitterWorldCoords(glm::vec3 pos);
	bool removeDyeEmitterClosestToWorldCoords(glm::vec3 pos);

private:
	float m_fFlowRoomTime;
	float m_fFlowRoomMinTime, m_fFlowRoomMaxTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTimeUpdate;
	std::chrono::duration<float, std::milli> m_msLoopTime;
		
	std::vector<FlowGrid*> m_vpFlowGrids;
			
	IllustrativeParticleSystem *m_pParticleSystem;
	bool m_bParticleSystemUpdating;

	std::future<void> m_Future;
};