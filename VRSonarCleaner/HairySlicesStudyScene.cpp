#include "HairySlicesStudyScene.h"
#include "BehaviorManager.h"
#include "LightingSystem.h"
#include <gtc/quaternion.hpp>
#include <gtc/random.hpp>
#include "utilities.h"
#include "DataLogger.h"
#include <random>


HairySlicesStudyScene::HairySlicesStudyScene(float displayDiagonalInches, TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pCosmoVolume(NULL)
	, m_pHairySlice(NULL)
	, m_bShowCosmoBBox(true)
	, m_bShowPlane(false)
	, m_bShowProbe(false)
	, m_bStudyActive(false)
	, m_bTraining(false)
	, m_bPaused(false)
	, m_bStudyComplete(false)
	, m_bCalibrated(false)
	, m_pEditParam(NULL)
	, m_fScreenDiagonalInches(displayDiagonalInches)
	, m_fEyeSeparationCentimeters(0.f)
	, m_bStereo(false)
	, m_nCurrentTrial(0)
	, m_nReplicatesPerCondition(15)
	, m_nCurrentReplicate(0)
	, m_strParticipantName("noname")
	, m_pCurrentCondition(NULL)
	, m_vec3ProbeDirection(0.f, 0.f, 1.f)
{
}


HairySlicesStudyScene::~HairySlicesStudyScene()
{
	DataLogger::getInstance().closeLog();
}

void HairySlicesStudyScene::init()
{
	srand(time(0)); // set random seed for std::random_shuffle()

	// Hide cursor on screen
	SDL_ShowCursor(0);

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
}

void HairySlicesStudyScene::processSDLEvent(SDL_Event & ev)
{
	if (m_bStudyActive)
	{
		if (m_bPaused)
		{
			if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
			{
				if (ev.key.keysym.sym == SDLK_RETURN)
				{
					endBreak();
				}
			}
		}
		else // study active
		{
			if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
			{
				if (ev.key.keysym.sym == SDLK_SPACE)
				{
					recordResponse();

					if (++m_nCurrentReplicate == m_nReplicatesPerCondition)
					{
						m_nCurrentReplicate = 0;

						m_pCurrentCondition = NULL;
						m_vStudyConditions.pop_back();

						startBreak();
					}

					m_nCurrentTrial++;

					m_pHairySlice->reseed();
					randomData();
					m_pHairySlice->set();
				}
			}
		}
	}
	else if (m_bTraining)
	{
		if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0)
		{
			if (ev.key.keysym.sym == SDLK_1)
			{
				m_pHairySlice->setGeomStyle("STREAMLET");
				m_pHairySlice->setShader("streamline_gradient_static");
			}

			if (ev.key.keysym.sym == SDLK_2)
			{
				m_pHairySlice->setGeomStyle("STREAMLET");
				m_pHairySlice->setShader("streamline_gradient_animated");
			}

			if (ev.key.keysym.sym == SDLK_3)
			{
				m_pHairySlice->setGeomStyle("CONE");
				m_pHairySlice->setShader("streamline_ring_static");
			}

			if (ev.key.keysym.sym == SDLK_4)
			{
				m_pHairySlice->setGeomStyle("TUBE");
				m_pHairySlice->setShader("streamline_gradient_static");
			}

			if (ev.key.keysym.sym == SDLK_5)
			{
				m_pHairySlice->setGeomStyle("TUBE");
				m_pHairySlice->setShader("streamline_gradient_animated");
			}

			if (ev.key.keysym.sym == SDLK_m)
			{
				m_pHairySlice->m_bOscillate = !m_pHairySlice->m_bOscillate;
			}

			if (ev.key.keysym.sym == SDLK_s)
			{
				m_bStereo = !m_bStereo;
				setupViews();
			}

			if (ev.key.keysym.sym == SDLK_g)
			{
				m_pHairySlice->m_bShowGrid = !m_pHairySlice->m_bShowGrid;
			}

			if (ev.key.keysym.sym == SDLK_t)
			{
				m_pHairySlice->m_bShowTarget = !m_pHairySlice->m_bShowTarget;
			}

			if (ev.key.keysym.sym == SDLK_p)
			{
				m_bShowProbe = !m_bShowProbe;
			}
			
			if (ev.key.keysym.sym == SDLK_HOME)
			{
				if (calibrateTracker())
					Renderer::getInstance().showMessage("Tracker calibration success!");
				else
					Renderer::getInstance().showMessage("ERROR: Tracker calibration FAILED!");
			}

			if (!(ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_SPACE)
			{
				m_pHairySlice->reseed();
				randomData();
				m_pHairySlice->set();
			}

			if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_SPACE)
			{
				startStudy();
			}
		}
	}
	else
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

					if (m_pEditParam->desc.compare("Name") == 0)
					{
						m_strParticipantName = m_pEditParam->buf;
					}

					if (m_pEditParam->desc.compare("IPD") == 0)
					{
						m_fEyeSeparationCentimeters = std::stof(m_pEditParam->buf);
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
					randomData();
					m_pHairySlice->set();
				}

				//if (ev.key.keysym.sym == SDLK_INSERT)
				//{
				//	std::stringstream ss;
				//	ss << m_pCosmoVolume->getPosition().x << "," << m_pCosmoVolume->getPosition().y << "," << m_pCosmoVolume->getPosition().z << "," << m_pCosmoVolume->getDimensions().x;
				//	DataLogger::getInstance().logMessage(ss.str());
				//}

				if (ev.key.keysym.sym == SDLK_HOME)
				{
					if (calibrateTracker())
						Renderer::getInstance().showMessage("Tracker calibration success!");
					else
						Renderer::getInstance().showMessage("ERROR: Tracker calibration FAILED!");
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

				if (ev.key.keysym.sym == SDLK_i)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("IPD") == 0;
					}));
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

				if (ev.key.keysym.sym == SDLK_n)
				{
					m_pEditParam = &(*std::find_if(m_vParams.begin(), m_vParams.end(), [](StudyParam p) {
						return p.desc.compare("Name") == 0;
					}));
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

				if (!(ev.key.keysym.mod & KMOD_SHIFT) && ev.key.keysym.sym == SDLK_s)
				{
					m_pHairySlice->m_bShowSeeds = !m_pHairySlice->m_bShowSeeds;
					Renderer::getInstance().showMessage("Seed visibility set to " + std::to_string(m_pHairySlice->m_bShowSeeds));
				}

				if ((ev.key.keysym.mod & KMOD_SHIFT) && ev.key.keysym.sym == SDLK_s)
				{
					m_bStereo = !m_bStereo;
					setupViews();
					Renderer::getInstance().showMessage("Stereo set to " + std::to_string(m_bStereo));
				}

				if (ev.key.keysym.sym == SDLK_t)
				{
					m_pHairySlice->m_bShowTarget = !m_pHairySlice->m_bShowTarget;
					Renderer::getInstance().showMessage("Target visibility set to " + std::to_string(m_pHairySlice->m_bShowTarget));
				}

				if (ev.key.keysym.sym == SDLK_v)
				{
					m_bShowProbe = !m_bShowProbe;
					Renderer::getInstance().showMessage("Probe visibility set to " + std::to_string(m_bShowProbe));
				}

				if ((ev.key.keysym.mod & KMOD_SHIFT) && ev.key.keysym.sym == SDLK_RETURN)
				{
					m_pHairySlice->reseed();
					Renderer::getInstance().showMessage("Hairy Slice reseeded");
				}

				if (!(ev.key.keysym.mod & KMOD_SHIFT) && ev.key.keysym.sym == SDLK_RETURN)
				{
					m_pHairySlice->set();
				}

				if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == SDLK_SPACE)
				{
					if (m_strParticipantName.compare("noname") == 0)
						Renderer::getInstance().showMessage("Please set participant name using (n) before starting training!");
					else if (m_fEyeSeparationCentimeters == 0.f)
						Renderer::getInstance().showMessage("Please measure and set participant IPD using (i) before starting training!");
					else if (!m_bCalibrated && m_pTDM)
						Renderer::getInstance().showMessage("Please make sure the tracking probe is calibrated to the monitor!");
					else
						startTraining();
				}
			}
		}
	}
}

void HairySlicesStudyScene::update()
{
	m_pCosmoVolume->update();

	LightingSystem::Light* l = LightingSystem::getInstance().getLight(0);
	l->direction = glm::inverse(Renderer::getInstance().getLeftEyeInfo()->view) * glm::normalize(glm::vec4(-1.f, -1.f, -1.f, 0.f));

	m_pHairySlice->update();

	if (m_pTDM && m_pTDM->getTracker())
	{
		m_pTDM->getTracker()->hideRenderModel();
		m_vec3ProbeDirection = m_mat4TrackingToScreen * (-m_pTDM->getTracker()->getDeviceToWorldTransform()[1]);
	}
}

void HairySlicesStudyScene::draw()
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

	if (m_bTraining)
	{
		std::stringstream ss;
		ss << "Geometry: " << m_pHairySlice->getGeomStyle() << std::endl;
		ss << "Texture: " << ( m_pHairySlice->getShaderName().find("static") != std::string::npos ? "Static" : "Animated" )<< std::endl;
		ss << "Stereo 3D: " << (m_bStereo ? "On" : "Off") << std::endl;
		ss << "Motion: " << (m_pHairySlice->m_bOscillate ? "On" : "Off");

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(1.f),
			glm::vec3(0.f, Renderer::getInstance().getUIRenderSize().y, 0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().y / 5.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::TOP_LEFT
		);

		ss.str("");
		ss.clear();

		ss.precision(2);

		float accuracy = glm::degrees(glm::acos(glm::dot(glm::normalize(m_pHairySlice->m_vec3FlowAtReticule), m_vec3ProbeDirection)));
		bool flipped = accuracy > 90.f;
		
		ss << "Error: " << std::fixed << accuracy << "deg";

		if (flipped)
			ss << " (REVERSED)";

		glm::vec3 accuracyTextColor;

		if (accuracy < 10.f)
			accuracyTextColor = glm::mix(glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), glm::mod(accuracy, 10.f) / 10.f);
		else if (accuracy < 20.f)
			accuracyTextColor = glm::mix(glm::vec3(1.f, 1.f, 0.f), glm::vec3(1.f, 0.5f, 0.f), glm::mod(accuracy - 10.f, 10.f) / 10.f);
		else if (accuracy < 50.f)
			accuracyTextColor = glm::mix(glm::vec3(1.f, 0.5f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::mod(accuracy - 20.f, 30.f) / 30.f);
		else if (accuracy < 90.f)
			accuracyTextColor = glm::mix(glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.1f, 0.1f, 0.1f), glm::mod(accuracy - 50.f, 40.f) / 40.f);
		else
			accuracyTextColor = glm::vec3(0.1f, 0.1f, 0.1f);

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(accuracyTextColor, 1.f),
			glm::vec3(0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().y / 10.f,
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	if (m_bPaused)
	{
		float pulseRate = 5.f;
		float ratio = glm::mod(Renderer::getInstance().getElapsedSeconds(), pulseRate) / pulseRate;
		float easeAmount = glm::sin(glm::two_pi<float>() * ratio);

		//Renderer::getInstance().setClearColor(glm::vec4(glm::mix(glm::vec3(0.4f, 0.4f, 0.6f), glm::vec3(0.6f, 0.6f, 0.4f), easeAmount), 1.f));

		std::stringstream ss;

		ss << "Please take a short break." << std::endl;
		ss << "Press <ENTER> to resume.";

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(glm::mix(glm::vec3(0.6f, 0.6f, 0.4f), glm::vec3(0.4f, 0.4f, 0.6f), easeAmount), 1.f),
			glm::vec3(glm::vec2(Renderer::getInstance().getUIRenderSize()) * 0.5f, 0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().x / 3.f,
			Renderer::WIDTH,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);

		float progress = (m_nCurrentTrial) / (20.f * m_nReplicatesPerCondition);

		ss.str("");
		ss.clear();
		ss.precision(1);

		ss << std::fixed << progress * 100.f << "% COMPLETE";

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(glm::mix(glm::vec3(1.f), glm::vec3(0.2f, 0.9f, 0.2f), progress), easeAmount),
			glm::vec3(Renderer::getInstance().getUIRenderSize().x * 0.5f, 0.f, 0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().x / 3.f,
			Renderer::WIDTH,
			Renderer::CENTER,
			Renderer::CENTER_BOTTOM
		);
	}

	if (m_bStudyComplete)
	{
		Renderer::getInstance().drawUIText(
			"COMPLETE",
			glm::vec4(1.f),
			glm::vec3(glm::vec2(Renderer::getInstance().getUIRenderSize()) * 0.5f, 0.f),
			glm::quat(),
			Renderer::getInstance().getUIRenderSize().x / 2.f,
			Renderer::WIDTH,
			Renderer::CENTER,
			Renderer::CENTER_MIDDLE
		);
	}
	
	m_rsPlane.modelToWorldTransform = m_pCosmoVolume->getTransformRawDomainToVolume();

	if (m_bShowPlane)
		Renderer::getInstance().addToDynamicRenderQueue(m_rsPlane);

	if (m_bShowProbe)
		Renderer::getInstance().drawPointerLit(glm::vec3(0.f), glm::normalize(m_vec3ProbeDirection) * 0.1f, 0.01f, glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f));

	m_pHairySlice->draw();
}


void HairySlicesStudyScene::setupParameters()
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

	m_vParams.push_back({ "Name" , m_strParticipantName, STUDYPARAM_ALPHA | STUDYPARAM_NUMERIC });
	m_vParams.push_back({ "IPD" , std::to_string(m_fEyeSeparationCentimeters), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });

	m_vParams.push_back({ "Display Diag (inches)" , std::to_string(m_fScreenDiagonalInches), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL });
	m_vParams.push_back({ "Clear Color" , utils::color::rgb2str(Renderer::getInstance().getClearColor()), STUDYPARAM_NUMERIC | STUDYPARAM_DECIMAL | STUDYPARAM_RGBA });
}

void HairySlicesStudyScene::setupViews()
{
	Renderer::Camera* cam = Renderer::getInstance().getCamera();
	cam->pos = glm::vec3(0.f, 0.f, 0.57f);
	cam->lookat = glm::vec3(0.f);
	cam->up = glm::vec3(0.f, 1.f, 0.f);

	float eyeSeparationMeters = m_fEyeSeparationCentimeters / 100.f;

	glm::vec3 COP = cam->pos;
	glm::vec3 COPOffset = m_bStereo ? glm::vec3(1.f, 0.f, 0.f) * eyeSeparationMeters * 0.5f : glm::vec3(0.f);

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

	Renderer::getInstance().setStereoRenderSize(winSize);

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

void HairySlicesStudyScene::buildScalarPlane()
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

void HairySlicesStudyScene::startTraining()
{
	m_bTraining = true;

	m_bShowCosmoBBox = false;
	m_bShowPlane = false;
	m_bShowProbe = false;
	m_pHairySlice->m_bJitterSeeds = true;
	m_pHairySlice->m_bShowFrame = true;
	m_pHairySlice->m_bShowGeometry = true;
	m_pHairySlice->m_bShowGrid = true;
	m_pHairySlice->m_bShowHalos = false;
	m_pHairySlice->m_bShowReticule = true;
	m_pHairySlice->m_bSpinReticule = true;
	m_pHairySlice->m_bShowSeeds = false;
	m_pHairySlice->m_bShowTarget = false;

	m_pHairySlice->reseed();
	randomData();
	m_pHairySlice->set();
}

void HairySlicesStudyScene::makeStudyConditions()
{
	for (auto &geom : { "CONE", "TUBE", "STREAMLET" })
	{
		for (auto &tex : { "STATIC", "ANIMATED" })
		{
			if (geom == "CONE" && tex == "ANIMATED")
				continue;

			for (auto &motion : { true, false })
			{
				for (auto &stereo : { true, false })
				{
					m_vStudyConditions.push_back(StudyCondition({ geom, tex, stereo, motion }));
				}
			}
		}
	}
	 
	std::random_shuffle(m_vStudyConditions.begin(), m_vStudyConditions.end());
}

void HairySlicesStudyScene::startStudy()
{
	Renderer::getInstance().setClearColor(glm::vec4(0.4f, 0.4f, 0.6f, 1.f));

	DataLogger::getInstance().setLogDirectory("/");
	DataLogger::getInstance().setID(m_strParticipantName);
	DataLogger::getInstance().openLog(m_strParticipantName);
	DataLogger::getInstance().setHeader("trial,ipd,geometry,texture,motion,stereo,target.pos.x,target.pos.y,target.pos.z,target.dir.x,target.dir.y,target.dir.z,target.magnitude,probe.x,probe.y,probe.z");
	DataLogger::getInstance().start();

	m_bTraining = false;

	m_bShowCosmoBBox = false;
	m_bShowPlane = false;
	m_bShowProbe = false;
	m_pHairySlice->m_bJitterSeeds = true;
	m_pHairySlice->m_bShowFrame = true;
	m_pHairySlice->m_bShowGeometry = true;
	m_pHairySlice->m_bShowGrid = true;
	m_pHairySlice->m_bShowHalos = false;
	m_pHairySlice->m_bShowReticule = true;
	m_pHairySlice->m_bSpinReticule = true;
	m_pHairySlice->m_bShowSeeds = false;
	m_pHairySlice->m_bShowTarget = false;

	makeStudyConditions();

	loadStudyCondition();

	m_nCurrentTrial = 0;

	m_nCurrentReplicate = 0;

	m_pHairySlice->reseed();
	randomData();
	m_pHairySlice->set();

	m_bStudyActive = true;
}

void HairySlicesStudyScene::loadStudyCondition()
{
	if (!m_vStudyConditions.empty())
	{
		m_pCurrentCondition = &m_vStudyConditions.back();
		
		m_pHairySlice->setGeomStyle(m_pCurrentCondition->geometry);

		if (m_pCurrentCondition->geometry.compare("CONE") == 0)
			m_pHairySlice->setShader("streamline_ring_static");
		else if (m_pCurrentCondition->texture.compare("STATIC") == 0)
			m_pHairySlice->setShader("streamline_gradient_static");
		else
			m_pHairySlice->setShader("streamline_gradient_animated");

		m_pHairySlice->m_bOscillate = m_pCurrentCondition->motion;

		m_bStereo = m_pCurrentCondition->stereo;
		setupViews();
	}
}

void HairySlicesStudyScene::startBreak()
{
	m_pHairySlice->m_bShowGeometry = false;
	m_pHairySlice->m_bShowFrame = false;
	m_pHairySlice->m_bShowGrid = false;
	m_pHairySlice->m_bShowReticule = false;

	loadStudyCondition();

	if (!m_pCurrentCondition)
	{
		m_bStudyActive = false;
		m_bStudyComplete = true;

		DataLogger::getInstance().closeLog();
	}
	else
	{
		m_bPaused = true;
		Renderer::getInstance().setClearColor(glm::vec4(0.6f, 0.6f, 0.6f, 1.f));
	}
}

void HairySlicesStudyScene::endBreak()
{
	m_bPaused = false;

	Renderer::getInstance().setClearColor(glm::vec4(0.4f, 0.4f, 0.6f, 1.f));

	m_pHairySlice->m_bShowGeometry = true;
	m_pHairySlice->m_bShowFrame = true;
	m_pHairySlice->m_bShowGrid = true;
	m_pHairySlice->m_bShowReticule = true;
	
}

void HairySlicesStudyScene::recordResponse()
{
	glm::vec3 flowNorm = glm::normalize(m_pHairySlice->m_vec3FlowAtReticule);

	std::stringstream ss;

	ss << m_nCurrentTrial << ",";
	ss << m_fEyeSeparationCentimeters << ",";
	ss << m_pCurrentCondition->geometry << ",";
	ss << m_pCurrentCondition->texture << ",";
	ss << m_pCurrentCondition->motion << ",";
	ss << m_pCurrentCondition->stereo << ",";
	ss << m_pHairySlice->m_vec3Reticule.x << ",";
	ss << m_pHairySlice->m_vec3Reticule.y << ",";
	ss << m_pHairySlice->m_vec3Reticule.z << ",";
	ss << flowNorm.x << ",";
	ss << flowNorm.y << ",";
	ss << flowNorm.z << ",";
	ss << glm::length(m_pHairySlice->m_vec3FlowAtReticule) << ",";
	ss << m_vec3ProbeDirection.x << ",";
	ss << m_vec3ProbeDirection.y << ",";
	ss << m_vec3ProbeDirection.z;

	DataLogger::getInstance().logMessage(ss.str());
}

void HairySlicesStudyScene::randomData()
{
	m_pCosmoVolume->setDimensions(m_pCosmoVolume->getOriginalDimensions() * 4.f);
	m_pCosmoVolume->setPosition(glm::linearRand(-(m_pCosmoVolume->getOriginalDimensions() / 2.f), (m_pCosmoVolume->getOriginalDimensions() / 2.f)));

	glm::vec3 w = glm::normalize(glm::sphericalRand(1.f));
	glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), w));
	glm::vec3 v = glm::normalize(glm::cross(w, u));

	m_pCosmoVolume->setOrientation(glm::mat3(u, v, w));
}

bool HairySlicesStudyScene::calibrateTracker()
{
	m_bCalibrated = false;

	if (m_pTDM && m_pTDM->getTracker())
	{
		// build coord frame using y axis of tracker for z axis of coordinate frame to match screen
		glm::vec3 w = glm::normalize(-m_pTDM->getTracker()->getDeviceToWorldTransform()[1]);
		glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), w));
		glm::vec3 v = glm::normalize(glm::cross(w, u));
		glm::mat3 temp(u, v, w);
		m_mat4TrackingToScreen = glm::inverse(temp);

		m_bCalibrated = true;
	}

	return m_bCalibrated;
}
