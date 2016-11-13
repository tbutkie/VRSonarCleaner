#pragma once

#include <map>
#include <tuple>
#include <openvr.h>

#include <shared\glm\glm.hpp>

#include "shared/Texture.h"
#include "Observer.h"

#define MAX_N_INFO_BOXES 16

class InfoBoxManager : public Observer
{
public:
	// Singleton instance access
	static InfoBoxManager& getInstance();
	
	void addInfoBox(std::string name, std::string pngFileName, float width, glm::mat4 pose);

	virtual void update();

	void render(const float *matVP);

	bool updateInfoBoxPose(std::string infoBoxName, glm::mat4 pose);

private:
	InfoBoxManager();
	~InfoBoxManager();

	void createGeometry();
	bool createShaders();

	std::map<std::string, std::tuple<Texture*, float, glm::mat4>> m_mapInfoBoxes;
	std::map<std::string, Texture*> m_mapTextureBank;

	GLuint m_unTransformProgramID;
	GLint m_nMatrixLocation;
	GLuint m_glVertBuffer;
	GLuint m_unVAO;
		
// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	InfoBoxManager(InfoBoxManager const&) = delete;
	void operator=(InfoBoxManager const&) = delete;
};

