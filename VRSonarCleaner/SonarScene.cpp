#include "SonarScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "arcball.h"
#include "LassoTool.h"
#include "SelectAreaBehavior.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "CurateStudyDataBehavior.h"
#include "RunStudyBehavior.h"
#include "StudyTutorialBehavior.h"
#include "StudyIntroBehavior.h"
#include "ScaleTutorial.h"
#include "StudyEditTutorial.h"
#include "CloudEditControllerTutorial.h"
#include "DesktopCleanBehavior.h"
#include "StudyTrialDesktopBehavior.h"
#include "SnellenTest.h"

using namespace std::chrono_literals;

SonarScene::SonarScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pTableVolume(NULL)
	, m_pWallVolume(NULL)
	, m_vec3RoomSize(1.f, 3.f, 1.f)
	, m_bUseDesktop(false)
	, m_bStudyMode(false)
	, m_bUseVR(true)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_bMiddleMouseDown(false)
	, m_bInitialColorRefresh(false)
{
}


SonarScene::~SonarScene()
{
	if (m_pTableVolume)
		delete m_pTableVolume;
	if (m_pWallVolume)
		delete m_pWallVolume;
}

void SonarScene::init()
{
	Renderer::getInstance().setWindowTitle("VR Sonar Cleaner | CCOM VisLab");

	glm::vec3 wallSize(10.f, (m_vec3RoomSize.y * 0.9f), 0.5f);
	glm::quat wallOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 wallPosition(0.f, m_vec3RoomSize.y * 0.5f, m_vec3RoomSize.z);

	m_pWallVolume = new DataVolume(wallPosition, wallOrientation, wallSize);
	m_pWallVolume->setBackingColor(glm::vec4(0.15f, 0.21f, 0.31f, 1.f));

	glm::vec3 tablePosition = glm::vec3(0.f, 1.f, -m_vec3RoomSize.z);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(1.5f, 1.5f, 0.5f);

	m_pTableVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	m_pTableVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pTableVolume->setFrameColor(glm::vec4(1.f));

	Renderer::getInstance().getCamera()->pos = tablePosition + glm::vec3(0.f, 0.f, 1.f) * 3.f;
	Renderer::getInstance().getCamera()->lookat = tablePosition;

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
		m_pWallVolume->add(cloud);
		m_pTableVolume->add(cloud);
	}

	m_vpDataVolumes.push_back(m_pTableVolume);
	//m_vpDataVolumes.push_back(m_pWallVolume);

	if (m_bUseDesktop)
	{
		DesktopCleanBehavior *tmp = new DesktopCleanBehavior(m_pTableVolume);
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
			std::stringstream ss;

			ss << "View Reset" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();

			DataLogger::getInstance().logMessage(ss.str());

			Renderer::getInstance().getCamera()->pos = m_pTableVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
			Renderer::getInstance().getCamera()->lookat = m_pTableVolume->getPosition();

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(Renderer::getInstance().getCamera()->pos, Renderer::getInstance().getCamera()->lookat, Renderer::getInstance().getCamera()->up);

			arcball->reset();
		}
	}

	if (ev.key.keysym.sym == SDLK_RETURN)
	{
		RunStudyBehavior *study = static_cast<RunStudyBehavior*>(BehaviorManager::getInstance().getBehavior("Desktop Study"));

		if (study)
			study->next();
	}

	if (ev.key.keysym.sym == SDLK_SPACE)
	{
		DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));

		if (desktopEdit)
			desktopEdit->activate();
	}


	if (ev.key.keysym.sym == SDLK_KP_1)
	{
		m_pWallVolume->setVisible(false);
		m_pTableVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		RunStudyBehavior *rsb = new RunStudyBehavior(m_pTDM, false);
		BehaviorManager::getInstance().addBehavior("Standing Study", rsb);
		rsb->init();
	}

	if (ev.key.keysym.sym == SDLK_KP_2)
	{
		m_pWallVolume->setVisible(false);
		m_pTableVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		RunStudyBehavior *rsb = new RunStudyBehavior(m_pTDM, true);
		BehaviorManager::getInstance().addBehavior("Sitting Study", rsb);
		rsb->init();
	}
	if (ev.key.keysym.sym == SDLK_KP_3)
	{
		m_pWallVolume->setVisible(false);
		m_pTableVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		RunStudyBehavior *rsb = new RunStudyBehavior();
		BehaviorManager::getInstance().addBehavior("Desktop Study", rsb);
		rsb->init();
	}

	if (m_bStudyMode)
	{
		if (ev.type == SDL_KEYDOWN)
		{
			if ((ev.key.keysym.mod & KMOD_LSHIFT) && ev.key.keysym.sym == SDLK_ESCAPE
				|| ev.key.keysym.sym == SDLK_q)
			{
				//bRet = true;
			}

		}

		// MOUSE
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

				if (DataLogger::getInstance().logging())
				{
					std::stringstream ss;

					ss << "Camera Zoom" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
					ss << "\t";
					ss << "cam-pos:\"" << cam->pos.x << "," << cam->pos.y << "," << cam->pos.z << "\"";
					ss << ";";
					ss << "cam-look:\"" << cam->lookat.x << "," << cam->lookat.y << "," << cam->lookat.z << "\"";
					ss << ";";
					ss << "cam-up:\"" << cam->up.x << "," << cam->up.y << "," << cam->up.z << "\"";

					DataLogger::getInstance().logMessage(ss.str());
				}
			}
		}
	}

	if (ev.key.keysym.sym == SDLK_l)
	{
		if (m_bUseVR)
		{
			if (!BehaviorManager::getInstance().getBehavior("harvestpoints"))
				BehaviorManager::getInstance().addBehavior("harvestpoints", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
			if (!BehaviorManager::getInstance().getBehavior("grab"))
				BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));
			if (!BehaviorManager::getInstance().getBehavior("scale"))
				BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));
		}
		else
			m_pWallVolume->setVisible(false);

		using namespace std::experimental::filesystem::v1;

		//path dataset("south_santa_rosa");
		//path dataset("santa_cruz_south");
		path dataset("santa_cruz_basin");

		auto basePath = current_path().append(path("resources/data/sonar/nautilus"));
		std::cout << "Base data directory: " << basePath << std::endl;

		auto acceptsPath = path(basePath).append(path("accept"));
		auto rejectsPath = path(basePath).append(path("reject"));

		for (directory_iterator it(acceptsPath.append(dataset)); it != directory_iterator(); ++it)
		{
			if (is_regular_file(*it))
			{
				if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
				{
					SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
					m_vpClouds.push_back(tmp);
					m_pTableVolume->add(tmp);
					m_pWallVolume->add(tmp);
					break;
				}
			}
		}

		//for (directory_iterator it(rejectsPath.append(dataset)); it != directory_iterator(); ++it)
		//{
		//	if (is_regular_file(*it))
		//	{
		//		if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
		//		{
		//			SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
		//			m_vpClouds.push_back(tmp);
		//			m_pTableVolume->add(tmp);
		//			m_pWallVolume->add(tmp);
		//			break;
		//		}
		//	}
		//}

		refreshColorScale(m_pColorScalerTPU, m_vpClouds);
	}

	if (ev.key.keysym.sym == SDLK_KP_ENTER)
	{
		m_pTableVolume->setVisible(false);
		m_pWallVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		SnellenTest *st = new SnellenTest(m_pTDM, 10.f);
		BehaviorManager::getInstance().addBehavior("snellen", st);
		st->init();
	}
	if (ev.key.keysym.sym == SDLK_UP)
	{
		SnellenTest *snellen = static_cast<SnellenTest*>(BehaviorManager::getInstance().getBehavior("snellen"));

		if (snellen)
		{
			snellen->setVisualAngle(snellen->getVisualAngle() + 1.f);
			snellen->newTest();
		}
	}
	if (ev.key.keysym.sym == SDLK_DOWN)
	{
		SnellenTest *snellen = static_cast<SnellenTest*>(BehaviorManager::getInstance().getBehavior("snellen"));

		if (snellen)
		{
			float angle = snellen->getVisualAngle() - 1.f;
			if (angle < 1.f) angle = 1.f;
			snellen->setVisualAngle(angle);
			snellen->newTest();
		}
	}


	if (ev.key.keysym.sym == SDLK_y)
	{
		m_pTableVolume->setVisible(false);
		m_pWallVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		CloudEditControllerTutorial *cet = new CloudEditControllerTutorial(m_pTDM);
		BehaviorManager::getInstance().addBehavior("Demo", cet);
		cet->init();
	}
	if (ev.key.keysym.sym == SDLK_u)
	{
		BehaviorManager::getInstance().addBehavior("GetStudyData", new CurateStudyDataBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
	}

	if (ev.key.keysym.sym == SDLK_KP_0)
	{
		m_pWallVolume->setVisible(false);
		m_pTableVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		BehaviorManager::getInstance().addBehavior("Tutorial", new StudyTutorialBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
	}

	if (ev.key.keysym.sym == SDLK_KP_PERIOD)
	{
		m_pWallVolume->setVisible(false);
		m_pTableVolume->setVisible(false);
		BehaviorManager::getInstance().clearBehaviors();
		StudyTrialDesktopBehavior  *stdb = new StudyTrialDesktopBehavior("tutorial_points.csv", "demo");
		BehaviorManager::getInstance().addBehavior("Tutorial", stdb);
		stdb->init();
	}

	if (ev.key.keysym.sym == SDLK_d)
	{

		static_cast<ProbeBehavior*>(BehaviorManager::getInstance().getBehavior("pointclean"))->activateDemoMode();

		//if ((sdlEvent.key.keysym.mod & KMOD_LCTRL))
		//{
		//	if (!m_bUseDesktop)
		//	{
		//		m_bUseDesktop = true;
		//		initDesktop();
		//	}
		//}
	}

	if (ev.key.keysym.sym == SDLK_r)
	{
		printf("Pressed r, resetting marks\n");

		if (!m_bStudyMode)
		{
			for (auto &cloud : m_vpClouds)
				cloud->resetAllMarks();
		}

		for (auto &dv : m_vpDataVolumes)
			dv->resetPositionAndOrientation();

		if (arcball)
		{
			cam->pos = m_pTableVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
			cam->lookat = m_pTableVolume->getPosition();

			Renderer::getInstance().getWindow3DViewInfo()->view = glm::lookAt(cam->pos, cam->lookat, cam->up);

			arcball->reset();
		}
	}

	if (ev.key.keysym.sym == SDLK_g)
	{
		printf("Pressed g, generating fake test cloud\n");
		//m_pClouds->generateFakeTestCloud(150, 150, 25, 40000);
		//m_pColorScalerTPU->resetBiValueScaleMinMax(m_pClouds->getMinDepthTPU(), m_pClouds->getMaxDepthTPU(), m_pClouds->getMinPositionalTPU(), m_pClouds->getMaxPositionalTPU());
	}

	if (ev.key.keysym.sym == SDLK_RETURN)
	{
		RunStudyBehavior *study = static_cast<RunStudyBehavior*>(BehaviorManager::getInstance().getBehavior("Desktop Study"));

		if (study)
			study->next();
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

			if (DataLogger::getInstance().logging())
			{
				std::stringstream ss;

				ss << "Camera Zoom" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
				ss << "\t";
				ss << "cam-pos:\"" << cam->pos.x << "," << cam->pos.y << "," << cam->pos.z << "\"";
				ss << ";";
				ss << "cam-look:\"" << cam->lookat.x << "," << cam->lookat.y << "," << cam->lookat.z << "\"";
				ss << ";";
				ss << "cam-up:\"" << cam->up.x << "," << cam->up.y << "," << cam->up.z << "\"";

				DataLogger::getInstance().logMessage(ss.str());
			}
		}
	}
}

void SonarScene::update()
{
	if (m_pTDM->getPrimaryController() && !BehaviorManager::getInstance().getBehavior("pointclean"))
		BehaviorManager::getInstance().addBehavior("pointclean", new PointCleanProbe(m_pTDM->getPrimaryController(), m_pTableVolume));

	//if (!BehaviorManager::getInstance().getBehavior("harvestpoints"))
	//	BehaviorManager::getInstance().addBehavior("harvestpoints", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
	if (!BehaviorManager::getInstance().getBehavior("grab"))
		BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));
	if (!BehaviorManager::getInstance().getBehavior("scale"))
		BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));

	for (auto &cloud : m_vpClouds)
		cloud->update();

	for (auto &dv : m_vpDataVolumes)
		dv->update();
}

void SonarScene::draw()
{
	Renderer::getInstance().drawPrimitive("plane", glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(m_vec3RoomSize.x * 2.f, m_vec3RoomSize.z * 2.f, 1.f)), glm::vec4(0.2f, 0.2f, 0.2f, 1.f), glm::vec4(1.f), 132.f);
	
	// draw the lasso points, if any
	//std::vector<glm::vec3> pts = m_pLasso->getPoints();
	//for (int i = 0; i < pts.size(); ++i)
	//{
	//	glm::vec3 pt1 = pts[i];
	//	glm::vec3 pt2 = pts[(i + 1) % pts.size()];
	//	DebugDrawer::getInstance().drawLine
	//	(
	//		glm::unProject(pt1, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)), 
	//		glm::unProject(pt2, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)),
	//		glm::vec4(0.f, 1.f, 0.f, 1.f)
	//	);
	//}

	bool unloadedData = false;

	for (auto &dv : m_vpDataVolumes)
	{
		if (!dv->isVisible()) continue;

		glm::mat4 trans;

		if (m_bUseDesktop)
			trans = glm::inverse(Renderer::getInstance().getWindow3DViewInfo()->view);

		if (m_bUseVR)
			trans = m_pTDM->getHMDToWorldTransform();

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

			rs.VAO = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewVAO() : static_cast<SonarPointCloud*>(cloud)->getPreviewVAO();
			rs.modelToWorldTransform = dv->getTransformDataset(cloud);
			rs.instanceCount = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewPointCount() : static_cast<SonarPointCloud*>(cloud)->getPreviewPointCount();
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

	colorScaler->resetMinMaxForColorScale(m_pTableVolume->getMinDataBound().z, m_pTableVolume->getMaxDataBound().z);
	colorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : clouds)
		cloud->resetAllMarks();
}