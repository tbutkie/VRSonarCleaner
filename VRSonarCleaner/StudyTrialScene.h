#pragma once
#include "SceneBase.h"

#include "TrackedDeviceManager.h"
#include "VectorFieldGenerator.h"
#include "DataVolume.h"
#include "DataLogger.h"
#include "Renderer.h"

class StudyTrialScene :
	public Scene
{
public:
	StudyTrialScene(TrackedDeviceManager* pTDM);
	~StudyTrialScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	VectorFieldGenerator* m_pVFG;

	std::vector<std::vector<glm::vec3>> m_vvvec3RawStreamlines;
	std::vector<std::vector<glm::vec3>> m_vvvec3TransformedStreamlines;

	GLuint m_glVBO, m_glEBO, m_glVAO;
	GLuint m_glHaloVBO, m_glHaloVAO;
	
	Renderer::RendererSubmission m_rs, m_rsHalo;

	bool m_bShowHalos;

	std::vector<glm::vec3> m_vvec3Zeros;
	std::vector<std::vector<glm::vec3>> m_vvvec3ZeroLines;

private:
	void generateStreamLines();
	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
};

