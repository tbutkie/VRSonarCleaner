#include "FlowScene.h"

using namespace std::chrono_literals;

FlowScene::FlowScene(TrackedDeviceManager* pTDM)
	: m_pFlowVolume(NULL)
{
}


FlowScene::~FlowScene()
{
	if (m_pFlowVolume)
		delete m_pFlowVolume;
}

void FlowScene::init()
{
}

void FlowScene::update()
{
	if (m_pTDM->getPrimaryController() && m_pTDM->getPrimaryController()->justPressedTouchpad())
		loadRandomFlowGrid();
}

void FlowScene::draw()
{

}

bool FlowScene::loadRandomFlowGrid()
{
	if (m_pFlowVolume)
		delete m_pFlowVolume;

	system("resources/data/flowgrid/VecFieldGen.exe -outfile flowgrid_test.fg");

	std::vector<std::string> grids = { "flowgrid_test.fg" };

	m_pFlowVolume = new FlowVolume(grids, true);

	return false;
}
