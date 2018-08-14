#include "FlowFieldCurator.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std::chrono_literals;

FlowFieldCurator::FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol)
	: m_pTDM(pTDM)
	, m_pFlowVolume(flowVol)
	, m_pvec3MovingPt(NULL)
{
}


FlowFieldCurator::~FlowFieldCurator()
{
}

void FlowFieldCurator::update()
{
	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedTouchpad())
	{
		m_pFlowVolume->removeFlowGrid("resources/data/flowgrid/test.fg");
		loadRandomFlowGrid();
	}

	if (m_pTDM->getSecondaryController() && m_pTDM->getPrimaryController()->justPressedTouchpad())
	{
		// Grab closest control point
	}

	if (m_pTDM->getSecondaryController() && m_pTDM->getPrimaryController()->justUnpressedTouchpad())
	{
		// Release control point and load new flow field
		m_pvec3MovingPt = NULL;
	}
}

void FlowFieldCurator::draw()
{
	for (auto &cp : m_vCPs)
	{
		glm::vec3 pt = m_pFlowVolume->convertToWorldCoords(cp.second.pos);
		glm::vec3 endPt = pt + glm::vec3(m_pFlowVolume->getCurrentVolumeTransform() * glm::vec4(cp.second.dir, 0.f));

		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), pt) * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f));
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), endPt) * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f, 0.f, 0.f, 1.f));
		Renderer::getInstance().drawConnector(pt, endPt, 0.005f, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
	}
}

bool FlowFieldCurator::loadRandomFlowGrid()
{
	if (system("resources\\data\\flowgrid\\VecFieldGen.exe -outfile flowgrid_test.fg") == EXIT_SUCCESS)
	{
		m_pFlowVolume->addFlowGrid("flowgrid_test.fg", true);
		loadMetaFile("flowgrid_test.fg.cp");
	}

	return false;
}

void FlowFieldCurator::loadMetaFile(std::string metaFileName)
{
	using namespace std::experimental::filesystem::v1;

	std::ifstream cpFile;

	cpFile.open(metaFileName);

	std::unordered_map<std::string, ControlPoint> CPMap;

	if (cpFile.is_open())
	{
		std::string line;
		std::vector<float> cpvals;

		while (std::getline(cpFile, line))
		{
			std::stringstream ss(line);

			std::string cpName, cpAttr;
			std::getline(ss, cpName, '_');
			std::getline(ss, cpAttr, ',');

			std::string cell;
			std::vector<float> vals;

			while (std::getline(ss, cell, ','))
			{
				float tmpVal = std::stof(cell);
				vals.push_back(tmpVal);
			}

			if (vals.size() == 3)
			{
				glm::vec3 vecData(vals[0], vals[1], vals[2]);
				glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(16.f)) * glm::scale(glm::mat4(), glm::vec3(31.f/2.f));

				if (cpAttr.compare("POINT") == 0)
					CPMap[cpName].pos = glm::vec3(trans * glm::vec4(vecData, 1.f));
				if (cpAttr.compare("DIRECTION") == 0)
					CPMap[cpName].dir = vecData;
				if (cpAttr.compare("LAMBDA") == 0)
					CPMap[cpName].lamda = vecData;
			}
			else
				continue;
		}

		cpFile.close();

		m_vCPs = CPMap;
	}
}
