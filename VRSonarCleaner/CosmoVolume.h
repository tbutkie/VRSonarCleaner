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
	CosmoVolume(std::string cosmoDataDir);
	virtual ~CosmoVolume();

	glm::vec3 getFlowWorldCoords(glm::vec3 pt_WorldCoords);

	void draw();

	void recalcVolumeBounds();

	std::vector<glm::vec3> getStreamline(glm::vec3 pos, float propagation_unit, int propagation_max_units, float terminal_speed, bool clipToDomain = true);

	void update();

private:

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastTimeUpdate;
	std::chrono::duration<float, std::milli> m_msLoopTime;
		
	CosmoGrid* m_pCosmoGrid;
};