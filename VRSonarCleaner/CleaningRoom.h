#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>

#include "HolodeckBackground.h"
#include "DataVolume.h"
#include "SonarPointCloud.h"
#include <vector>
#include "CloudCollection.h"

#include <stdio.h>
#include <chrono>
#include <algorithm>

#include <shared/glm/glm.hpp>

extern CloudCollection *clouds;

class CleaningRoom
{
public:
	CleaningRoom(glm::vec3 roomSize);
	virtual ~CleaningRoom();

	float cylTest(const glm::vec4 & pt1, const glm::vec4 & pt2, float lengthsq, float radius_sq, const glm::vec3 & testpt);

	void draw();

	void recalcVolumeBounds();

	//bool checkCleaningTable(const Matrix4 & currentCursorPose, const Matrix4 & lastCursorPose, float radius, unsigned int sensitivity);
	bool editCleaningTable(const glm::mat4 & currentCursorPose, const glm::mat4 & lastCursorPose, float radius, bool clearPoints);

	void resetVolumes();

	DataVolume* getDataVolume();

private:
	DataVolume *wallVolume;
	DataVolume *tableVolume;
	
	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fPtHighlightAmt;
};