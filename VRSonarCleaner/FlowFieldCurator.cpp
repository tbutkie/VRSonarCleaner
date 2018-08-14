#include "FlowFieldCurator.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std::chrono_literals;

FlowFieldCurator::FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol)
	: m_pTDM(pTDM)
	, m_pFlowVolume(flowVol)
	, m_pvec3MovingPt(NULL)
	, m_mat4CPOffsetTransform(glm::translate(glm::mat4(), glm::vec3(16.f)) * glm::scale(glm::mat4(), glm::vec3(31.f / 2.f)))
	, m_mat4CPOffsetTransformInv(glm::inverse(m_mat4CPOffsetTransform))
{
	
}


FlowFieldCurator::~FlowFieldCurator()
{
}

void FlowFieldCurator::update()
{
	for (auto &cp : m_vCPs)
	{
		cp.second.pos_world = m_pFlowVolume->convertToWorldCoords(cp.second.pos);
		cp.second.end_world = m_pFlowVolume->convertToWorldCoords(cp.second.end);
	}

	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedMenu())
	{
		m_pFlowVolume->removeFlowGrid("resources/data/flowgrid/test.fg");
		loadRandomFlowGrid();
	}

	if (m_pTDM->getSecondaryController())
	{
		if (m_pTDM->getSecondaryController()->justPressedTouchpad())
		{
			// Grab closest control point
			glm::vec3 ctrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			float closestSqDist = std::numeric_limits<float>::max();

			for (auto &cp : m_vCPs)
			{
				float dist = glm::length2(cp.second.pos_world - ctrlrPos);
				if (dist < closestSqDist)
				{
					closestSqDist = dist;
					m_pvec3MovingPt = &cp.second.pos;
					m_vec3ControllerToMovingPt = (cp.second.pos_world) - ctrlrPos;
				}

				dist = glm::length2(cp.second.end_world - ctrlrPos);
				if (dist < closestSqDist)
				{
					closestSqDist = dist;
					m_pvec3MovingPt = &cp.second.end;
					m_vec3ControllerToMovingPt = (cp.second.end_world) - ctrlrPos;
				}
			}

			
		}

		if (m_pTDM->getSecondaryController()->isTouchpadClicked())
		{
			*m_pvec3MovingPt = glm::vec4(m_pFlowVolume->convertToRawDomainCoords(glm::vec3(m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3]) + m_vec3ControllerToMovingPt), 1.f);
		}

		if (m_pTDM->getSecondaryController()->justUnpressedTouchpad())
		{
			// Release control point and load new flow field
			recalculateRawPositions();
			loadFlowGridFromCurrentCPs();
			m_pvec3MovingPt = NULL;
		}
	}
}

void FlowFieldCurator::draw()
{
	for (auto &cp : m_vCPs)
	{
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), cp.second.pos_world) * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f));
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), cp.second.end_world) * glm::scale(glm::mat4(), glm::vec3(0.01f)), glm::vec4(1.f, 0.f, 0.f, 1.f));
		Renderer::getInstance().drawConnector(cp.second.pos_world, cp.second.end_world, 0.005f, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
	}
}

bool FlowFieldCurator::loadRandomFlowGrid()
{
	if (system("resources\\data\\flowgrid\\VecFieldGen.exe -outfile flowgrid_test.fg") == EXIT_SUCCESS)
	{
		m_pFlowVolume->addFlowGrid("flowgrid_test.fg", true);
		loadMetaFile("flowgrid_test.fg.cp");

		return true;
	}

	return false;
}

bool FlowFieldCurator::loadFlowGridFromCurrentCPs()
{
	using namespace std::experimental::filesystem::v1;

	std::ofstream cpFile;

	cpFile.open("tmp.flowfieldcurator.cp");

	if (cpFile.is_open())
	{
		for (auto &cp : m_vCPs)
		{
			cpFile << cp.first << "_POINT," << cp.second.pos_raw.x << "," << cp.second.pos_raw.y << "," << cp.second.pos_raw.z << std::endl;
			cpFile << cp.first << "_DIRECTION," << cp.second.dir.x << "," << cp.second.dir.y << "," << cp.second.dir.z << std::endl;
		}

		cpFile.close();
	}

	if (system("resources\\data\\flowgrid\\VecFieldGen.exe -outfile flowgrid_test.fg -cpfile tmp.flowfieldcurator.cp") == EXIT_SUCCESS)
	{
		m_pFlowVolume->addFlowGrid("flowgrid_test.fg", true);
		loadMetaFile("flowgrid_test.fg.cp");

		return true;
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

				if (cpAttr.compare("POINT") == 0)
				{
					CPMap[cpName].pos_raw = vecData;
					CPMap[cpName].pos = glm::vec3(m_mat4CPOffsetTransform * glm::vec4(vecData, 1.f));
				}
				if (cpAttr.compare("DIRECTION") == 0)
					CPMap[cpName].dir = vecData;
				if (cpAttr.compare("LAMBDA") == 0)
					CPMap[cpName].lamda = vecData;
			}
			else
				continue;
		}

		cpFile.close();

		for (auto &cp : CPMap)
			cp.second.end = glm::vec3(m_mat4CPOffsetTransform * glm::vec4(cp.second.pos_raw + cp.second.dir, 1.f));

		m_vCPs = CPMap;
	}
}


void FlowFieldCurator::recalculateRawPositions()
{
	for (auto &cp : m_vCPs)
	{
		cp.second.pos_raw = glm::vec3(m_mat4CPOffsetTransformInv * glm::vec4(cp.second.pos, 1.f));
		glm::vec3 endRaw = glm::vec3(m_mat4CPOffsetTransformInv * glm::vec4(cp.second.end, 1.f));
		cp.second.dir = endRaw - cp.second.pos_raw;
	}
}