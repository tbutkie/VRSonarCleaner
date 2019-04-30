#include "FlowScene.h"
#include "BehaviorManager.h"
#include "HairyFlowProbe.h"
#include "FlowProbe.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "FlowFieldCurator.h"

using namespace std::chrono_literals;

FlowScene::FlowScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pFlowVolume(NULL)
	, m_vec3RoomSize(1.f, 3.f, 1.f)
{
}


FlowScene::~FlowScene()
{
	if (m_pFlowVolume)
		delete m_pFlowVolume;
}

void FlowScene::init()
{
	Renderer::getInstance().setWindowTitle("VR Flow 4D | CCOM VisLab");

	std::vector<std::string> flowGrids;

	if (0)
	{
		flowGrids.push_back("resources/data/flowgrid/gb.fg");
		m_pFlowVolume = new FlowVolume(flowGrids, false);
		m_pFlowVolume->setDimensions(glm::vec3(fmin(m_vec3RoomSize.x, m_vec3RoomSize.z) * 0.5f, fmin(m_vec3RoomSize.x, m_vec3RoomSize.z) * 0.5f, m_vec3RoomSize.y * 0.05f));
		m_pFlowVolume->setParticleVelocityScale(0.5f);
	}
	else
	{
		flowGrids.push_back("resources/data/bin/vectors_400.bov");
		m_pFlowVolume = new FlowVolume(flowGrids, true, false);
		m_pFlowVolume->setParticleVelocityScale(0.01f);

	}

	m_pFlowVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pFlowVolume->setFrameColor(glm::vec4(1.f));
}

void FlowScene::processSDLEvent(SDL_Event & ev)
{
	if (m_pFlowVolume)
	{
		if (ev.key.keysym.sym == SDLK_r)
		{
			printf("Pressed r, resetting something...\n");

			m_pFlowVolume->resetPositionAndOrientation();
		}

		if (ev.key.keysym.sym == SDLK_1 && ev.key.keysym.mod & KMOD_SHIFT)
		{
			glm::mat3 matHMD(m_pTDM->getHMDToWorldTransform());
			m_pFlowVolume->setDimensions(glm::vec3(1.f, 1.f, 0.1f));
			m_pFlowVolume->setPosition(glm::vec3(m_pTDM->getHMDToWorldTransform()[3] - m_pTDM->getHMDToWorldTransform()[2] * 0.5f));

			glm::mat3 matOrientation;
			matOrientation[0] = matHMD[0];
			matOrientation[1] = matHMD[2];
			matOrientation[2] = -matHMD[1];
			m_pFlowVolume->setOrientation(glm::quat_cast(matHMD) * glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)));
		}

		if (ev.key.keysym.sym == SDLK_2 && ev.key.keysym.mod & KMOD_SHIFT)
		{
			m_pFlowVolume->setDimensions(glm::vec3(fmin(m_vec3RoomSize.x, m_vec3RoomSize.z) * 0.9f, fmin(m_vec3RoomSize.x, m_vec3RoomSize.z) * 0.9f, m_vec3RoomSize.y * 0.1f));
			m_pFlowVolume->setPosition(glm::vec3(0.f, m_vec3RoomSize.y * 0.1f * 0.5f, 0.f));
			m_pFlowVolume->setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
		}

		if (ev.key.keysym.sym == SDLK_i)
		{
			if (ev.key.keysym.mod & KMOD_SHIFT)
			{
				m_pFlowVolume->particleSystemIntegrateEuler();
				Renderer::getInstance().showMessage("Switched to Euler Forward Integration");
			}
			else
			{
				m_pFlowVolume->particleSystemIntegrateRK4();
				Renderer::getInstance().showMessage("Switched to RK4 Integration");
			}
		}
	}
}

void FlowScene::update()
{
	if (m_pFlowVolume)
	{
		bool flowProbeActive = BehaviorManager::getInstance().getBehavior("flowprobe") != nullptr;

		if (m_pTDM->getPrimaryController())
		{
			if (!flowProbeActive)
				BehaviorManager::getInstance().addBehavior("flowprobe", new FlowProbe(m_pTDM->getPrimaryController(), m_pFlowVolume));
		}
		else if (flowProbeActive)
		{
			BehaviorManager::getInstance().removeBehavior("flowprobe");
		}


		bool hairySliceActive = BehaviorManager::getInstance().getBehavior("hairyslice") != nullptr;

		if (m_pTDM->getSecondaryController())
		{
			if (!hairySliceActive)
				BehaviorManager::getInstance().addBehavior("hairyslice", new HairyFlowProbe(m_pTDM->getSecondaryController(), m_pFlowVolume));
		}
		else if (hairySliceActive)
		{
			BehaviorManager::getInstance().removeBehavior("hairyslice");
		}


		//if (!BehaviorManager::getInstance().getBehavior("flowcurator"))
		//	BehaviorManager::getInstance().addBehavior("flowcurator", new FlowFieldCurator(m_pTDM, m_pFlowVolume));

		//if (!BehaviorManager::getInstance().getBehavior("debugprobe"))
		//	BehaviorManager::getInstance().addBehavior("debugprobe", new DebugProbe(m_pTDM->getPrimaryController(), m_pFlowVolume));

		if (!BehaviorManager::getInstance().getBehavior("grab"))
			BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pFlowVolume));
		if (!BehaviorManager::getInstance().getBehavior("scale"))
			BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pFlowVolume));

		m_pFlowVolume->update();
	}
}

void FlowScene::draw()
{
	if (m_pFlowVolume)
	{
		m_pFlowVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 1.f);
		m_pFlowVolume->drawBBox(0.f);

		m_pFlowVolume->draw();
	}
}
