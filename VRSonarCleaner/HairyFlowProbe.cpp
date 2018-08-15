#include "HairyFlowProbe.h"

using namespace std::chrono_literals;

HairyFlowProbe::HairyFlowProbe(TrackedDeviceManager* pTDM, FlowVolume* flowVolume)
	: ProbeBehavior(pTDM, flowVolume)
	, m_bProbeActive(false)
	, m_pFlowVolume(flowVolume)
{
	m_vec4ProbeColorDiff = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
}


HairyFlowProbe::~HairyFlowProbe()
{
}

void HairyFlowProbe::update()
{
	ProbeBehavior::update();
}

void HairyFlowProbe::draw()
{
	if (m_pTDM->getPrimaryController())
	{
		drawProbe(m_fProbeOffset);

		glm::mat4 probeMat = getProbeToWorldTransform();

		for (int i = 0; i < 1; ++i)
		{
			glm::mat4 mat = probeMat;
			glm::vec3 flow = m_pFlowVolume->getFlowWorldCoords(getPosition());
			Renderer::getInstance().drawConnector(getPosition(), getPosition() + flow, 0.01f, glm::vec4(1.f, 1.f, 0.f, 1.f));
		}
	}
}

void HairyFlowProbe::activateProbe()
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

void HairyFlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}