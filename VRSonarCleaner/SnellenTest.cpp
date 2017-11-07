#include "SnellenTest.h"

#include "InfoBoxManager.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include <gtc/quaternion.hpp>
#include "Renderer.h"

#include <random>
#include <sstream>

SnellenTest::SnellenTest(TrackedDeviceManager* pTDM, float visualAngle)
	: m_pTDM(pTDM)
	, m_fVisualAngle(visualAngle)
	, m_bWaitForTriggerRelease(true)
{
	std::cout << getHeightForOptotype(m_fVisualAngle) << std::endl;
}


SnellenTest::~SnellenTest()
{
}

void SnellenTest::init()
{
	m_vSloanLetters = { 'C', 'D', 'H', 'K', 'N', 'O', 'R', 'S', 'V', 'Z' };

	m_strCurrent = generateSnellenString();

	m_bInitialized = true;
}

void SnellenTest::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if (m_bWaitForTriggerRelease && !m_pTDM->getPrimaryController()->isTriggerEngaged() && !m_pTDM->getSecondaryController()->isTriggerEngaged())
		m_bWaitForTriggerRelease = false;

	if (!m_bWaitForTriggerRelease && m_pTDM->getPrimaryController()->justClickedTrigger())
		m_strCurrent = generateSnellenString();
}

void SnellenTest::draw()
{
	glm::mat4 hmdTrans = m_pTDM->getHMDToWorldTransform();

	// position 6m from eyes
	glm::vec3 stringPos = hmdTrans[3] - hmdTrans[2] * s_fDistance;

	// 88.6mm tall text @6m distance  = 1 minute of visual angle
	Renderer::getInstance().drawText(
		m_strCurrent,
		glm::vec4(1.f),
		stringPos,
		glm::quat(Renderer::getBillBoardTransform(stringPos, hmdTrans[3], hmdTrans[1], true)),
		getHeightForOptotype(m_fVisualAngle),
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		Renderer::TextAnchor::CENTER_MIDDLE,
		true
	);
}

void SnellenTest::newTest()
{
	m_strCurrent = generateSnellenString();
}

void SnellenTest::setVisualAngle(float visualAngleToTest)
{
	m_fVisualAngle = visualAngleToTest;
}

std::string SnellenTest::generateSnellenString()
{
	std::shuffle(m_vSloanLetters.begin(), m_vSloanLetters.end(), std::mt19937_64(std::random_device()()));

	std::stringstream ss;

	ss << m_vSloanLetters[0];

	for (int i = 1; i < s_nCharacters; ++i)
		ss << " " << m_vSloanLetters[i];

	std::cout << "Current Snellen string: " << ss.str();

	return ss.str();
}

float SnellenTest::getHeightForOptotype(float visualAngle)
{
	visualAngle *= s_fOptotypeGrating;
	visualAngle *= 0.5f;
	return 2.f * s_fDistance * glm::atan(glm::radians(visualAngle));
}
