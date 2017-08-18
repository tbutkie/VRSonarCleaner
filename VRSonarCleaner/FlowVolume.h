#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "CoordinateScaler.h"
#include <vector>

#include <stdio.h>
#include <algorithm>
#include <chrono>
#include "FlowGrid.h"
#include "LightingSystem.h"

#include "BroadcastSystem.h"

#include <shared/glm/glm.hpp>

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
	//StreakletSystem *m_pStreakletSystem;
};