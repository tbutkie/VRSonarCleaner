#include "FlowFieldCurator.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std::chrono_literals;

FlowFieldCurator::FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol)
	: m_pTDM(pTDM)
	, m_pFlowVolume(flowVol)
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
}

void FlowFieldCurator::draw()
{

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
				if (cpAttr.compare("POINT") == 0)
					CPMap[cpName].pos = glm::vec3(vals[0], vals[1], vals[2]);
				if (cpAttr.compare("DIRECTION") == 0)
					CPMap[cpName].dir = glm::vec3(vals[0], vals[1], vals[2]);
				if (cpAttr.compare("LAMBDA") == 0)
					CPMap[cpName].lamda = glm::vec3(vals[0], vals[1], vals[2]);
			}
			else
				continue;
		}

		cpFile.close();

		m_vCPs = CPMap;
	}
}
