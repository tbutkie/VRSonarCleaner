#include "CosmoStudyTrialDesktopScene.h"
#include "BehaviorManager.h"
#include "LightingSystem.h"
#include <gtc/quaternion.hpp>
#include <gtc/random.hpp>
#include "utilities.h"
#include "DataLogger.h"
#include <random>


CosmoStudyTrialDesktopScene::CosmoStudyTrialDesktopScene()
	: m_pCosmoVolume(NULL)
	, m_pHairySlice(NULL)
	, m_bShowCosmoBBox(true)
	, m_bShowPlane(false)
	, m_bRandomSliceRequested(false)
	, m_pEditParam(NULL)
	, m_fScreenDiagonalInches(29.7f)
{
}


CosmoStudyTrialDesktopScene::~CosmoStudyTrialDesktopScene()
{
	DataLogger::getInstance().closeLog();
}

void CosmoStudyTrialDesktopScene::init()
{

	Renderer::getInstance().addShader("cosmo", { "resources/shaders/cosmo.vert", "resources/shaders/cosmo.frag" });
	Renderer::getInstance().toggleSkybox();
	Renderer::getInstance().setClearColor(glm::vec4(0.4f, 0.4f, 0.6f, 1.f));

	m_pCosmoVolume = new CosmoVolume("resources/data/bin");

	m_pCosmoVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pCosmoVolume->setFrameColor(glm::vec4(1.f));

	if (m_pHairySlice)
		delete m_pHairySlice;

	m_pHairySlice = new HairySlice(m_pCosmoVolume);
	
	setupViews();
	setupParameters();
	buildScalarPlane();

	//DataLogger::getInstance().setLogDirectory("/");
	//DataLogger::getInstance().openLog("views");
	//DataLogger::getInstance().setID("view");
	//DataLogger::getInstance().start();
}

void CosmoStudyTrialDesktopScene::processSDLEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
	{
		if (m_pEditParam)
		{
			if (ev.key.keysym.sym == SDLK_BACKSPACE && m_pEditParam->buf.length() > 0)
				m_pEditParam->buf.pop_back();

			if (m_pEditParam->format & STUDYPARAM_NUMERIC)
			{
				if (ev.key.keysym.sym >= SDLK_0 && ev.key.keysym.sym <= SDLK_9)
					m_pEditParam->buf += std::to_string(ev.key.keysym.sym - SDLK_0);
			}

			if (m_pEditParam->format & STUDYPARAM_ALPHA)
			{
				if (ev.key.keysym.sym >= SDLK_a && ev.key.keysym.sym <= SDLK_z)
					m_pEditParam->buf += SDL_GetKeyName(ev.key.keysym.sym);
			}

			if (m_pEditParam->format & STUDYPARAM_POSNEG)
			{
				if (ev.key.keysym.sym == SDLK_MINUS)
				{
					if (m_pEditParam->buf[0] == '-')
						m_pEditParam->buf.erase(0, 1);
					else
						m_pEditParam->buf.insert(0, 1, '-');
				}
			}

			if (m_pEditParam->format & STUDYPARAM_DECIMAL)
			{
				if (ev.key.keysym.sym == SDLK_PERIOD)
				{
					auto decimalCount = std::count(m_pEditParam->buf.begin(), m_pEditParam->buf.end(), '.');

					if (decimalCount == 0 ||
						((m_pEditParam->format & STUDYPARAM_IP) && decimalCount < 3) || 
						((m_pEditParam->format & STUDYPARAM_RGB) && decimalCount < 3) ||
						((m_pEditParam->format & STUDYPARAM_RGBA) && decimalCount < 4))
						m_pEditParam->buf += ".";
				}
			}

			if (m_pEditParam->format & STUDYPARAM_RGB)
			{
				if (ev.key.keysym.sym == SDLK_COMMA)
				{
					auto commaCount = std::count(m_pEditParam->buf.begin(), m_pEditParam->buf.end(), ',');

					if (commaCount == 0 || ((m_pEditParam->format & STUDYPARAM_RGB) && commaCount < 2))
						m_pEditParam->buf += ",";
				}
			}

			if (m_pEditParam->format & STUDYPARAM_RGBA)
			{
				if (ev.key.keysym.sym == SDLK_COMMA)
				{
					auto commaCount = std::count(m_pEditParam->buf.begin(), m_pEditParam->buf.end(), ',');

					if (commaCount == 0 || ((m_pEditParam->format & STUDYPARAM_RGBA) && commaCount < 3))
						m_pEditParam->buf += ",";
				}
			}


			if (ev.key.keysym.sym == SDLK_RETURN && m_pEditParam->buf.length() > 0)
			{
				Renderer::getInstance().showMessage(m_pEditParam->desc + " set to " + m_pEditParam->buf);
				bool cuttingPlaneReseed = false;

				if (m_pEditParam->desc.compare("RK4 Step Size") == 0)
				{
					m_pHairySlice->m_fRK4StepSize = std::stof(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("RK4 Max Steps") == 0)
				{
					m_pHairySlice->m_uiRK4MaxPropagation_OneWay = std::stoi(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("RK4 End Velocity") == 0)
				{
					m_pHairySlice->m_fRK4StopVelocity = std::stof(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("Cutting Plane Seeding Resolution") == 0)
				{
					m_pHairySlice->m_uiCuttingPlaneGridRes = std::stoi(m_pEditParam->buf);
					cuttingPlaneReseed = true;
				}
				
				if (m_pEditParam->desc.compare("Cutting Plane Width") == 0)
				{
					m_pHairySlice->m_vec2CuttingPlaneSize.x = std::stof(m_pEditParam->buf);
					cuttingPlaneReseed = true;
				}
				
				if (m_pEditParam->desc.compare("Cutting Plane Height") == 0)
				{
					m_pHairySlice->m_vec2CuttingPlaneSize.y = std::stof(m_pEditParam->buf);
					cuttingPlaneReseed = true;
				}
				
				if (m_pEditParam->desc.compare("Streamtube Radius") == 0)
				{
					m_pHairySlice->m_fTubeRadius = std::stof(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("Min Velocity Color") == 0)
				{
					m_pHairySlice->m_vec4VelColorMin = utils::color::str2rgb(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("Max Velocity Color") == 0)
				{
					m_pHairySlice->m_vec4VelColorMax = utils::color::str2rgb(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("Halo Radius Factor") == 0)
				{
					m_pHairySlice->m_fHaloRadiusFactor = std::stof(m_pEditParam->buf);
				}
				
				if (m_pEditParam->desc.compare("Halo Color") == 0)
				{
					m_pHairySlice->m_vec4HaloColor = utils::color::str2rgb(m_pEditParam->buf);
					m_pHairySlice->m_rsHalo.diffuseColor = m_pHairySlice->m_vec4HaloColor;
				}

				if (m_pEditParam->desc.compare("Osc. Amplitude") == 0)
				{
					m_pHairySlice->m_fOscAmpDeg = std::stof(m_pEditParam->buf);
				}

				if (m_pEditParam->desc.compare("Osc. Period") == 0)
				{
					m_pHairySlice->m_fOscTime = std::stof(m_pEditParam->buf);
				}

				if (m_pEditParam->desc.compare("Display Diag (inches)") == 0)
				{
					m_fScreenDiagonalInches = std::stof(m_pEditParam->buf);
					setupViews();
				}

				if (m_pEditParam->desc.compare("Clear Color") == 0)
				{
					Renderer::getInstance().setClearColor(utils::color::str2rgb(m_pEditParam->buf));
				}
				
				m_pEditParam = NULL;
			}
		}
		else
		{
			if (ev.key.keysym.sym == SDLK_F1)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("RK4 Step Size") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F2)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("RK4 Max Steps") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F3)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("RK4 End Velocity") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F4)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Cutting Plane Seeding Resolution") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F5)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Cutting Plane Width") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F6)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Cutting Plane Height") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F7)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Streamtube Radius") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F8)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Min Velocity Color") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F9)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Max Velocity Color") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F10)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Halo Radius Factor") == 0;
				}));
			}
			
			if (ev.key.keysym.sym == SDLK_F11)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Halo Color") == 0;
				}));
			}

			if (!(ev.key.keysym.mod & KMOD_LSHIFT) && ev.key.keysym.sym == SDLK_PAGEUP)
			{
				m_pHairySlice->nextGeomStyle();
				Renderer::getInstance().showMessage("Hairy Slice geometry set to " + m_pHairySlice->getGeomStyle());
			}

			if (!(ev.key.keysym.mod & KMOD_LSHIFT) && ev.key.keysym.sym == SDLK_PAGEDOWN)
			{
				m_pHairySlice->prevGeomStyle();
				Renderer::getInstance().showMessage("Hairy Slice geometry set to " + m_pHairySlice->getGeomStyle());
			}

			if ((ev.key.keysym.mod & KMOD_LSHIFT) && ev.key.keysym.sym == SDLK_PAGEUP)
			{
				m_pHairySlice->nextShader();
				Renderer::getInstance().showMessage("Hairy Slice shader set to " + m_pHairySlice->getShaderName());
			}

			if ((ev.key.keysym.mod & KMOD_LSHIFT) && ev.key.keysym.sym == SDLK_PAGEDOWN)
			{
				m_pHairySlice->prevShader();
				Renderer::getInstance().showMessage("Hairy Slice shader set to " + m_pHairySlice->getShaderName());
			}

			if (ev.key.keysym.sym == SDLK_KP_DIVIDE)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Osc. Amplitude") == 0;
				}));
			}

			if (ev.key.keysym.sym == SDLK_KP_MULTIPLY)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Osc. Period") == 0;
				}));
			}

			if (ev.key.keysym.sym == SDLK_KP_PERIOD)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Display Diag (inches)") == 0;
				}));
			}

			if (ev.key.keysym.sym == SDLK_BACKSPACE)
			{
				m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
					return p.desc.compare("Clear Color") == 0;
				}));
			}

			if (ev.key.keysym.sym == SDLK_KP_PLUS)
			{
				m_pCosmoVolume->setDimensions(m_pCosmoVolume->getDimensions() * 1.1f);
			}

			if (ev.key.keysym.sym == SDLK_KP_MINUS)
			{
				m_pCosmoVolume->setDimensions(m_pCosmoVolume->getDimensions() * 0.9f);
			}

			if (ev.key.keysym.sym == SDLK_KP_2)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() + glm::vec3(0.f, 1.f, 0.f) * 0.01f * dimratio);
			}

			if (ev.key.keysym.sym == SDLK_KP_8)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() - glm::vec3(0.f, 1.f, 0.f) * 0.01f * dimratio);
			}

			if (ev.key.keysym.sym == SDLK_KP_4)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() + glm::vec3(1.f, 0.f, 0.f) * 0.01f * dimratio);
			}

			if (ev.key.keysym.sym == SDLK_KP_6)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() - glm::vec3(1.f, 0.f, 0.f) * 0.01f * dimratio);
			}

			if (ev.key.keysym.sym == SDLK_UP)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 0.01f * dimratio);
			}

			if (ev.key.keysym.sym == SDLK_DOWN)
			{
				float dimratio = m_pCosmoVolume->getOriginalDimensions().x / m_pCosmoVolume->getDimensions().x;
				m_pCosmoVolume->setPosition(m_pCosmoVolume->getPosition() - glm::vec3(0.f, 0.f, 1.f) * 0.01f * dimratio);
			}
			
			if (ev.key.keysym.sym == SDLK_KP_ENTER)
			{
				m_pCosmoVolume->setDimensions(m_pCosmoVolume->getOriginalDimensions() * 4.f);
				m_pCosmoVolume->setPosition(glm::linearRand(-(m_pCosmoVolume->getOriginalDimensions() / 2.f), (m_pCosmoVolume->getOriginalDimensions() / 2.f)));

				glm::vec3 w = glm::normalize(glm::sphericalRand(1.f));
				glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), w));
				glm::vec3 v = glm::normalize(glm::cross(w, u));

				m_pCosmoVolume->setOrientation(glm::mat3(u, v, w));

				m_pHairySlice->set();
			}

			if (ev.key.keysym.sym == SDLK_INSERT)
			{
				std::stringstream ss;
				ss << m_pCosmoVolume->getPosition().x << "," << m_pCosmoVolume->getPosition().y << "," << m_pCosmoVolume->getPosition().z << "," << m_pCosmoVolume->getDimensions().x;
				DataLogger::getInstance().logMessage(ss.str());
			}

			if (ev.key.keysym.sym == SDLK_b)
			{
				Renderer::getInstance().toggleSkybox();
			}

			if (ev.key.keysym.sym == SDLK_c)
			{
				m_bShowPlane = !m_bShowPlane;
				Renderer::getInstance().showMessage("Scalar plane set to " + std::to_string(m_bShowPlane));
			}

			if (ev.key.keysym.sym == SDLK_f)
			{
				m_pHairySlice->m_bShowFrame = !m_pHairySlice->m_bShowFrame;
				Renderer::getInstance().showMessage("Frame visibility set to " + std::to_string(m_pHairySlice->m_bShowFrame));
			}

			if (ev.key.keysym.sym == SDLK_g)
			{
				m_pHairySlice->m_bShowGeometry = !m_pHairySlice->m_bShowGeometry;
				Renderer::getInstance().showMessage("Geometry visibility set to " + std::to_string(m_pHairySlice->m_bShowGeometry));
			}

			if (ev.key.keysym.sym == SDLK_h)
			{
				m_pHairySlice->m_bShowHalos = !m_pHairySlice->m_bShowHalos;
				Renderer::getInstance().showMessage("Haloing set to " + std::to_string(m_pHairySlice->m_bShowHalos));
			}

			if (ev.key.keysym.sym == SDLK_j)
			{
				m_pHairySlice->m_bJitterSeeds = !m_pHairySlice->m_bJitterSeeds;
				Renderer::getInstance().showMessage("Seed Jittering set to " + std::to_string(m_pHairySlice->m_bJitterSeeds));
			}

			if (ev.key.keysym.sym == SDLK_l)
			{
				m_bShowCosmoBBox = !m_bShowCosmoBBox;
				Renderer::getInstance().showMessage("Dataset bbox visibility set to " + std::to_string(m_bShowCosmoBBox));
			}

			if (ev.key.keysym.sym == SDLK_m)
			{
				m_pHairySlice->m_bSpinReticule = !m_pHairySlice->m_bSpinReticule;
				Renderer::getInstance().showMessage("Reticule spin set to " + std::to_string(m_pHairySlice->m_bSpinReticule));
			}

			if (ev.key.keysym.sym == SDLK_o)
			{
				m_pHairySlice->m_bOscillate = !m_pHairySlice->m_bOscillate;
			}

			if (ev.key.keysym.sym == SDLK_p)
			{
				m_pHairySlice->m_bShowGrid = !m_pHairySlice->m_bShowGrid;
				Renderer::getInstance().showMessage("Grid visibility set to " + std::to_string(m_pHairySlice->m_bShowGrid));
			}

			if (ev.key.keysym.sym == SDLK_r)
			{
				m_pHairySlice->m_bShowReticule = !m_pHairySlice->m_bShowReticule;
				Renderer::getInstance().showMessage("Target reticule visibility set to " + std::to_string(m_pHairySlice->m_bShowReticule));
			}

			if (ev.key.keysym.sym == SDLK_s)
			{
				m_pHairySlice->m_bShowSeeds = !m_pHairySlice->m_bShowSeeds;
				Renderer::getInstance().showMessage("Seed visibility set to " + std::to_string(m_pHairySlice->m_bShowSeeds));
			}

			if (ev.key.keysym.sym == SDLK_t)
			{
				m_pHairySlice->m_bShowTarget = !m_pHairySlice->m_bShowTarget;
				Renderer::getInstance().showMessage("Target visibility set to " + std::to_string(m_pHairySlice->m_bShowTarget));
			}

			if ((ev.key.keysym.mod & KMOD_LSHIFT || ev.key.keysym.mod & KMOD_RSHIFT) && ev.key.keysym.sym == SDLK_RETURN)
			{
				m_pHairySlice->reseed();
				Renderer::getInstance().showMessage("Hairy Slice reseeded");
			}
			
			if (!(ev.key.keysym.mod & KMOD_LSHIFT || ev.key.keysym.mod & KMOD_RSHIFT) && ev.key.keysym.sym == SDLK_RETURN)
			{
				m_pHairySlice->set();
			}
		}
	}
}

void CosmoStudyTrialDesktopScene::update()
{
	if (m_bRandomSliceRequested)
	{

		m_bRandomSliceRequested = false;
	}

	m_pCosmoVolume->update();

	LightingSystem::Light* l = LightingSystem::getInstance().getLight(0);
	l->direction = glm::inverse(Renderer::getInstance().getLeftEyeInfo()->view) * glm::normalize(glm::vec4(-1.f, -1.f, -1.f, 0.f));

	m_pHairySlice->update();
}

void CosmoStudyTrialDesktopScene::draw()
{
	//m_pCosmoVolume->drawVolumeBacking(glm::inverse(glm::lookAt(Renderer::getInstance().getCamera()->pos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f))), 1.f);
	if (m_bShowCosmoBBox)
		m_pCosmoVolume->drawBBox(0.f);
		
	if (m_pEditParam)
	{
		Renderer::getInstance().drawUIText(
			m_pEditParam->desc + ": " + m_pEditParam->buf + "_",
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().y / 50.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	
	m_rsPlane.modelToWorldTransform = m_pCosmoVolume->getTransformRawDomainToVolume();

	if (m_bShowPlane)
		Renderer::getInstance().addToDynamicRenderQueue(m_rsPlane);

	m_pHairySlice->draw();
}


void CosmoStudyTrialDesktopScene::setupParameters()
{
	m_vParams.clear();

	m_vParams.push_back({ "RK4 Step Size" , std::to_string(m_pHairySlice->m_fRK4StepSize), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "RK4 Max Steps" , std::to_string(m_pHairySlice->m_uiRK4MaxPropagation_OneWay), STUDYPARAM_NUMERIC });
	m_vParams.push_back({ "RK4 End Velocity" , std::to_string(m_pHairySlice->m_fRK4StopVelocity), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Cutting Plane Seeding Resolution" , std::to_string(m_pHairySlice->m_uiCuttingPlaneGridRes), STUDYPARAM_NUMERIC });
	m_vParams.push_back({ "Cutting Plane Width" , std::to_string(m_pHairySlice->m_vec2CuttingPlaneSize.x), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Cutting Plane Height" , std::to_string(m_pHairySlice->m_vec2CuttingPlaneSize.y), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Streamtube Radius" , std::to_string(m_pHairySlice->m_fTubeRadius), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Min Velocity Color" , utils::color::rgb2str(m_pHairySlice->m_vec4VelColorMin), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_RGBA });
	m_vParams.push_back({ "Max Velocity Color" , utils::color::rgb2str(m_pHairySlice->m_vec4VelColorMax), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_RGBA });
	m_vParams.push_back({ "Halo Radius Factor" , std::to_string(m_pHairySlice->m_fHaloRadiusFactor), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Halo Color" , utils::color::rgb2str(m_pHairySlice->m_vec4HaloColor), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_RGBA });	
	m_vParams.push_back({ "Osc. Amplitude" , std::to_string(m_pHairySlice->m_fOscAmpDeg), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Osc. Period" , std::to_string(m_pHairySlice->m_fOscTime), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });

	m_vParams.push_back({ "Display Diag (inches)" , std::to_string(m_fScreenDiagonalInches), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Clear Color" , utils::color::rgb2str(Renderer::getInstance().getClearColor()), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_RGBA });
}

void CosmoStudyTrialDesktopScene::setupViews()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->pos = glm::vec3(0.f, 0.f, 0.57f);
	cam->lookat = glm::vec3(0.f);
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 COP = cam->pos;
	glm::vec3 COPOffset = glm::vec3(0.f); //glm::vec3(1.f, 0.f, 0.f) * 0.067f * 0.5f;

										  // Update eye positions using current head position
	glm::vec3 leftEyePos = COP - COPOffset;
	glm::vec3 rightEyePos = COP + COPOffset;

	glm::vec3 screenPos(0.f, 0.f, 0.f);
	glm::vec3 screenNormal(0.f, 0.f, 1.f);
	glm::vec3 screenUp(0.f, 1.f, 0.f);

	glm::ivec2 winSize = Renderer::getInstance().getPresentationWindowSize();

	float sizer = (m_fScreenDiagonalInches * 0.0254f) / sqrt(winSize.x * winSize.x + winSize.y * winSize.y);

	float width_m = winSize.x * sizer;
	float height_m = winSize.y * sizer;

	Renderer::SceneViewInfo* sviLE = Renderer::getInstance().getLeftEyeInfo();
	Renderer::SceneViewInfo* sviRE = Renderer::getInstance().getRightEyeInfo();
	sviLE->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	sviLE->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	sviLE->view = glm::translate(glm::mat4(), -leftEyePos);
	sviLE->projection = utils::getViewingFrustum(leftEyePos, screenPos, screenNormal, screenUp, glm::vec2(width_m, height_m));
	sviLE->viewport = glm::ivec4(0, 0, sviLE->m_nRenderWidth, sviLE->m_nRenderHeight);

	sviRE->m_nRenderWidth = Renderer::getInstance().getUIRenderSize().x;
	sviRE->m_nRenderHeight = Renderer::getInstance().getUIRenderSize().y;
	sviRE->view = glm::translate(glm::mat4(), -rightEyePos);
	sviRE->projection = utils::getViewingFrustum(rightEyePos, screenPos, screenNormal, screenUp, glm::vec2(width_m, height_m));
	sviRE->viewport = glm::ivec4(0, 0, sviRE->m_nRenderWidth, sviRE->m_nRenderHeight);
}

void CosmoStudyTrialDesktopScene::buildScalarPlane()
{
	CosmoGrid* cgrid = static_cast<CosmoGrid*>(m_pCosmoVolume->getDatasets()[0]);

	m_rsPlane.VAO = Renderer::getInstance().getPrimitiveVAO();;
	m_rsPlane.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("quaddouble");
	m_rsPlane.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("quaddouble");
	m_rsPlane.vertCount = Renderer::getInstance().getPrimitiveIndexCount("quaddouble");
	m_rsPlane.glPrimitiveType = GL_TRIANGLES;
	m_rsPlane.shaderName = "cosmo";
	m_rsPlane.indexType = GL_UNSIGNED_SHORT;
	m_rsPlane.diffuseColor = glm::vec4(cgrid->getMaxVelocity(), cgrid->getMaxDensity(), cgrid->getMaxH2IIDensity(), cgrid->getMaxTemperature());
	m_rsPlane.specularColor = glm::vec4(cgrid->getMinVelocity(), cgrid->getMinDensity(), cgrid->getMinH2IIDensity(), cgrid->getMinTemperature());
	m_rsPlane.diffuseTexName = "vectorfield";
	m_rsPlane.specularTexName = "vectorfieldattributes";
	m_rsPlane.hasTransparency = true;
}