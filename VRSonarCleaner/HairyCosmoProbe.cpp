#include "HairyCosmoProbe.h"

using namespace std::chrono_literals;

HairyCosmoProbe::HairyCosmoProbe(ViveController* pController, CosmoVolume* cosmoVolume)
	: ProbeBehavior(pController, cosmoVolume)
	, m_bProbeActive(false)
	, m_pCosmoVolume(cosmoVolume)
	, m_eSeedType(STREAMTUBES)
{
	m_vec4ProbeColorDiff = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);
}


HairyCosmoProbe::~HairyCosmoProbe()
{
}

void HairyCosmoProbe::update()
{
	ProbeBehavior::update();
}

void HairyCosmoProbe::draw()
{
	int gridRes = 20;
	float width = 0.25f;
	float height = 0.25f;

	float hairLength = 0.25f;

	if (m_pController)
	{
		drawProbe();

		glm::mat4 probeMat = getTransformProbeToWorld();
		glm::vec3 probePos = getPosition();

		for (int i = 0; i < gridRes; ++i)
		{
			float ratioWidth = gridRes == 1 ? 0.f : (float)i / (gridRes - 1) - 0.5f;

			for (int j = 0; j < gridRes; ++j)
			{
				float ratioHeight = gridRes == 1 ? 0.f : (float)j / (gridRes - 1);
				glm::vec3 pos = probePos + glm::vec3(probeMat[0]) * ratioWidth * width - glm::vec3(probeMat[2]) * ratioHeight * height;
				glm::vec3 flow = m_pCosmoVolume->getFlowWorldCoords(pos);
				float mag = glm::length(flow);
				Renderer::getInstance().drawPointerLit(pos, pos + flow * hairLength, 0.005f, glm::vec4(0.f, 0.f, 0.f, 0.5f), glm::vec4(1.f), glm::vec4(1.f, 1.f - glm::clamp(mag/0.75f, 0.f, 1.f), 0.f, 1.f));
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

void HairyCosmoProbe::activateProbe()
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

void HairyCosmoProbe::deactivateProbe()
{
	m_bProbeActive = false;
}