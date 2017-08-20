#pragma once

#include <GL/glew.h>

#include <math.h>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <future>

#include <shared/glm/glm.hpp>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "CoordinateScaler.h"
#include "FlowGrid.h"
#include "LightingSystem.h"

#include "BroadcastSystem.h"

class FlowVolume
	: public DataVolume
{
public:
	FlowVolume(FlowGrid* flowGrid);
	virtual ~FlowVolume();

	void draw();

	void recalcVolumeBounds();

	void preRenderUpdates();

	IllustrativeParticleEmitter* placeDyeEmitterWorldCoords(glm::vec3 pos);
	bool removeDyeEmitterClosestToWorldCoords(glm::vec3 pos);

private:
	CoordinateScaler *m_pScaler;

	float m_fFlowRoomTime;
	float m_fFlowRoomMinTime, m_fFlowRoomMaxTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTimeUpdate;
	std::chrono::duration<float, std::milli> m_msLoopTime;
		
	FlowGrid* m_pFlowGrid;
			
	IllustrativeParticleSystem *m_pParticleSystem;
	bool m_bParticleSystemUpdating;

	std::future<void> m_Future;
};