#include "StudyTrialScene.h"



StudyTrialScene::StudyTrialScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pVFG(NULL)
{
}


StudyTrialScene::~StudyTrialScene()
{
}

void StudyTrialScene::init()
{
	m_pVFG = new VectorFieldGenerator(glm::vec3(0.f, 1.f, 0.f), glm::quat(), glm::vec3(1.f));
	m_pVFG->setGridResolution(32u);
	m_pVFG->setGaussianShape(1.2f);
	m_pVFG->createRandomControlPoints(6u);
	m_pVFG->generate();
}

void StudyTrialScene::processSDLEvent(SDL_Event & ev)
{
}

void StudyTrialScene::update()
{
}

void StudyTrialScene::draw()
{
	m_pVFG->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 1.f);
	m_pVFG->drawBBox(0.f);

	m_pVFG->draw();
}
