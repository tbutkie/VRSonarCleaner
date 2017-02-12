#include "FlowRoom.h"
#include "DebugDrawer.h"

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
	
	m_pParticleSystem = new IllustrativeParticleSystem(m_pScaler, m_vpFlowGridCollection);


	//tableVolume = new DataVolume(0, 0.25, 0, 0, 1.25, 0.4, 1.25);
	m_pMainModelVolume = new DataVolume(0.f, 1.f, 0.f, 0, 1.f, 1.f, 1.f);
	//	wallVolume = new DataVolume(0, (roomSizeY / 2) + (roomSizeY*0.09), (roomSizeZ / 2) - 0.42, 1, (roomSizeX*0.9), 0.8, (roomSizeY*0.80));

	m_pMainModelVolume->setInnerCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMin(), 
		m_vpFlowGridCollection.at(0)->getScaledXMax(), 
		m_vpFlowGridCollection.at(0)->getScaledMinDepth(), 
		m_vpFlowGridCollection.at(0)->getScaledMaxDepth(), 
		m_vpFlowGridCollection.at(0)->getScaledYMin(), 
		m_vpFlowGridCollection.at(0)->getScaledYMax()
	);
}

FlowRoom::~FlowRoom()
{

}

void FlowRoom::recalcVolumeBounds()
{
	m_pMainModelVolume->setInnerCoords(
		m_vpFlowGridCollection.at(0)->getScaledXMin(),
		m_vpFlowGridCollection.at(0)->getScaledXMax(),
		m_vpFlowGridCollection.at(0)->getScaledMinDepth(),
		m_vpFlowGridCollection.at(0)->getScaledMaxDepth(),
		m_vpFlowGridCollection.at(0)->getScaledYMin(),
		m_vpFlowGridCollection.at(0)->getScaledYMax()
	);
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
	glPushMatrix();
	m_pMainModelVolume->activateTransformationMatrix();
	//m_vpFlowGridCollection.at(0)->drawBBox();
	m_pParticleSystem->drawStreakVBOs();
	//m_pParticleSystem->drawParticleVBOs();
	glPopMatrix();
		
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
		glm::vec4 cursorPos = cursorPose * glm::vec4(0.f, 0.f, 0.f, 1.f);
		float x, y, z;
		m_pMainModelVolume->convertToInnerCoords(cursorPos.x, cursorPos.y, cursorPos.z, &x, &y, &z);
		printf("Dye In:  %0.4f, %0.4f, %0.4f\n", x, y, z);
		//int dyePoleID = particleSystem->addDyePole(x, z, y - 0.1f, y + 0.1f);
		//particleSystem->dyePoles[dyePoleID]->emitters[0]->setRate(particleSystem->dyePoles[dyePoleID]->emitters[0]->getRate() * 0.1f);

		IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(x, z, y - 0.1f, y + 0.1f, m_pScaler);
		tmp->setRate(10.f);
		tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
		m_pParticleSystem->m_vpDyePots.push_back(tmp);
		//particleSystem->addDyeParticle(x, z, y, 1.f, 0.f, 0.f, 10.f);
	}

	if (event == BroadcastSystem::EVENT::EDIT_GRIP_PRESSED)
	{
		glm::mat4 cursorPose;
		memcpy(&cursorPose, data, sizeof(cursorPose));
		glm::vec4 cursorPos = cursorPose * glm::vec4(0.f, 0.f, 0.f, 1.f);
		float x, y, z;
		m_pMainModelVolume->convertToInnerCoords(cursorPos.x, cursorPos.y, cursorPos.z, &x, &y, &z);

		printf("Deleting Dye Pot Closest to:  %0.4f, %0.4f, %0.4f\n", x, y, z);
		
		IllustrativeParticleEmitter *tmp = m_pParticleSystem->getDyePotClosestTo(x, z, y);
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
	//std::cout << "Particle System Update Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
	//start = std::clock();
	m_pParticleSystem->loadStreakVBOs();
	//std::cout << "Particle System Load Streaks Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
	//start = std::clock();
	m_pParticleSystem->loadParticleVBOs();
	//std::cout << "Particle System Load Particles Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}