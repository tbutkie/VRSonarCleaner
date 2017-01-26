#include "FlowRoom.h"
#include "Quaternion.h"
#include "DebugDrawer.h"

FlowRoom::FlowRoom()
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	scaler = new CoordinateScaler();

	roomSizeX = 10;
	roomSizeY = 4;
	roomSizeZ = 6;

	holodeck = new HolodeckBackground(roomSizeX, roomSizeY, roomSizeZ, 0.25);

	minX = 0 - (roomSizeX / 2);
	minY = 0 - (roomSizeY / 2);
	minZ = 0;

	maxX = (roomSizeX / 2);
	maxY = (roomSizeY / 2);
	maxZ = roomSizeZ;

	flowGridCollection = new std::vector <FlowGrid*>;

	FlowGrid *tempFG = new FlowGrid("tidalwalls.fg");
	tempFG->setCoordinateScaler(scaler);
	flowGridCollection->push_back(tempFG);
	flowRoomMinTime = flowGridCollection->at(0)->minTime;
	flowRoomMaxTime = flowGridCollection->at(0)->maxTime;
	flowRoomTime = flowRoomMinTime;
	lastTimeUpdate = GetTickCount64();
	
	particleSystem = new IllustrativeParticleSystem(scaler, flowGridCollection);


	//tableVolume = new DataVolume(0, 0.25, 0, 0, 1.25, 0.4, 1.25);
	mainModelVolume = new DataVolume(0, 1.10, 0, 0, 2.25, 0.75, 2.25);
	//	wallVolume = new DataVolume(0, (roomSizeY / 2) + (roomSizeY*0.09), (roomSizeZ / 2) - 0.42, 1, (roomSizeX*0.9), 0.8, (roomSizeY*0.80));

	mainModelVolume->setInnerCoords(flowGridCollection->at(0)->getScaledXMin(), flowGridCollection->at(0)->getScaledXMax(), flowGridCollection->at(0)->getScaledMaxDepth(), flowGridCollection->at(0)->getScaledMinDepth(), flowGridCollection->at(0)->getScaledYMin(), flowGridCollection->at(0)->getScaledYMax());
	
	m_fPtHighlightAmt = 1.f;
	m_LastTime = std::chrono::high_resolution_clock::now();
}

FlowRoom::~FlowRoom()
{

}

void FlowRoom::recalcVolumeBounds()
{
	mainModelVolume->setInnerCoords(flowGridCollection->at(0)->getScaledXMin(), flowGridCollection->at(0)->getScaledXMax(), flowGridCollection->at(0)->getScaledMaxDepth(), flowGridCollection->at(0)->getScaledMinDepth(), flowGridCollection->at(0)->getScaledYMin(), flowGridCollection->at(0)->getScaledYMax());
}

void FlowRoom::setRoomSize(float SizeX, float SizeY, float SizeZ)
{
	roomSizeX = SizeX;
	roomSizeY = SizeY;
	roomSizeZ = SizeZ;
}



bool FlowRoom::gripModel(const Matrix4 *controllerPose)
{
	if (!controllerPose)
	{
		if (mainModelVolume->isBeingRotated())
		{
			mainModelVolume->endRotation();
			//printf("|| Rotation Ended\n");
		}

		return false;
	}

	if (!mainModelVolume->isBeingRotated())
	{
		mainModelVolume->startRotation(controllerPose->get());
		//printf("++ Rotation Started\n");
		return true;
	}
	else
	{
		mainModelVolume->continueRotation(controllerPose->get());
		//printf("==== Rotating\n");
	}
	return false;
}


void FlowRoom::draw()
{
	//printf("In CleaningRoom Draw()\n");
	holodeck->drawSolid();

	//draw debug
	mainModelVolume->drawBBox();
	mainModelVolume->drawBacking();
	mainModelVolume->drawAxes();
	
	//draw model
	glPushMatrix();
	mainModelVolume->activateTransformationMatrix();
	flowGridCollection->at(0)->drawBBox();
	particleSystem->drawStreakVBOs();
	//particleSystem->drawParticleVBOs();
	glPopMatrix();
		
}

void FlowRoom::reset()
{
	mainModelVolume->resetPositionAndOrientation();
}

void FlowRoom::preRenderUpdates()
{
	//update time
	ULONGLONG tick = GetTickCount64();
	ULONGLONG timeSinceLast = tick - lastTimeUpdate;
	if (timeSinceLast > 20)
	{
		lastTimeUpdate = tick;
		float currentTime = flowRoomTime;
		float minTime = flowRoomMinTime;
		float maxTime = flowRoomMaxTime;
		float timeRange = maxTime - minTime;

		if (timeRange > 0)
		{
			if (true) ///playing v paused
			{
				float factorToAdvance = (float)timeSinceLast / 35000; //35 second loop time		
				float newTime = currentTime + (factorToAdvance * timeRange);
				if (newTime > maxTime)
					newTime = minTime;
				flowRoomTime = newTime;
			}
		}//end if timerange greater than zero
		else //no time range
		{
			//playing = false;
		}
	}//end if need update


	particleSystem->update(flowRoomTime);
	particleSystem->loadStreakVBOs();
	particleSystem->loadParticleVBOs();
}