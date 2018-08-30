#include "HairyFlowProbe.h"

using namespace std::chrono_literals;

HairyFlowProbe::HairyFlowProbe(ViveController* pController, FlowVolume* flowVolume)
	: ProbeBehavior(pController, flowVolume)
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
	int gridRes = 10;
	float width = 0.1f;
	float height = 0.1f;

	if (m_pController)
	{
		drawProbe(m_fProbeOffset);

		glm::mat4 probeMat = getTransformProbeToWorld();
		glm::vec3 probePos = getPosition();

		for (int i = 0; i < gridRes; ++i)
		{
			float ratioWidth = gridRes == 1 ? 0.f : (float)i / (gridRes - 1) - 0.5f;

			for (int j = 0; j < gridRes; ++j)
			{
				float ratioHeight = gridRes == 1 ? 0.f : (float)j / (gridRes - 1);
				glm::vec3 pos = probePos + glm::vec3(probeMat[0]) * ratioWidth * width - glm::vec3(probeMat[2]) * ratioHeight * height;
				glm::vec3 flow = m_pFlowVolume->getFlowWorldCoords(pos);
				Renderer::getInstance().drawPointerLit(pos, pos + flow, 0.001f, glm::vec4(1.f, 1.f, 0.f, 1.f));
				//Renderer::getInstance().drawDirectedPrimitive("cylinder", pos, pos + flow, 0.005f, glm::vec4(1.f, 1.f, 0.f, 1.f));
			}
		}

		glm::vec3 x0y0 = probePos + glm::vec3(probeMat[0]) * -0.5f * width;
		glm::vec3 x0y1 = probePos + glm::vec3(probeMat[0]) * -0.5f * width - glm::vec3(probeMat[2]) * height;
		glm::vec3 x1y0 = probePos + glm::vec3(probeMat[0]) * 0.5f * width;
		glm::vec3 x1y1 = probePos + glm::vec3(probeMat[0]) * 0.5f * width - glm::vec3(probeMat[2]) * height;

		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y0, x0y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x0y1, x1y1, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y1, x1y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", x1y0, x0y0, 0.001f, glm::vec4(0.7f, 0.7f, 0.7f, 1.f));

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