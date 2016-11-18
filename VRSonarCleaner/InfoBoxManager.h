#pragma once

#include <map>
#include <tuple>
#include <openvr.h>

#include <shared\glm\glm.hpp>

#include "shared/Texture.h"
#include "Observer.h"
#include "TrackedDeviceManager.h"

#define MAX_N_INFO_BOXES 16

class InfoBoxManager : public Observer
{
public:
	enum RELATIVE_TO {
		WORLD = 0,
		HMD,
		EDIT_CONTROLLER,
		MANIP_CONTROLLER
	};

public:
	// Singleton instance access
	static InfoBoxManager& getInstance();
	
	bool BInit(TrackedDeviceManager* tdm);

	void addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose, RELATIVE_TO what);
	bool removeInfoBox(std::string name);

	virtual void receiveEvent(TrackedDevice* device, const int event);

	void render(const float *matVP);

	bool updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose);
	bool updateInfoBoxSize(std::string infoBoxName, float size);

private:
	InfoBoxManager();
	~InfoBoxManager();

	void createGeometry();
	bool createShaders();

	typedef std::tuple<Texture*, float, glm::mat4, RELATIVE_TO> InfoBoxT;
	typedef std::map<std::string, InfoBoxT> IBMapT;

	IBMapT m_mapInfoBoxes;
	std::map<std::string, Texture*> m_mapTextureBank;

	GLuint m_unTransformProgramID;
	GLint m_nMatrixLocation;
	GLuint m_glVertBuffer;
	GLuint m_unVAO;

	TrackedDeviceManager* m_pTDM;

	enum IBIndex {
		TEXTURE = 0,
		SIZE_METERS,
		TRANSFORM_MATRIX,
		TRANSFORM_RELATION
	};
		
// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	InfoBoxManager(InfoBoxManager const&) = delete;
	void operator=(InfoBoxManager const&) = delete;
};

