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

int pinX = 2521;
int pinY = 1522;

int initX = 1279;
int initY = 802;

int expandX = 0;
int expandY = 23;

auto maxSize = glm::ivec2(pinX - expandX, pinY - expandY);
auto minSize = glm::ivec2(pinX - initX, pinY - initY);
auto winRange = maxSize - minSize;

int clickBoxMinX = 548;
int clickBoxMinY = 271;
int clickBoxMaxX = 680;
int clickBoxMaxY = 441;

int clickBoxRangeX = clickBoxMaxX - clickBoxMinX;
int clickBoxRangeY = clickBoxMaxY - clickBoxMinY;

SonarScene::SonarScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDataVolume(NULL)
	, m_bUseDesktop(false)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_bMiddleMouseDown(false)
	, m_bInitialColorRefresh(false)
	, m_funcWindowEasing(NULL)
	, m_fTransitionRate(0.2f)
	, m_nCurrentSlice(1)
	, m_b3DMode(false)
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
		svi->viewport = glm::ivec4(initX, initY, minSize.x, minSize.y);
	}

	Renderer::getInstance().addTexture(new GLTexture("resources/images/slice1.png", false));
	Renderer::getInstance().addTexture(new GLTexture("resources/images/slice2.png", false));
	Renderer::getInstance().addTexture(new GLTexture("resources/images/slice3.png", false));
	Renderer::getInstance().addTexture(new GLTexture("resources/images/slice4.png", false));
	Renderer::getInstance().addTexture(new GLTexture("resources/images/slice5.png", false));

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
		m_pDataVolume->add(cloud);
	}

	m_vpDataVolumes.push_back(m_pDataVolume);

	//if (m_bUseDesktop)
	//{
	//	DesktopCleanBehavior *tmp = new DesktopCleanBehavior(m_pDataVolume);
	//	BehaviorManager::getInstance().addBehavior("desktop_edit", tmp);
	//	tmp->init();
	//	
	//	Renderer::getInstance().getCamera()->pos = glm::vec3(0.f, 0.f, 1.f);
	//	Renderer::getInstance().getCamera()->up = glm::vec3(0.f, 1.f, 0.f);
	//	Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(Renderer::getInstance().getCamera()->pos, Renderer::getInstance().getCamera()->lookat, Renderer::getInstance().getCamera()->up);
	//}
}

void SonarScene::processSDLEvent(SDL_Event & ev)
{
	ArcBall *arcball = static_cast<ArcBall*>(BehaviorManager::getInstance().getBehavior("arcball"));
	LassoTool *lasso = static_cast<LassoTool*>(BehaviorManager::getInstance().getBehavior("lasso"));

	glm::ivec2 windowSize(Renderer::getInstance().getWindow3DViewInfo()->m_nRenderWidth, Renderer::getInstance().getWindow3DViewInfo()->m_nRenderHeight);
	Renderer::Camera* cam = Renderer::getInstance().getCamera();	

	if (ev.key.keysym.sym >= SDLK_1 && ev.key.keysym.sym <= SDLK_5)
	{
		setView(ev.key.keysym.sym - SDLK_0);
	}


	if (ev.key.keysym.sym == SDLK_f)
	{
		if (m_funcWindowEasing == &easeOut)
		{
			easeOut(0.f, true);
		}
		m_funcWindowEasing = &easeIn;
		m_b3DMode = true;
	}

	if (ev.key.keysym.sym == SDLK_m)
	{
		if (m_funcWindowEasing == &easeIn)
		{
			easeIn(0.f, true);
		}
		m_funcWindowEasing = &easeOut;
		m_b3DMode = false;
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
			Renderer::getInstance().getCamera()->pos = m_pDataVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
			Renderer::getInstance().getCamera()->lookat = m_pDataVolume->getPosition();

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(Renderer::getInstance().getCamera()->pos, Renderer::getInstance().getCamera()->lookat, Renderer::getInstance().getCamera()->up);

			arcball->reset();
		}
	}

	if (ev.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
	{
		if (ev.button.button == SDL_BUTTON_LEFT)
		{
			m_bLeftMouseDown = true;

			int xPos = ev.button.x;
			int yPos = Renderer::getInstance().getPresentationWindowSize().y - ev.button.y;
			
			if (!m_b3DMode &&
				xPos >= clickBoxMinX && xPos <= clickBoxMaxX &&
				yPos > clickBoxMinY && yPos < clickBoxMaxY)
			{
				int sliceSize = clickBoxRangeY / 5;
				int slice = ((yPos - clickBoxMinY) / sliceSize) + 1;

				setView(slice);
			}

		}
		if (ev.button.button == SDL_BUTTON_RIGHT)
		{
			m_bRightMouseDown = true;
		}
		if (ev.button.button == SDL_BUTTON_MIDDLE)
		{
			m_bMiddleMouseDown = true;
		}

	}//end mouse down
	else if (ev.type == SDL_MOUSEBUTTONUP) //MOUSE UP
	{
		if (ev.button.button == SDL_BUTTON_LEFT)
		{
			m_bLeftMouseDown = false;
		}
		if (ev.button.button == SDL_BUTTON_RIGHT)
		{
			m_bRightMouseDown = false;
		}
		if (ev.button.button == SDL_BUTTON_MIDDLE)
		{
			m_bMiddleMouseDown = false;
		}

	}//end mouse up

	if (ev.type == SDL_MOUSEMOTION) //MOUSE MOTION
	{
		int xPos = ev.button.x;
		int yPos = Renderer::getInstance().getPresentationWindowSize().y - ev.button.y;

		if (m_bLeftMouseDown && !m_b3DMode &&
			xPos >= clickBoxMinX && xPos <= clickBoxMaxX &&
			yPos > clickBoxMinY && yPos < clickBoxMaxY)
		{
			int sliceSize = clickBoxRangeY / 5;
			int slice = ((yPos - clickBoxMinY) / sliceSize) + 1;

			setView(slice);
		}
	}

}

void SonarScene::update()
{	
	for (auto &cloud : m_vpClouds)
		cloud->update();

	for (auto &dv : m_vpDataVolumes)
		dv->update();

	if (m_funcWindowEasing && !m_funcWindowEasing(m_fTransitionRate, false))
		m_funcWindowEasing = NULL;

	m_pDataVolume->setOrientation(glm::rotate(m_pDataVolume->getOrientation(), glm::radians(1.f), glm::vec3(0.f, 0.f, 1.f)));
}

void SonarScene::draw()
{
	auto svi = Renderer::getInstance().getMonoInfo();

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "flat";
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("quad");
	rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("quad");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("quad");
	rs.VAO = Renderer::getInstance().getPrimitiveVAO();
	rs.modelToWorldTransform = glm::translate(glm::mat4(), glm::vec3(svi->m_nRenderWidth / 2.f, svi->m_nRenderHeight / 2.f, 0)) * glm::scale(glm::mat4(), glm::vec3(svi->m_nRenderWidth, svi->m_nRenderHeight, 1));
	rs.diffuseTexName = "resources/images/slice" + std::to_string(m_nCurrentSlice) + ".png";
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

		for (auto &cloud : dv->getDatasets())
		{
			if (!static_cast<SonarPointCloud*>(cloud)->ready())
			{
				unloadedData = true;
				continue;
			}

			rs.VAO = static_cast<SonarPointCloud*>(cloud)->getPreviewVAO();
			rs.modelToWorldTransform = dv->getTransformDataset(cloud);
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

bool SonarScene::easeIn(float transitionRate, bool reset)
{
	static float start;

	if (reset)
	{
		start = 0.f;
		return false;
	}

	auto svi = Renderer::getInstance().getMonoInfo();

	auto transitionRatio = 1.f - static_cast<float>(svi->viewport.x) / static_cast<float>(winRange.x);

	if (start == 0.f)
	{
		start = Renderer::getInstance().getElapsedSeconds() - transitionRatio * transitionRate;
	}

	auto elapsed = Renderer::getInstance().getElapsedSeconds() - start;

	auto ratio = elapsed / transitionRate;

	if (ratio <= 1.f)
	{
		//ratio = glm::sin(glm::half_pi<float>() * ratio);

		auto sizeNow = glm::ceil(glm::vec2(minSize) + glm::vec2(winRange) * glm::sin(glm::half_pi<float>() * ratio));

		svi->viewport = glm::ivec4(
			pinX - sizeNow.x,
			pinY - sizeNow.y,
			sizeNow.x,
			sizeNow.y
		);

		return true;
	}

	svi->viewport = glm::ivec4(
		expandX,
		expandY,
		maxSize.x,
		maxSize.y
	);

	start = 0.f;
	return false;
}

bool SonarScene::easeOut(float transitionRate, bool reset)
{
	static float start;

	if (reset)
	{
		start = 0.f;
		return false;
	}

	auto svi = Renderer::getInstance().getMonoInfo();

	auto transitionRatio = static_cast<float>(svi->viewport.x) / static_cast<float>(winRange.x);

	if (start == 0.f)
	{
		start = Renderer::getInstance().getElapsedSeconds() - transitionRatio * transitionRate;
	}

	auto elapsed = Renderer::getInstance().getElapsedSeconds() - start;

	auto ratio = elapsed / transitionRate;

	if (ratio <= 1.f)
	{
		auto sizeNow = glm::ceil(glm::vec2(minSize) + glm::vec2(winRange) * glm::sin(glm::half_pi<float>() * (1.f - ratio)));

		svi->viewport = glm::ivec4(
			pinX - sizeNow.x,
			pinY - sizeNow.y,
			sizeNow.x,
			sizeNow.y
		);

		return true;
	}

	svi->viewport = glm::ivec4(
		initX,
		initY,
		minSize.x,
		minSize.y
	);

	start = 0.f;
	return false;
}

void SonarScene::setView(int sliceNum)
{
	if (m_nCurrentSlice == sliceNum)
		return;

	m_nCurrentSlice = sliceNum;

	auto minBounds = m_pDataVolume->getMinDataBound();
	auto maxBounds = m_pDataVolume->getMaxDataBound();
	auto dims = m_pDataVolume->getDataDimensions();

	switch (sliceNum)
	{
	case 1:
		//m_pDataVolume->setCustomBounds(minBounds, maxBounds);
		//m_pDataVolume->useCustomBounds(true);
		break;
	case 2:
		//m_pDataVolume->useCustomBounds(false);
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	default:
		break;
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