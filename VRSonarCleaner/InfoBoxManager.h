#pragma once

#include <map>
#include <tuple>
#include <queue>
#include <openvr.h>

#include <shared/glm/glm.hpp>
#include <shared/GLTexture.h>

#include "TrackedDeviceManager.h"

#define MAX_N_INFO_BOXES 16

class InfoBoxManager
{
public:
	enum RELATIVE_TO {
		WORLD = 0,
		HMD,
		PRIMARY_CONTROLLER,
		SECONDARY_CONTROLLER
	};

private:
	enum IBIndex {
		TEXTURE = 0,
		SIZE_METERS,
		TRANSFORM_MATRIX,
		TRANSFORM_RELATION,
		BILLBOARDED
	};

public:
	// Singleton instance access
	static InfoBoxManager& getInstance();
	
	bool BInit(TrackedDeviceManager* tdm);

	void addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose, RELATIVE_TO what, bool billboarded);
	bool removeInfoBox(std::string name);

	void draw();

	bool updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose);
	bool updateInfoBoxSize(std::string infoBoxName, float size);

private:
	InfoBoxManager();
	~InfoBoxManager();

	typedef std::tuple<GLTexture*, float, glm::mat4, RELATIVE_TO, bool> InfoBoxT;
	typedef std::map<std::string, InfoBoxT> IBMapT;

	IBMapT m_mapInfoBoxes;

	TrackedDeviceManager* m_pTDM;

	std::queue<std::vector<InfoBoxT>> m_Tutorial;
		
// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	InfoBoxManager(InfoBoxManager const&) = delete;
	void operator=(InfoBoxManager const&) = delete;
};

