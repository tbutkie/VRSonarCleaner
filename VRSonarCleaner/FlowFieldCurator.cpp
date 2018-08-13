#include "FlowFieldCurator.h"

using namespace std::chrono_literals;

FlowFieldCurator::FlowFieldCurator(TrackedDeviceManager* pTDM)
	: m_pFlowVolume(NULL)
{
}


FlowFieldCurator::~FlowFieldCurator()
{
	if (m_pFlowVolume)
		delete m_pFlowVolume;
}

void FlowFieldCurator::update()
{
	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedTouchpad)
		loadRandomFlowGrid();
}

void FlowFieldCurator::draw()
{

}

bool FlowFieldCurator::loadRandomFlowGrid()
{
	if (m_pFlowVolume)
		delete m_pFlowVolume;

	system("resources/data/flowgrid/VecFieldGen.exe -outfile flowgrid_test.fg");

	std::vector<std::string> grids = { "flowgrid_test.fg" };

	m_pFlowVolume = new FlowVolume(grids, true);

	return false;
}
