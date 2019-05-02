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
#include "CosmoGrid.h"
#include "LightingSystem.h"

class CosmoVolume
	: public DataVolume
{
public:
	CosmoVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth, bool fgFile = true);
	virtual ~CosmoVolume();

	void addFlowGrid(std::string fileName);
	void removeFlowGrid(std::string fileName);

	glm::vec3 getFlowWorldCoords(glm::vec3 pt_WorldCoords);

	void draw();

	void recalcVolumeBounds();

	void update();

	void setParticleVelocityScale(float velocityScale);
	float getParticleVelocityScale();

	IllustrativeParticleEmitter* placeDyeEmitterWorldCoords(glm::vec3 pos);
	bool removeDyeEmitterClosestToWorldCoords(glm::vec3 pos);

	void particleSystemIntegrateEuler();
	void particleSystemIntegrateRK4();

private:
	float m_fFlowRoomTime;
	float m_fFlowRoomMinTime, m_fFlowRoomMaxTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTimeUpdate;
	std::chrono::duration<float, std::milli> m_msLoopTime;
		
	std::vector<CosmoGrid*> m_vpCosmoGrids;
			
	IllustrativeParticleSystem *m_pParticleSystem;
	bool m_bParticleSystemUpdating;

	std::future<void> m_Future;
};