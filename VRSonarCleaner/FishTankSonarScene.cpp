#include "FishTankSonarScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "PointCleanProbe.h"
#include <gtc/quaternion.hpp>
#include "utilities.h"
#include <random>
#include <filesystem>

FishTankSonarScene::FishTankSonarScene(TrackedDeviceManager* pTDM, float screenDiagInches)
	: m_pTDM(pTDM)
	, m_bHeadTracking(false)
	, m_pTableVolume(NULL)
	, m_vec3COPOffsetTrackerSpace(0.00420141f, -0.0414f, 0.0778124f)
	, m_bInitialColorRefresh(false)
	, m_fScreenDiagonalMeters(screenDiagInches * 0.0254f)
{

}


FishTankSonarScene::~FishTankSonarScene()
{
	if (m_pTableVolume)
		delete m_pTableVolume;
}

void FishTankSonarScene::init()
{
	Renderer::getInstance().toggleSkybox();
	
	m_vec3ScreenCenter = glm::vec3(-0.2f, 1.f, 0.5f);
	m_vec3ScreenNormal = glm::normalize(glm::vec3(0.2f, -0.5f, 1.f));
	m_vec3ScreenUp = glm::vec3(0.f, 1.f, 0.f);

	calcWorldToScreen();

	m_mat4TrackerToEyeCenterOffset = glm::translate(glm::mat4(), m_vec3COPOffsetTrackerSpace);
	
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->pos = m_vec3ScreenCenter + m_vec3ScreenNormal * 0.57f;// +glm::normalize(glm::cross(m_vec3ScreenNormal, m_vec3ScreenUp)) * 0.1f + m_vec3ScreenUp * 0.2f;
	cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = m_fScreenDiagonalMeters / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	m_vec2ScreenSizeMeters.x = winSize.x * sizer;
	m_vec2ScreenSizeMeters.y = winSize.y * sizer;

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
	sviMono->projection = utils::getViewingFrustum(m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f), m_vec3ScreenCenter, m_vec3ScreenNormal, m_vec3ScreenUp, m_vec2ScreenSizeMeters);
	sviMono->viewport = glm::ivec4(0, 0, sviMono->m_nRenderWidth, sviMono->m_nRenderHeight);

	m_pTableVolume = new DataVolume(m_mat4ScreenToWorld[3] - m_mat4ScreenToWorld[2] * (0.39f / 2.f), m_mat4ScreenToWorld, glm::vec3(m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.y));
	m_pTableVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pTableVolume->setFrameColor(glm::vec4(1.f));

	m_pColorScalerTPU = new ColorScaler();
	m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale);
	m_pColorScalerTPU->setColorMap(ColorScaler::ColorMap::Rainbow);

	using namespace std::experimental::filesystem::v1;

	path dataset("davidson_seamount");

	auto basePath = current_path().append(path("resources/data/sonar/nautilus"));

	auto acceptsPath = path(basePath).append(path("accept"));

	for (directory_iterator it(acceptsPath.append(dataset)); it != directory_iterator(); ++it)
	{
		if (is_regular_file(*it))
		{
			if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
			{
				SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
				m_vpClouds.push_back(tmp);
			}
		}
	}

	for (auto const &cloud : m_vpClouds)
	{
		m_pTableVolume->add(cloud);
	}
}

void FishTankSonarScene::processSDLEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
	{		
		if (ev.key.keysym.sym == SDLK_h)
		{
			m_bHeadTracking = !m_bHeadTracking;
			Renderer::getInstance().showMessage("Head tracking set to " + std::to_string(m_bHeadTracking));
		}		
	}
}

void FishTankSonarScene::update()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();	

	if (m_pTDM->getPrimaryController())
	{
		m_pTDM->getPrimaryController()->hideRenderModel();

		if (m_pTDM->getPrimaryController()->justPressedTouchpad())
		{
			glm::mat4 ctrTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter));
			glm::mat4 vecTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter - m_pTDM->getPrimaryController()->c_vec4HoleNormal));

			m_vec3ScreenCenter = ctrTrans[3];
			m_vec3ScreenNormal = glm::normalize(vecTrans[3] - ctrTrans[3]);

			calcWorldToScreen();

			cam->pos = m_vec3ScreenCenter + m_vec3ScreenNormal * 0.57f;
			cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;

			m_pTableVolume->setPosition(m_mat4ScreenToWorld[3] - m_mat4ScreenToWorld[2] * (m_vec2ScreenSizeMeters.x / 2.f));
			m_pTableVolume->setOrientation(m_mat4ScreenToWorld * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
		}
	}

	if (m_pTDM->getSecondaryController())
	{
		m_pTDM->getSecondaryController()->hideRenderModel();

		if (m_pTDM->getSecondaryController()->justPressedGrip() && m_pTDM->getTracker())
		{
			glm::mat4 ctrTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getSecondaryController()->c_vec4HoleCenter));
			glm::vec4 eyeCenterTrackerSpace = glm::inverse(m_pTDM->getTracker()->getDeviceToWorldTransform()) * ctrTrans[3];
			std::cout << "Tracker to Center of Projection Offset: (" << m_mat4TrackerToEyeCenterOffset[3].x << ", " << m_mat4TrackerToEyeCenterOffset[3].y << ", " << m_mat4TrackerToEyeCenterOffset[3].z << ")" << std::endl;
		}
	}
	
	if (m_pTDM->getTracker())
	{
		m_pTDM->getTracker()->hideRenderModel();

		if (m_bHeadTracking)
		{
			cam->pos = (m_pTDM->getTracker()->getDeviceToWorldTransform() * m_mat4TrackerToEyeCenterOffset)[3];
			cam->lookat = cam->pos - glm::dot(cam->pos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
		}
	}

	Renderer::SceneViewInfo* svi = Renderer::getInstance().getMonoInfo();

	svi->view = glm::lookAt(cam->pos, cam->lookat, cam->up);

	svi->projection = utils::getViewingFrustum(
		m_mat4WorldToScreen * glm::vec4(cam->pos, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenCenter, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenNormal, 0.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenUp, 0.f),
		m_vec2ScreenSizeMeters
	);

	if (m_pTDM->getPrimaryController() && !BehaviorManager::getInstance().getBehavior("pointclean"))
		BehaviorManager::getInstance().addBehavior("pointclean", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pTableVolume));

	if (!BehaviorManager::getInstance().getBehavior("grab"))
		BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));

	for (auto &cloud : m_vpClouds)
		cloud->update();

	m_pTableVolume->update();
}

void FishTankSonarScene::draw()
{
	//if (m_pTDM->getPrimaryController())
	//{
	//	glm::mat4 ctrTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter));
	//	glm::mat4 vecTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(m_pTDM->getPrimaryController()->c_vec4HoleCenter + m_pTDM->getPrimaryController()->c_vec4HoleNormal * 0.1f));
	//	//Renderer::getInstance().drawPrimitive("icosphere", ctrTrans * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f, 1.f, 0.f, 1.f));
	//	Renderer::getInstance().drawPointerLit(ctrTrans[3], vecTrans[3], 0.01f, glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
	//}

	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(-m_vec2ScreenSizeMeters.x / 2.f, 0.f, 0.f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(-m_vec2ScreenSizeMeters.x / 2.f, 0.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.f, 0.f, 0.8f, 1.f)
	//);
	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(m_vec2ScreenSizeMeters.x / 2.f, 0.f, 0.f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(m_vec2ScreenSizeMeters.x / 2.f, 0.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.8f, 0.f, 0.f, 1.f)
	//);
	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, -m_vec2ScreenSizeMeters.y / 2.f, 0.f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, -m_vec2ScreenSizeMeters.y / 2.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.f, 0.8f, 0.f, 1.f)
	//);
	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, m_vec2ScreenSizeMeters.y / 2.f, 0.f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, m_vec2ScreenSizeMeters.y / 2.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.8f, 0.f, 0.8f, 1.f)
	//);
	//
	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(-m_vec2ScreenSizeMeters.x / 2.f, 0.f, -0.5f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(m_vec2ScreenSizeMeters.x / 2.f, 0.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.f, 0.8f, 0.8f, 1.f)
	//);
	//Renderer::getInstance().drawDirectedPrimitiveLit(
	//	"cylinder",
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, -m_vec2ScreenSizeMeters.y / 2.f, -0.5f)))[3],
	//	(m_mat4ScreenToWorld * glm::translate(glm::mat4(), glm::vec3(0.f, m_vec2ScreenSizeMeters.y / 2.f, -0.5f)))[3],
	//	0.01f,
	//	glm::vec4(0.8f, 0.8f, 0.f, 1.f)
	//);

	bool unloadedData = false;

	if (m_pTableVolume->isVisible())
	{
		m_pTableVolume->drawVolumeBacking(m_mat4ScreenToWorld, 1.f);
		m_pTableVolume->drawBBox(0.f);
		m_pTableVolume->drawAxes(1.f);

		Renderer::RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "instanced";
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("disc");
		rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("disc");
		rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("disc");
		rs.instanced = true;
		rs.specularExponent = 0.f;
		//rs.diffuseColor = glm::vec4(1.f, 1.f, 1.f, 0.5f);
		//rs.diffuseTexName = "resources/images/circle.png";

		for (auto &cloud : m_pTableVolume->getDatasets())
		{
			if (!static_cast<SonarPointCloud*>(cloud)->ready())
			{
				unloadedData = true;
				continue;
			}

			rs.VAO = static_cast<SonarPointCloud*>(cloud)->getPreviewVAO();
			rs.modelToWorldTransform = m_pTableVolume->getTransformDataset(cloud);
			rs.instanceCount = static_cast<SonarPointCloud*>(cloud)->getPreviewPointCount();
			Renderer::getInstance().addToDynamicRenderQueue(rs);
		}
	}

	if (!unloadedData && !m_bInitialColorRefresh)
	{
		refreshColorScale(m_pColorScalerTPU, m_vpClouds);
		m_bInitialColorRefresh = true;
	}
}

void FishTankSonarScene::calcWorldToScreen()
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

void FishTankSonarScene::refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds)
{
	if (clouds.size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	colorScaler->resetMinMaxForColorScale(m_pTableVolume->getMinDataBound().z, m_pTableVolume->getMaxDataBound().z);
	colorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : clouds)
		cloud->resetAllMarks();
}