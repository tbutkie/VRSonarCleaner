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
	m_vec3ScreenCenter = glm::vec3(-0.2f, 1.f, 0.5f);
	m_vec3ScreenNormal = glm::normalize(glm::vec3(0.2f, -0.5f, 1.f));
	m_vec3ScreenUp = glm::vec3(0.f, 1.f, 0.f);

	calcWorldToScreen();
	
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->pos = m_vec3ScreenCenter + m_vec3ScreenNormal * 0.57f;// +glm::normalize(glm::cross(m_vec3ScreenNormal, m_vec3ScreenUp)) * 0.1f + m_vec3ScreenUp * 0.2f;
	cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	float width_m = winSize.x * sizer;
	float height_m = winSize.y * sizer;

	Renderer::SceneViewInfo* sviMono = Renderer::getInstance().getMonoInfo();

	sviMono->nearClip = 0.01f;
	sviMono->farClip = 10.f;
	sviMono->m_nRenderWidth = winSize.x;
	sviMono->m_nRenderHeight = winSize.y;
	sviMono->view = glm::lookAt(
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f)),
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->lookat, 1.f)),
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->up, 0.f))
	);
	sviMono->projection = utils::getViewingFrustum(m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f), m_vec3ScreenCenter, m_vec3ScreenNormal, m_vec3ScreenUp, glm::vec2(width_m, height_m));
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
		glm::mat4 ctrTrans = m_pTDM->getPrimaryController()->getPose() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter));
		glm::mat4 vecTrans = m_pTDM->getPrimaryController()->getPose() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter - m_pTDM->getPrimaryController()->c_vec4HoleNormal));

		m_vec3ScreenCenter = ctrTrans[3];
		m_vec3ScreenNormal = glm::normalize(vecTrans[3] - ctrTrans[3]);

		calcWorldToScreen();
		
		cam->pos = m_vec3ScreenCenter + m_vec3ScreenNormal * 0.57f;
		cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
	}

	if (m_pTDM->getTracker() && m_pTDM->getSecondaryController() && m_pTDM->getSecondaryController()->isTouchpadClicked())
	{
		cam->pos = m_pTDM->getTracker()->getDeviceToWorldTransform()[3];
		cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
		//cam->up = glm::normalize(m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[1]);
	}

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();
	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	float width_m = winSize.x * sizer;
	float height_m = winSize.y * sizer;

	Renderer::SceneViewInfo* svi = Renderer::getInstance().getMonoInfo();
	//svi->view = glm::lookAt(cam->pos, cam->lookat, cam->up);
	svi->view = glm::lookAt(
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f)),
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->lookat, 1.f)),
		glm::vec3(m_mat4WorldToScreen * glm::vec4(cam->up, 0.f))
	);
	svi->projection = utils::getViewingFrustum(
		m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenCenter, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenNormal, 0.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenUp, 0.f), glm::vec2(width_m, height_m));
}

void FishTankScene::draw()
{
	Renderer::getInstance().drawPrimitive("cube", glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -1.f) * 0.0f) * glm::scale(glm::mat4(), glm::vec3(0.1f)), glm::vec4(1.f, 0.f, 0.f, 1.f));

	if (m_pTDM->getPrimaryController())
	{
		glm::mat4 ctrTrans = m_pTDM->getPrimaryController()->getPose() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter));
		glm::mat4 vecTrans = m_pTDM->getPrimaryController()->getPose() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter + m_pTDM->getPrimaryController()->c_vec4HoleNormal * 0.1f));
		//Renderer::getInstance().drawPrimitive("icosphere", ctrTrans * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f, 1.f, 0.f, 1.f));
		Renderer::getInstance().drawPointerLit(ctrTrans[3], vecTrans[3], 0.01f, glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
	}
}

void FishTankScene::calcWorldToScreen()
{
	glm::vec3 u, v, w;
	u = glm::normalize(glm::cross(m_vec3ScreenUp, m_vec3ScreenNormal));
	v = glm::normalize(glm::cross(m_vec3ScreenNormal, u));
	w = m_vec3ScreenNormal;

	m_mat4ScreenToWorld = glm::mat4(
		glm::vec4(u, 0.f),
		glm::vec4(v, 0.f),
		glm::vec4(w, 0.f),
		glm::vec4(m_vec3ScreenCenter, 1.f)
	);

	m_mat4WorldToScreen = glm::inverse(m_mat4ScreenToWorld);
}
