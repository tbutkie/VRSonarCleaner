#include "FlowFieldCurator.h"

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
	}

	return false;
}
