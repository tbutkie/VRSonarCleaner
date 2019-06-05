#include "FishTankScene.h"
#include "BehaviorManager.h"
#include <gtc/quaternion.hpp>
#include "utilities.h"
#include <random>


FishTankScene::FishTankScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{

}


FishTankScene::~FishTankScene()
{
}

void FishTankScene::init()
{	
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->pos = glm::vec3(0.f, 0.f, 0.57f);
	cam->lookat = glm::vec3(0.f);
	cam->up = glm::vec3(0.f, 1.f, 0.f);
	
	m_vec3ScreenCenter = glm::vec3(0.f, 0.f, 0.f);
	m_vec3ScreenNormal = glm::vec3(0.f, 0.f, 1.f);
	m_vec3ScreenUp = glm::vec3(0.f, 1.f, 0.f);

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	float width_m = winSize.x * sizer;
	float height_m = winSize.y * sizer;

	Renderer::SceneViewInfo* sviMono = Renderer::getInstance().getMonoInfo();

	sviMono->nearClip = 0.01f;
	sviMono->farClip = 10.f;
	sviMono->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	sviMono->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	sviMono->view = glm::translate(glm::mat4(), -cam->pos);
	sviMono->projection = utils::getViewingFrustum(cam->pos, m_vec3ScreenCenter, m_vec3ScreenNormal, m_vec3ScreenUp, glm::vec2(width_m, height_m));
	sviMono->viewport = glm::ivec4(0, 0, sviMono->m_nRenderWidth, sviMono->m_nRenderHeight);
}

void FishTankScene::processSDLEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
	{		
		if (ev.key.keysym.sym == SDLK_RETURN)
		{

		}		
	}
}

void FishTankScene::update()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();	

	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedTouchpad())
	{
		m_vec3ScreenCenter = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * m_pTDM->getPrimaryController()->c_vec4HoleCenter;
		m_vec3ScreenNormal = -glm::normalize(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * m_pTDM->getPrimaryController()->c_vec4HoleNormal);
		
		cam->pos = m_vec3ScreenCenter + glm::normalize(m_vec3ScreenNormal) * 1.f;
		cam->lookat = m_vec3ScreenCenter;
		cam->up = glm::vec3(0.f, 1.f, 0.f);
	}

	if (m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->isTouchpadClicked())
	{
		cam->pos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
	}

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();
	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	float width_m = winSize.x * sizer;
	float height_m = winSize.y * sizer;

	Renderer::SceneViewInfo* svi = Renderer::getInstance().getMonoInfo();
	svi->view = glm::lookAt(cam->pos, cam->lookat, cam->up);
	svi->projection = utils::getViewingFrustum(cam->pos, m_vec3ScreenCenter, m_vec3ScreenNormal, m_vec3ScreenUp, glm::vec2(width_m, height_m));	
}

void FishTankScene::draw()
{
	Renderer::getInstance().drawPrimitive("cube", glm::translate(glm::mat4(), m_vec3ScreenCenter) * glm::scale(glm::mat4(), glm::vec3(0.1f)), glm::vec4(1.f, 0.f, 0.f, 1.f));

	glm::mat4 ctrTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter));
	glm::mat4 vecTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter + m_pTDM->getPrimaryController()->c_vec4HoleNormal * 0.1f));
	//Renderer::getInstance().drawPrimitive("icosphere", ctrTrans * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f, 1.f, 0.f, 1.f));
	Renderer::getInstance().drawPointerLit(ctrTrans[3], vecTrans[3], 0.01f, glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f));

}