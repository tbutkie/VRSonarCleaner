#include "DebugProbe.h"
#include "Renderer.h"

#include <sstream>

using namespace std::chrono_literals;

DebugProbe::DebugProbe(ViveController* pController, DataVolume* dataVolume)
	: ProbeBehavior(pController, dataVolume)
	, m_bProbeActive(false)
	, m_pDataVolume(dataVolume)
{
	m_vec4ProbeColorDiff = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
}


DebugProbe::~DebugProbe()
{
}

void DebugProbe::update()
{
	ProbeBehavior::update();
}

void DebugProbe::draw()
{
	if (m_pController)
	{
		drawProbe(m_fProbeOffset);

		glm::vec3 probePos = getPosition();

		glm::mat4 domainTrans = m_pDataVolume->getTransformRawDomainToVolume();
		glm::vec3 probePosDomain = m_pDataVolume->convertToRawDomainCoords(probePos);

		glm::mat4 menuButtonTextAnchorTrans = getTransformProbeToWorld() * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		std::stringstream ss;
		ss << "x: " << probePosDomain.x << std::endl;
		ss << "y: " << probePosDomain.y << std::endl;
		ss << "z: " << probePosDomain.z;

		Renderer::getInstance().drawText(
			ss.str(),
			glm::vec4(1.f),
			menuButtonTextAnchorTrans[3],
			glm::quat(menuButtonTextAnchorTrans),
			0.05f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::LEFT,
			Renderer::TextAnchor::TOP_LEFT
		);
	}
}

void DebugProbe::activateProbe()
{
	//if (m_pTDM->getPrimaryController())
	//{
	//	auto pos = getPosition();
	//	if (m_pDataVolume->isWorldCoordPointInDomainBounds(pos))
	//	{
	//		glm::dvec3 ptXform = m_pDataVolume->convertToRawDomainCoords(pos);
	//		printf("World Pos (%0.4f, %0.4f, %0.4f) is INSIDE the data volume at (%0.4f, %0.4f, %0.4f).\n", pos.x, pos.y, pos.z, ptXform.x, ptXform.y, ptXform.z);
	//	}
	//	else
	//		printf("World Pos (%0.4f, %0.4f, %0.4f) is outside the data volume.\n", pos.x, pos.y, pos.z);
	//}
}

void DebugProbe::deactivateProbe()
{
	m_bProbeActive = false;
}