#include "TrackedDevice.h"



TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id) :
	id(id)
{	
	m_rDevClassChar = 0;
}


TrackedDevice::~TrackedDevice()
{
}

bool TrackedDevice::BInit()
{
	return false;
}
