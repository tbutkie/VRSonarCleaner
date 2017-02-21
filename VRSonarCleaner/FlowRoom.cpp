#include "FlowRoom.h"
#include "DebugDrawer.h"

#include "ParticleManager.h"

FlowRoom::FlowRoom()
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	m_pScaler = new CoordinateScaler();

	m_vec3RoomSize.x = 10;
	m_vec3RoomSize.y = 4;
	m_vec3RoomSize.z = 6;

	m_pHolodeck = new HolodeckBackground(m_vec3RoomSize.x, m_vec3RoomSize.y, m_vec3RoomSize.z, 0.25);

	m_vec3Min.x = 0 - (m_vec3RoomSize.x / 2);
	m_vec3Min.y = 0 - (m_vec3RoomSize.y / 2);
	m_vec3Min.z = 0;

	m_vec3Max.x = (m_vec3RoomSize.x / 2);
	m_vec3Max.y = (m_vec3RoomSize.y / 2);
	m_vec3Max.z = m_vec3RoomSize.z;
	
	FlowGrid *tempFG = new FlowGrid("test.fg");
	tempFG->setCoordinateScaler(m_pScaler);
	m_vpFlowGridCollection.push_back(tempFG);
	m_fFlowRoomMinTime = m_vpFlowGridCollection.at(0)->minTime;
	m_fFlowRoomMaxTime = m_vpFlowGridCollection.at(0)->maxTime;
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_ullLastTimeUpdate = GetTickCount64();

	glm::vec3 pos(0.f, 1.f, 0.f);
	glm::vec3 size(1.f);
	glm::vec3 minCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMin(),
		m_vpFlowGridCollection.at(0)->getScaledMinDepth(),
		m_vpFlowGridCollection.at(0)->getScaledYMin()
	);
	glm::vec3 maxCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMax(), 
		m_vpFlowGridCollection.at(0)->getScaledMaxDepth(), 
		m_vpFlowGridCollection.at(0)->getScaledYMax()
	);

	m_pMainModelVolume = new DataVolume(pos, 0, size, minCoords, maxCoords);
	
	m_pParticleSystem = new IllustrativeParticleSystem(m_pScaler, m_vpFlowGridCollection);

	StreakletSystem::ConstructionInfo ci;
	ci.dataVolume = m_pMainModelVolume;
	ci.flowGrid = m_vpFlowGridCollection.at(0);
	ci.scaler = m_pScaler;

	m_pStreakletSystem = new StreakletSystem(100, glm::vec3(0.f), &ci);

	ParticleManager::getInstance().add(m_pStreakletSystem);
}

FlowRoom::~FlowRoom()
{

}

void FlowRoom::recalcVolumeBounds()
{
	glm::vec3 minCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMin(),
		m_vpFlowGridCollection.at(0)->getScaledMinDepth(),
		m_vpFlowGridCollection.at(0)->getScaledYMin()
	);
	glm::vec3 maxCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMax(),
		m_vpFlowGridCollection.at(0)->getScaledMaxDepth(),
		m_vpFlowGridCollection.at(0)->getScaledYMax()
	);

	m_pMainModelVolume->setInnerCoords(minCoords, maxCoords);
}

void FlowRoom::setRoomSize(float SizeX, float SizeY, float SizeZ)
{
	m_vec3RoomSize.x = SizeX;
	m_vec3RoomSize.y = SizeY;
	m_vec3RoomSize.z = SizeZ;
}

bool FlowRoom::gripModel(const glm::mat4 &controllerPose)
{
	if (!m_pMainModelVolume->isBeingRotated())
	{
		m_pMainModelVolume->startRotation(controllerPose);
		//printf("++ Rotation Started\n");
		return true;
	}
	else
	{
		m_pMainModelVolume->continueRotation(controllerPose);
		//printf("==== Rotating\n");
	}
	return false;
}

void FlowRoom::releaseModel()
{
	if (m_pMainModelVolume->isBeingRotated())
	{
		m_pMainModelVolume->endRotation();
		//printf("|| Rotation Ended\n");
	}
}

void FlowRoom::draw()
{
	//printf("In CleaningRoom Draw()\n");
	m_pHolodeck->drawSolid();

	//draw debug
	m_pMainModelVolume->drawBBox();
	m_pMainModelVolume->drawBacking();
	m_pMainModelVolume->drawAxes();
	
	//draw model
	DebugDrawer::getInstance().setTransform(m_pMainModelVolume->getCurrentDataTransform());
	//m_pParticleSystem->drawDyePots();                
	m_pParticleSystem->drawStreakVBOs();
	//m_pParticleSystem->drawParticleVBOs();
		
}

void FlowRoom::reset()
{
	m_pMainModelVolume->resetPositionAndOrientation();
}

void FlowRoom::receiveEvent(TrackedDevice * device, const int event, void* data)
{
	if (event == BroadcastSystem::EVENT::EDIT_TRIGGER_CLICKED)
	{
		glm::mat4 cursorPose;
		memcpy(&cursorPose, data, sizeof(cursorPose));
		glm::vec3 innerPos = m_pMainModelVolume->convertToInnerCoords(glm::vec3(cursorPose[3]));

		printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

		IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos.x, innerPos.z, innerPos.y - 0.1f, innerPos.y + 0.1f, m_pScaler);
		tmp->setRate(10.f);
		tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
		m_pParticleSystem->m_vpDyePots.push_back(tmp);
		//particleSystem->addDyeParticle(x, z, y, 1.f, 0.f, 0.f, 10.f);
	}

	if (event == BroadcastSystem::EVENT::EDIT_GRIP_PRESSED)
	{
		glm::mat4 cursorPose;
		memcpy(&cursorPose, data, sizeof(cursorPose));
		glm::vec3 innerPos = m_pMainModelVolume->convertToInnerCoords(glm::vec3(cursorPose[3]));

		printf("Deleting Dye Pot Closest to:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);
		
		IllustrativeParticleEmitter *tmp = m_pParticleSystem->getDyePotClosestTo(innerPos.x, innerPos.z, innerPos.y);
		if (tmp)
		{
			m_pParticleSystem->m_vpDyePots.erase(std::remove(m_pParticleSystem->m_vpDyePots.begin(), m_pParticleSystem->m_vpDyePots.end(), tmp), m_pParticleSystem->m_vpDyePots.end());
			delete tmp;
		}		
	}
}

void FlowRoom::preRenderUpdates()
{
	//update time
	ULONGLONG tick = GetTickCount64();
	ULONGLONG timeSinceLast = tick - m_ullLastTimeUpdate;
	if (timeSinceLast > 20)
	{
		m_ullLastTimeUpdate = tick;
		float currentTime = m_fFlowRoomTime;
		float minTime = m_fFlowRoomMinTime;
		float maxTime = m_fFlowRoomMaxTime;
		float timeRange = maxTime - minTime;

		if (timeRange > 0)
		{
			if (true) ///playing v paused
			{
				float factorToAdvance = (float)timeSinceLast / 35000; //35 second loop time		
				float newTime = currentTime + (factorToAdvance * timeRange);
				if (newTime > maxTime)
					newTime = minTime;
				m_fFlowRoomTime = newTime;
			}
		}//end if timerange greater than zero
		else //no time range
		{
			//playing = false;
		}
	}//end if need update

	//std::cout << "Updating particle system with time " << flowRoomTime << std::endl;
	m_pParticleSystem->update(m_fFlowRoomTime);
	m_pStreakletSystem->update(timeSinceLast);
	//std::cout << "Particle System Update Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
	//start = std::clock();
	//m_pParticleSystem->loadStreakVBOs();
	//std::cout << "Particle System Load Streaks Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
	//start = std::clock();
	//m_pParticleSystem->loadParticleVBOs();
	//std::cout << "Particle System Load Particles Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}