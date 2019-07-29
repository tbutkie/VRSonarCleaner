#include "FishTankSonarScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include <gtc/quaternion.hpp>
#include "utilities.h"
#include <random>
#include <filesystem>

FishTankSonarScene::FishTankSonarScene(TrackedDeviceManager* pTDM, float screenDiagInches)
	: m_pTDM(pTDM)
	, m_bHeadTracking(false)
	, m_bStereo(true)
	, m_bEditMode(false)
	, m_fEyeSeparationCentimeters(6.2f)
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

	setupViews();

	m_pTableVolume = new DataVolume(m_mat4ScreenToWorld[3] - m_mat4ScreenToWorld[2] * (0.39f / 2.f), m_mat4ScreenToWorld, glm::vec3(m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.y));
	m_pTableVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pTableVolume->setFrameColor(glm::vec4(1.f));

	m_pColorScalerTPU = new ColorScaler();
	m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale);
	m_pColorScalerTPU->setColorMap(ColorScaler::ColorMap::Rainbow);

	using namespace std::experimental::filesystem::v1;

	if (false)
	{
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
	}
	else
	{
		auto basePath = current_path().append(path("resources/data/lidar"));

		for (directory_iterator it(basePath); it != directory_iterator(); ++it)
		{
			if (is_regular_file(*it))
			{
				if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
				{
					SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::LIDAR);
					m_vpClouds.push_back(tmp);
				}
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
		if (m_bEditMode && ev.key.keysym.sym == SDLK_h)
		{
			m_bHeadTracking = !m_bHeadTracking;
			Renderer::getInstance().showMessage("Head tracking set to " + std::to_string(m_bHeadTracking));
		}

		if (ev.key.keysym.sym == SDLK_r)
		{
			m_pTableVolume->setPosition(m_mat4ScreenToWorld[3] - m_mat4ScreenToWorld[2] * (m_vec2ScreenSizeMeters.x / 2.f));
			m_pTableVolume->setOrientation(m_mat4ScreenToWorld * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
			m_pTableVolume->setDimensions(glm::vec3(m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.y));

			for (auto &cloud : m_vpClouds)
				cloud->resetAllMarks();			
		}

		if (m_bEditMode && ev.key.keysym.sym == SDLK_s)
		{
			m_bStereo = !m_bStereo;
		}

		if (ev.key.keysym.sym == SDLK_HOME)
		{
			m_bEditMode = !m_bEditMode;
			Renderer::getInstance().showMessage("Edit mode set to " + std::to_string(m_bEditMode));
		}
	}
}

void FishTankSonarScene::update()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();	

	if (m_pTDM->getPrimaryController())
	{
		m_pTDM->getPrimaryController()->hideRenderModel();

		if (m_bEditMode && m_pTDM->getPrimaryController()->justPressedTouchpad())
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
			m_pTableVolume->setDimensions(glm::vec3(m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.x, m_vec2ScreenSizeMeters.y));
		}

		if (!BehaviorManager::getInstance().getBehavior("pointclean"))
		{
			auto pcp = new PointCleanProbe(m_pTDM->getPrimaryController(), m_pTableVolume);		
			BehaviorManager::getInstance().addBehavior("pointclean", pcp);
			pcp->activateDemoMode();
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

		if (!BehaviorManager::getInstance().getBehavior("grab"))
			BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));
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

	if (m_pTDM->getPrimaryController() && m_pTDM->getSecondaryController())
	{
		if (!BehaviorManager::getInstance().getBehavior("scale"))
			BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));
	}

	setupViews();

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

void FishTankSonarScene::setupViews()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	float eyeSeparationMeters = m_fEyeSeparationCentimeters / 100.f;

	glm::vec3 COP = cam->pos;
	glm::vec3 COPOffset = m_bStereo ? glm::vec3(1.f, 0.f, 0.f) * eyeSeparationMeters * 0.5f : glm::vec3(0.f);

	// Update eye positions using current head position
	glm::vec3 leftEyePos = COP - COPOffset;
	glm::vec3 leftEyeLook = leftEyePos - glm::dot(leftEyePos - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
	glm::vec3 rightEyePos = COP + COPOffset;
	glm::vec3 rightEyeLook = rightEyePos - glm::dot(rightEyeLook - m_vec3ScreenCenter, m_vec3ScreenNormal) * m_vec3ScreenNormal;
	
	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = m_fScreenDiagonalMeters / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);
	
	m_vec2ScreenSizeMeters.x = winSize.x * sizer;
	m_vec2ScreenSizeMeters.y = winSize.y * sizer;

	Renderer::getInstance().setStereoRenderSize(winSize);

	Renderer::SceneViewInfo* sviLE = Renderer::getInstance().getLeftEyeInfo();
	sviLE->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	sviLE->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	sviLE->view = glm::lookAt(leftEyePos, leftEyeLook, cam->up);
	sviLE->projection = utils::getViewingFrustum(
		m_mat4WorldToScreen * glm::vec4(leftEyePos, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenCenter, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenNormal, 0.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenUp, 0.f),
		m_vec2ScreenSizeMeters);
	sviLE->viewport = glm::ivec4(0, 0, sviLE->m_nRenderWidth, sviLE->m_nRenderHeight);

	Renderer::SceneViewInfo* sviRE = Renderer::getInstance().getRightEyeInfo();
	sviRE->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	sviRE->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	sviRE->view = glm::lookAt(rightEyePos, rightEyeLook, cam->up);
	sviRE->projection = utils::getViewingFrustum(
		m_mat4WorldToScreen * glm::vec4(rightEyePos, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenCenter, 1.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenNormal, 0.f),
		m_mat4WorldToScreen * glm::vec4(m_vec3ScreenUp, 0.f),
		m_vec2ScreenSizeMeters);
	sviRE->viewport = glm::ivec4(0, 0, sviRE->m_nRenderWidth, sviRE->m_nRenderHeight);
}