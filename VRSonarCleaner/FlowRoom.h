#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "IllustrativeParticleSystem.h"
#include "StreakletSystem.h"
#include "CoordinateScaler.h"
#include <vector>

#include <stdio.h>
#include <algorithm>
#include "FlowGrid.h"
#include "LightingSystem.h"

#include "BroadcastSystem.h"

#include <shared/glm/glm.hpp>

class FlowRoom
{
public:
	FlowRoom();
	virtual ~FlowRoom();

	void draw();

	void recalcVolumeBounds();

	void preRenderUpdates();

	IllustrativeParticleEmitter* placeDyeEmitterWorldCoords(glm::vec3 pos);
	bool removeDyeEmitterClosestToWorldCoords(glm::vec3 pos);

	void reset();

	DataVolume* getDataVolume();
	IllustrativeParticleSystem* getParticleSystem();

private:
	CoordinateScaler *m_pScaler;

	float m_fFlowRoomTime;
	float m_fFlowRoomMinTime, m_fFlowRoomMaxTime;
	ULONGLONG m_ullLastTimeUpdate;

	DataVolume *m_pMainModelVolume;
		
	std::vector<FlowGrid*> m_vpFlowGridCollection;
			
	IllustrativeParticleSystem *m_pParticleSystem;
	StreakletSystem *m_pStreakletSystem;
};