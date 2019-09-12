#include "StudyTutorialDesktopBehavior.h"
#include "BehaviorManager.h"
#include "Renderer.h"
#include "DataLogger.h"
#include "DesktopCleanBehavior.h"
#include "utilities.h"

#include <filesystem>
#include <sstream>

#include <gtc/random.hpp>

using namespace std::chrono;

StudyTutorialDesktopBehavior::StudyTutorialDesktopBehavior()
	: m_nPointsLeft(0u)
	, m_bPointsCleaned(false)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_bMiddleMouseDown(false)
{
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);


	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	glm::vec2 vec2ScreenSizeMeters(winSize.x * sizer, winSize.y * sizer);
	
	m_pPointCloud = new SonarPointCloud(m_pColorScaler, "resources/data/sonar/tutorial_points.csv", SonarPointCloud::SONAR_FILETYPE::XYZF);
	
	// point cloud loading is async, but files are small so let them load so we can refresh the color scale
	while (!m_pPointCloud->ready()) Sleep(10);

	m_pDataVolume = new DataVolume(glm::vec3(0.f, 0.f, -vec2ScreenSizeMeters.y * 0.5f), glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)), glm::vec3(vec2ScreenSizeMeters.y));
	m_pDataVolume->setBackingColor(glm::vec4(0.f, 0.f, 0.f, 1.f));
	m_pDataVolume->setFrameColor(glm::vec4(0.f, 0.f, 0.7f, 1.f));
	m_pDataVolume->add(m_pPointCloud);

	m_pColorScaler->resetMinMaxForColorScale(m_pDataVolume->getMinDataBound().z, m_pDataVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pPointCloud->getMinDepthTPU(),
		m_pPointCloud->getMaxDepthTPU(),
		m_pPointCloud->getMinPositionalTPU(),
		m_pPointCloud->getMaxPositionalTPU()
	);

	m_pPointCloud->resetAllMarks();

	m_tpLastUpdate = high_resolution_clock::now();

	GLuint* shaderHandle = Renderer::getInstance().getShader("instanced");
	if (shaderHandle)
	{
		glUseProgram(*shaderHandle);
		glUniform1f(glGetUniformLocation(*shaderHandle, "size"), 0.0005f);
	}
}


StudyTutorialDesktopBehavior::~StudyTutorialDesktopBehavior()
{
	if (m_pPointCloud)
		delete m_pPointCloud;

	if (m_pDataVolume)
		delete m_pDataVolume;

	if (m_pColorScaler)
		delete m_pColorScaler;
}

void StudyTutorialDesktopBehavior::init()
{
	setupViews();

	std::cout << "Starting tutorial..." << std::endl;

	DesktopCleanBehavior *dcb = new DesktopCleanBehavior(m_pDataVolume);
	BehaviorManager::getInstance().addBehavior("desktop_edit", dcb);
	dcb->init();
}


void StudyTutorialDesktopBehavior::processEvent(SDL_Event & ev)
{
	ArcBall *arcball = static_cast<ArcBall*>(BehaviorManager::getInstance().getBehavior("arcball"));
	LassoTool *lasso = static_cast<LassoTool*>(BehaviorManager::getInstance().getBehavior("lasso"));

	auto m_ivec2WindowSize = Renderer::getInstance().getUIRenderSize();
	auto m_Camera = Renderer::getInstance().getCamera();

	if (ev.type == SDL_KEYDOWN)
	{
		if (ev.key.keysym.sym == SDLK_SPACE)
		{
			DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));

			if (desktopEdit)
				desktopEdit->activate();
		}

		if (ev.key.keysym.sym == SDLK_r)
		{
			arcball->reset();
			m_pDataVolume->resetPositionAndOrientation();
			m_Camera->pos = glm::vec3(0.f, 0.f, 0.57f);
			Renderer::getInstance().getMonoInfo()->view = glm::lookAt(m_Camera->pos, m_Camera->lookat, m_Camera->up);
		}

		if (ev.key.keysym.sym == SDLK_BACKSPACE)
		{
			reset();

			arcball->reset();
			m_pDataVolume->resetPositionAndOrientation();
			m_Camera->pos = glm::vec3(0.f, 0.f, 0.57f);
			Renderer::getInstance().getMonoInfo()->view = glm::lookAt(m_Camera->pos, m_Camera->lookat, m_Camera->up);

			init();
		}
	}

	// MOUSE

	if (ev.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
	{
		if (ev.button.button == SDL_BUTTON_LEFT)
		{
			m_bLeftMouseDown = true;

			if (arcball)
				arcball->beginDrag(glm::vec2(ev.button.x, m_ivec2WindowSize.y - ev.button.y));

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
				lasso->start(ev.button.x, m_ivec2WindowSize.y - ev.button.y);
		}
		if (ev.button.button == SDL_BUTTON_MIDDLE)
		{
			if (lasso && m_bRightMouseDown)
			{
				lasso->end();
			}
			m_bMiddleMouseDown = true;

			if (arcball)
				arcball->translate(glm::vec2(ev.button.x, m_ivec2WindowSize.y - ev.button.y));
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
				arcball->drag(glm::vec2(ev.button.x, m_ivec2WindowSize.y - ev.button.y));
		}
		if (m_bRightMouseDown && !m_bLeftMouseDown)
		{
			if (lasso)
				lasso->move(ev.button.x, m_ivec2WindowSize.y - ev.button.y);
		}
	}
	if (ev.type == SDL_MOUSEWHEEL)
	{
		if (lasso)
			lasso->reset();

		glm::vec3 eyeForward = glm::normalize(m_Camera->lookat - m_Camera->pos);
		m_Camera->pos += eyeForward * ((float)ev.wheel.y*0.1f);

		float newLen = glm::length(m_Camera->lookat - m_Camera->pos);

		if (newLen < 0.1f)
			m_Camera->pos = m_Camera->lookat - eyeForward * 0.1f;
		if (newLen > 10.f)
			m_Camera->pos = m_Camera->lookat - eyeForward * 10.f;

		Renderer::getInstance().getMonoInfo()->view = glm::lookAt(m_Camera->pos, m_Camera->lookat, m_Camera->up);
	}
}


void StudyTutorialDesktopBehavior::update()
{
	m_pPointCloud->update();
	m_pDataVolume->update();

	if (!m_bPointsCleaned)
	{
		unsigned int prevPointCount = m_nPointsLeft;
		m_nPointsLeft = m_nCleanedGoodPoints = m_nPointsCleaned = 0u;

		for (unsigned int i = 0; i < m_pPointCloud->getPointCount(); ++i)
		{
			if (m_pPointCloud->getPointDepthTPU(i) == 1.f)
			{
				if (m_pPointCloud->getPointMark(i) == 1)
					m_nPointsCleaned++;
				else
					m_nPointsLeft++;
			}
			else if (m_pPointCloud->getPointMark(i) == 1)
				m_nCleanedGoodPoints++;
		}

		if (m_nPointsLeft == 0u)
		{
			BehaviorManager::getInstance().removeBehavior("desktop_edit");

			m_bPointsCleaned = true;
		}
	}
}

void StudyTutorialDesktopBehavior::draw()
{
	m_pDataVolume->drawVolumeBacking(glm::inverse(Renderer::getInstance().getMonoInfo()->view), 2.f);
	m_pDataVolume->drawBBox(0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "instanced";
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("disc");
	rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("disc");
	rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("disc");
	rs.instanced = true;
	rs.specularExponent = 0.f;

	for (auto &cloud : m_pDataVolume->getDatasets())
	{
		if (!static_cast<SonarPointCloud*>(cloud)->ready())
		{
			continue;
		}

		rs.VAO = static_cast<SonarPointCloud*>(cloud)->getVAO();
		rs.modelToWorldTransform = m_pDataVolume->getTransformDataset(cloud);
		rs.instanceCount = static_cast<SonarPointCloud*>(cloud)->getPointCount();
		Renderer::getInstance().addToDynamicRenderQueue(rs);
	}

	float accuracy = static_cast<float>(m_nPointsCleaned) / static_cast<float>(m_nCleanedGoodPoints + m_nPointsCleaned);
	float accuracyPct = accuracy * 100.f;
	std::stringstream accuracyStr;
	glm::vec4 accColor;

	if (std::isnan(accuracy))
	{
		accuracyStr << "n/a";
		accColor = glm::vec4(glm::vec3(0.7f), 1.f);
	}
	else
	{
		accuracyStr.precision(2);
		accuracyStr << std::fixed << accuracyPct << "%";
		accColor = glm::mix(glm::vec4(0.8f, 0.1f, 0.2f, 1.f), glm::vec4(0.1f, 0.8f, 0.2f, 1.f), accuracy);
	}

	if (!m_bPointsCleaned)
	{
		std::stringstream ss;

		// Point Count Label
		ss << "Bad Points Remaining: " << m_nPointsLeft;

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
			glm::vec3(0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::LEFT,
			Renderer::TextAnchor::BOTTOM_LEFT
		);

		ss = std::stringstream();
		ss << "Accuracy: " << accuracyStr.str();

		Renderer::getInstance().drawUIText(
			ss.str(),
			accColor,
			glm::vec3(0.f, 25.f, 0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::LEFT,
			Renderer::TextAnchor::BOTTOM_LEFT
		);
	}
	else
	{
		Renderer::getInstance().drawUIText(
			"Tutorial Complete!",
			glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] *0.5f, Renderer::getInstance().getMonoInfo()->viewport[3], 0.f),
			glm::quat(),
			100,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			"Press the <Enter> key to continue...",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, Renderer::getInstance().getMonoInfo()->viewport[3] - 110, 0.f),
			glm::quat(),
			50,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);

		Renderer::getInstance().drawUIText(
			accuracyStr.str(),
			accColor,
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, 30.f, 0.f),
			glm::quat(),
			75,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		Renderer::getInstance().drawUIText(
			"Accuracy",
			glm::vec4(glm::vec3(0.7f), 1.f),
			glm::vec3(Renderer::getInstance().getMonoInfo()->viewport[2] * 0.5f, 0.f, 0.f),
			glm::quat(),
			25,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);
	}
}

void StudyTutorialDesktopBehavior::reset()
{
	m_pPointCloud->resetAllMarks();

	m_bPointsCleaned = false;

	m_nPointsLeft = m_pPointCloud->getPointCount();

	m_bActive = true;
}

void StudyTutorialDesktopBehavior::finish()
{
	if (m_bPointsCleaned)
		m_bActive = false;
}

void StudyTutorialDesktopBehavior::setupViews()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();

	cam->pos = glm::vec3(0.f, 0.f, 0.57f);
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = (29.7f * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	glm::vec2 vec2ScreenSizeMeters(winSize.x * sizer, winSize.y * sizer);

	Renderer::getInstance().setMonoRenderSize(winSize);

	Renderer::SceneViewInfo* svi = Renderer::getInstance().getMonoInfo();
	svi->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	svi->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	svi->view = glm::lookAt(cam->pos, glm::vec3(0.f), cam->up);
	svi->projection = utils::getViewingFrustum(
		glm::vec4(cam->pos, 1.f),
		glm::vec4(glm::vec3(0.f), 1.f),
		glm::vec4(glm::vec3(0.f, 0.f, 1.f), 0.f),
		glm::vec4(cam->up, 0.f),
		vec2ScreenSizeMeters);
	svi->viewport = glm::ivec4(0, 0, svi->m_nRenderWidth, svi->m_nRenderHeight);
}