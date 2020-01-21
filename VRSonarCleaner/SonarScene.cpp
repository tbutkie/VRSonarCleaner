#include "SonarScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "arcball.h"
#include "LassoTool.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "ProbeBehavior.h"
#include "DesktopCleanBehavior.h"

#include <filesystem>

using namespace std::chrono_literals;

SonarScene::SonarScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDataVolume(NULL)
	, m_bUseDesktop(false)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_bMiddleMouseDown(false)
	, m_bInitialColorRefresh(false)
{
}


SonarScene::~SonarScene()
{
	if (m_pDataVolume)
		delete m_pDataVolume;
}

void SonarScene::init()
{
	Renderer::getInstance().setWindowTitle("VR Sonar Cleaner | CCOM VisLab");
	
	glm::vec3 tablePosition = glm::vec3(0.f);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(1.5f, 1.5f, 0.5f);

	m_pDataVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	m_pDataVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pDataVolume->setFrameColor(glm::vec4(1.f));

	{
		auto cam = Renderer::getInstance().getCamera();
		cam->pos = tablePosition + glm::vec3(0.f, 0.f, 1.f) * 2.f;
		cam->lookat = tablePosition;
		cam->up = glm::vec3(0.f, 1.f, 0.f);

		glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();
		float aspect = static_cast<float>(winSize.x) / static_cast<float>(winSize.y);
		
		Renderer::getInstance().setMonoRenderSize(winSize);

		Renderer::SceneViewInfo* svi = Renderer::getInstance().getMonoInfo();
		svi->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
		svi->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
		svi->view = glm::lookAt(cam->pos, cam->lookat, cam->up);
		svi->projection = glm::perspective(glm::radians(60.f), aspect, 0.01f, 100.f);
		svi->viewport = glm::ivec4(0, 0, svi->m_nRenderWidth, svi->m_nRenderHeight);
	}

	GLTexture* tex = Renderer::getInstance().getTexture("resources/images/quadview.png");

	if (tex == NULL)
	{
		tex = new GLTexture("resources/images/quadview.png", false);
		Renderer::getInstance().addTexture(tex);
	}

	glScissor(1280, 800, 1280, 800);

	m_pColorScalerTPU = new ColorScaler();
	//m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	//m_pColorScalerTPU->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);
	m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale);
	m_pColorScalerTPU->setColorMap(ColorScaler::ColorMap::Rainbow);

	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
	//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));


	using namespace std::experimental::filesystem::v1;
	
	//path dataset("south_santa_rosa");
	//path dataset("santa_cruz_south");
	//path dataset("santa_cruz_basin");
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
		m_pDataVolume->add(cloud);
	}

	m_vpDataVolumes.push_back(m_pDataVolume);

	if (m_bUseDesktop)
	{
		DesktopCleanBehavior *tmp = new DesktopCleanBehavior(m_pDataVolume);
		BehaviorManager::getInstance().addBehavior("desktop_edit", tmp);
		tmp->init();
		
		Renderer::getInstance().getCamera()->pos = glm::vec3(0.f, 0.f, 1.f);
		Renderer::getInstance().getCamera()->up = glm::vec3(0.f, 1.f, 0.f);
		Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(Renderer::getInstance().getCamera()->pos, Renderer::getInstance().getCamera()->lookat, Renderer::getInstance().getCamera()->up);
	}
}

void SonarScene::processSDLEvent(SDL_Event & ev)
{
	ArcBall *arcball = static_cast<ArcBall*>(BehaviorManager::getInstance().getBehavior("arcball"));
	LassoTool *lasso = static_cast<LassoTool*>(BehaviorManager::getInstance().getBehavior("lasso"));

	glm::ivec2 windowSize(Renderer::getInstance().getWindow3DViewInfo()->m_nRenderWidth, Renderer::getInstance().getWindow3DViewInfo()->m_nRenderHeight);
	Renderer::Camera* cam = Renderer::getInstance().getCamera();

	if (ev.key.keysym.sym == SDLK_r)
	{
		if (arcball)
		{
			Renderer::getInstance().getCamera()->pos = m_pDataVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
			Renderer::getInstance().getCamera()->lookat = m_pDataVolume->getPosition();

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(Renderer::getInstance().getCamera()->pos, Renderer::getInstance().getCamera()->lookat, Renderer::getInstance().getCamera()->up);

			arcball->reset();
		}
	}

	if (ev.key.keysym.sym == SDLK_SPACE)
	{
		DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));

		if (desktopEdit)
			desktopEdit->activate();
	}

	if (ev.key.keysym.sym == SDLK_r)
	{
		printf("Pressed r, resetting marks\n");

		for (auto &cloud : m_vpClouds)
			cloud->resetAllMarks();
		

		for (auto &dv : m_vpDataVolumes)
			dv->resetPositionAndOrientation();

		if (arcball)
		{
			cam->pos = m_pDataVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
			cam->lookat = m_pDataVolume->getPosition();

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(cam->pos, cam->lookat, cam->up);

			arcball->reset();
		}
	}

	if (ev.key.keysym.sym == SDLK_SPACE)
	{
		DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));

		if (desktopEdit)
			desktopEdit->activate();
	}

	//MOUSE
	if (m_bUseDesktop)
	{
		if (ev.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
		{
			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				m_bLeftMouseDown = true;

				if (arcball)
					arcball->beginDrag(glm::vec2(ev.button.x, windowSize.y - ev.button.y));

				if (lasso)
				{
					if (m_bRightMouseDown)
						lasso->end();

					lasso->reset();
				}

			}
			if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				m_bRightMouseDown = true;
				if (lasso && !m_bLeftMouseDown)
					lasso->start(ev.button.x, windowSize.y - ev.button.y);
			}
			if (ev.button.button == SDL_BUTTON_MIDDLE)
			{
				if (lasso && m_bRightMouseDown)
				{
					lasso->end();
				}
				m_bMiddleMouseDown = true;

				if (arcball)
					arcball->translate(glm::vec2(ev.button.x, windowSize.y - ev.button.y));
			}

		}//end mouse down 
		else if (ev.type == SDL_MOUSEBUTTONUP) //MOUSE UP
		{
			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				m_bLeftMouseDown = false;

				if (arcball)
					arcball->endDrag();

				if (lasso)
					lasso->reset();
			}
			if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				m_bRightMouseDown = false;

				if (lasso)
					lasso->end();
			}
			if (ev.button.button == SDL_BUTTON_MIDDLE)
			{
				m_bMiddleMouseDown = false;
			}

		}//end mouse up
		if (ev.type == SDL_MOUSEMOTION)
		{
			if (m_bLeftMouseDown)
			{
				if (arcball)
					arcball->drag(glm::vec2(ev.button.x, windowSize.y - ev.button.y));
			}
			if (m_bRightMouseDown && !m_bLeftMouseDown)
			{
				if (lasso)
					lasso->move(ev.button.x, windowSize.y - ev.button.y);
			}
		}
		if (ev.type == SDL_MOUSEWHEEL)
		{
			if (lasso)
				lasso->reset();

			glm::vec3 eyeForward = glm::normalize(cam->lookat - cam->pos);
			cam->pos += eyeForward * ((float)ev.wheel.y*0.1f);

			float newLen = glm::length(cam->lookat - cam->pos);

			if (newLen < 0.1f)
				cam->pos = cam->lookat - eyeForward * 0.1f;
			if (newLen > 10.f)
				cam->pos = cam->lookat - eyeForward * 10.f;

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(cam->pos, cam->lookat, cam->up);
		}
	}
}

void SonarScene::update()
{	
	for (auto &cloud : m_vpClouds)
		cloud->update();

	for (auto &dv : m_vpDataVolumes)
		dv->update();
}

void SonarScene::draw()
{
	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("quad");
	rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("quad");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("quad");
	rs.VAO = Renderer::getInstance().getPrimitiveVAO();
	rs.modelToWorldTransform = glm::translate(glm::mat4(), glm::vec3(1280, 800, 0)) * glm::scale(glm::mat4(), glm::vec3(2560, 1600, 1));
	rs.diffuseTexName = "resources/images/quadview.png";
	Renderer::getInstance().addToUIRenderQueue(rs);

	
	bool unloadedData = false;

	for (auto &dv : m_vpDataVolumes)
	{
		if (!dv->isVisible()) continue;

		glm::mat4 trans;

		if (m_bUseDesktop)
			trans = glm::inverse(Renderer::getInstance().getWindow3DViewInfo()->view);

		dv->drawVolumeBacking(trans, 1.f);
		dv->drawBBox(0.f);
		dv->drawAxes(1.f);

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

		for (auto &cloud : dv->getDatasets())
		{
			if (!static_cast<SonarPointCloud*>(cloud)->ready())
			{
				unloadedData = true;
				continue;
			}

			rs.VAO = static_cast<SonarPointCloud*>(cloud)->getVAO();
			rs.modelToWorldTransform = dv->getTransformDataset(cloud);
			rs.instanceCount = static_cast<SonarPointCloud*>(cloud)->getPointCount();
			Renderer::getInstance().addToDynamicRenderQueue(rs);
		}
	}



	if (!unloadedData && !m_bInitialColorRefresh)
	{
		refreshColorScale(m_pColorScalerTPU, m_vpClouds);
		m_bInitialColorRefresh = true;
	}
}


void SonarScene::refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds)
{
	if (clouds.size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	colorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	colorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : clouds)
		cloud->resetAllMarks();
}