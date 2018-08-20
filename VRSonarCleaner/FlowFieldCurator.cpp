#include "FlowFieldCurator.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std::chrono_literals;

FlowFieldCurator::FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol, int gridRes)
	: m_pTDM(pTDM)
	, m_pFlowVolume(flowVol)
	, m_pvec3MovingPt(NULL)
	, m_iGridRes(gridRes)
	, m_mat4CPOffsetTransform(glm::translate(glm::mat4(), glm::vec3(1.f)) * glm::scale(glm::mat4(), glm::vec3(static_cast<float>(gridRes - 1) / 2.f)) * glm::translate(glm::mat4(), glm::vec3(1.f)))
	, m_mat4CPOffsetTransformInv(glm::inverse(m_mat4CPOffsetTransform))
{
	m_pFlowVolume->removeFlowGrid("resources/data/flowgrid/test.fg");
	loadRandomFlowGrid();
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

	if (m_pTDM->getPrimaryController())
	{
		if (m_pTDM->getPrimaryController()->justPressedMenu())
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
	glm::vec3 dimratio = m_pFlowVolume->getDimensions() / m_pFlowVolume->getOriginalDimensions();
	for (auto &cp : m_vCPs)
	{
		glm::vec3 connectorVec(cp.second.end_world - cp.second.pos_world);
		float connectorRadius = 0.005f * glm::length(dimratio);
		Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), cp.second.pos_world) * glm::scale(glm::mat4(), glm::vec3(0.01f * dimratio)), glm::vec4(1.f));
		Renderer::getInstance().drawPointerLit(cp.second.end_world, cp.second.end_world + connectorVec * 0.1f + glm::normalize(connectorVec) * connectorRadius, connectorRadius * 2.f, glm::vec4(0.9f, 0.f, 0.f, 1.f), glm::vec4(1.f));
		Renderer::getInstance().drawConnectorLit(cp.second.pos_world, cp.second.end_world, connectorRadius, glm::vec4(0.9f, 0.9f, 0.9f, 1.f), glm::vec4(1.f));
	}

	if (m_pTDM->getPrimaryController())
	{
		glm::mat4 menuButtonPose = m_pTDM->getDeviceComponentPose(m_pTDM->getPrimaryController()->getIndex(), m_pTDM->getDeviceComponentID(m_pTDM->getPrimaryController()->getIndex(), "button"));
		glm::mat4 menuButtonTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.025f, 0.01f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		Renderer::getInstance().drawText(
			"Load Random\nFlowGrid",
			glm::vec4(1.f),
			menuButtonTextAnchorTrans[3],
			glm::quat(menuButtonTextAnchorTrans),
			0.015f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_LEFT
		);
		Renderer::getInstance().drawConnector(
			menuButtonTextAnchorTrans[3],
			menuButtonPose[3],
			0.001f,
			glm::vec4(1.f, 1.f, 1.f, 0.75f)
		);
	}

	if (m_pTDM->getSecondaryController())
	{
		glm::mat4 touchButtonPose = m_pTDM->getDeviceComponentPose(m_pTDM->getSecondaryController()->getIndex(), m_pTDM->getDeviceComponentID(m_pTDM->getSecondaryController()->getIndex(), "trackpad"));
		glm::mat4 touchpadTextAnchorTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(-0.025f, 0.02f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		Renderer::getInstance().drawText(
			"Grab Closest\nControl Point",
			m_pTDM->getSecondaryController()->isTouchpadClicked() ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(1.f),
			touchpadTextAnchorTrans[3],
			glm::quat(touchpadTextAnchorTrans),
			0.015f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_RIGHT
		);
		Renderer::getInstance().drawConnector(
			touchpadTextAnchorTrans[3],
			touchButtonPose[3],
			0.001f,
			glm::vec4(1.f, 1.f, 1.f, 0.75f)
		);
	}
}

bool FlowFieldCurator::loadRandomFlowGrid()
{
	std::stringstream ss;
	ss << "resources\\data\\flowgrid\\VecFieldGen.exe -outfile flowgrid_test.fg -grid " << m_iGridRes;
	if (system(ss.str().c_str()) == EXIT_SUCCESS)
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
	
	std::stringstream ss;
	ss << "resources\\data\\flowgrid\\VecFieldGen.exe -outfile flowgrid_test.fg -cpfile tmp.flowfieldcurator.cp -grid " << m_iGridRes;

	if (system(ss.str().c_str()) == EXIT_SUCCESS)
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