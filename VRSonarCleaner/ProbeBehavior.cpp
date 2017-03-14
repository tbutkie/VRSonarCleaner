#include "ProbeBehavior.h"

#include "shared/glm/gtc/matrix_transform.hpp" // for translate()

#include "DebugDrawer.h"

ProbeBehavior::ProbeBehavior(ViveController* controller, DataVolume* dataVolume)
	: SingleControllerBehavior(controller)
	, m_pDataVolume(dataVolume)
	, c_fTouchDeltaThreshold(0.2f)
	, m_bShowProbe(true)
	, m_bVerticalSwipeMode(false)
	, m_bHorizontalSwipeMode(false)
	, m_vec3ProbeOffsetDirection(glm::vec3(0.f, 0.f, -1.f))
	, m_fProbeOffset(0.1f)
	, m_fProbeOffsetMin(0.1f)
	, m_fProbeOffsetMax(2.f)
	, m_fProbeRadius(0.05f)
	, m_fProbeRadiusMin(0.001f)
	, m_fProbeRadiusMax(0.5f)
	, m_LastTime(std::chrono::high_resolution_clock::now())
	, m_fCursorHoopAngle(0.f)
{
}


ProbeBehavior::~ProbeBehavior()
{
}

glm::mat4 ProbeBehavior::getPose()
{
	return m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

void ProbeBehavior::update()
{
}

void ProbeBehavior::draw()
{
	// Draw cursor hoop
	if (m_bShowProbe)
	{
		GLuint num_segments = 64;

		glm::vec4 color;
		if (m_pController->isTriggerClicked())
			color = glm::vec4(1.f, 0.f, 0.f, 1.f);
		else
			color = glm::vec4(1.f, 1.f, 1.f - m_pController->getTriggerPullAmount(), 0.75f);

		std::vector<glm::vec3> circlePoints;
		for (GLuint i = 0; i < num_segments; i++)
		{
			GLfloat theta = glm::two_pi<float>() * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			glm::vec3 circlePt;
			circlePt.x = cosf(theta);
			circlePt.y = sinf(theta);
			circlePt.z = 0.f;

			circlePoints.push_back(circlePt);
		}

		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
		m_LastTime = std::chrono::high_resolution_clock::now();

		long long rate_ms_per_rev = 2000ll / (1.f + 10.f * m_pController->getTriggerPullAmount());

		glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(m_fProbeRadius));
		glm::mat4 rot;

		float angleNeeded = 360.f * (elapsed_ms.count() % rate_ms_per_rev) / rate_ms_per_rev;
		m_fCursorHoopAngle += angleNeeded;

		for (int n = 0; n < 3; ++n)
		{
			if (n == 0)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(1.f, 0.f, 0.f));
			if (n == 1)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
			if (n == 2)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 0.f, 1.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

			DebugDrawer::getInstance().setTransform(getPose() * rot * scl);

			glm::vec3 prevVert = circlePoints.back();
			for (size_t i = 0; i < circlePoints.size(); ++i)
			{
				glm::vec3 thisVert = circlePoints[i];

				DebugDrawer::getInstance().drawLine(prevVert, thisVert, color);

				prevVert = thisVert;
			}
		}

		// DISPLAY CURSOR DOT
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		//{
		//	color = Vector3(1.f, 0.f, 0.f);
		//	if (cursorActive())
		//	{
		//		Vector4 thisCtrPos = m_mat4CursorCurrentPose * Vector4(0.f, 0.f, 0.f, 1.f);
		//		Vector4 lastCtrPos = m_mat4CursorLastPose * Vector4(0.f, 0.f, 0.f, 1.f);

		//		vertdataarray.push_back(lastCtrPos.x);
		//		vertdataarray.push_back(lastCtrPos.y);
		//		vertdataarray.push_back(lastCtrPos.z);
		//		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		//		vertdataarray.push_back(thisCtrPos.x);
		//		vertdataarray.push_back(thisCtrPos.y);
		//		vertdataarray.push_back(thisCtrPos.z);
		//		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		//		m_uiLineVertcount += 2;
		//	}
		//}

		// DISPLAY CONNECTING LINE TO CURSOR
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		{
			color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
			if (m_bShowProbe)
			{
				glm::vec3 controllerCtr = glm::vec3(m_pController->getDeviceToWorldTransform() * glm::vec4(0.f, 0.f, 0.f, 1.f));
				glm::vec3 cursorEdge = glm::vec3(getPose() * glm::vec4(0.f, 0.f, m_fProbeRadius, 1.f));

				DebugDrawer::getInstance().setTransformDefault();
				DebugDrawer::getInstance().drawLine(controllerCtr, cursorEdge, color);
			}
		}
	}
}

void ProbeBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_ENGAGE:
	{
		BroadcastSystem::Payload::Touchpad* payload;
		memcpy(&payload, &payloadData, sizeof(payload));
		
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_TOUCH:
	{
		BroadcastSystem::Payload::Touchpad* payload;
		memcpy(&payload, &payloadData, sizeof(payload));
		
		glm::vec2 delta = payload->m_vec2CurrentTouch - payload->m_vec2InitialTouch;

		if (!(m_bVerticalSwipeMode || m_bHorizontalSwipeMode) &&
			glm::length(delta) > c_fTouchDeltaThreshold)
		{
			m_vec2InitialMeasurementPoint = payload->m_vec2CurrentTouch;

			if (abs(delta.x) > abs(delta.y))
			{
				m_bHorizontalSwipeMode = true;
				m_fProbeRadiusInitial = m_fProbeRadius;
			}
			else
			{
				m_bVerticalSwipeMode = true;
				m_pController->showScrollWheel();
				m_fProbeInitialOffset = m_fProbeOffset;
			}
		}

		assert(!(m_bVerticalSwipeMode && m_bHorizontalSwipeMode));

		glm::vec2 measuredOffset = payload->m_vec2CurrentTouch - m_vec2InitialMeasurementPoint;

		if (m_bVerticalSwipeMode)
		{
			m_fProbeOffset = measuredOffset.y;

			float dy = measuredOffset.y;

			float range = m_fProbeOffsetMax - m_fProbeOffsetMin;

			m_fProbeOffset = m_fProbeInitialOffset + dy * range * 0.5f;
			
			if (m_fProbeOffset > m_fProbeOffsetMax)
			{
				m_fProbeOffset = m_fProbeOffsetMax;
				m_fProbeInitialOffset = m_fProbeOffsetMax;
				m_vec2InitialMeasurementPoint.y = payload->m_vec2CurrentTouch.y;
			}
			else if (m_fProbeOffset < m_fProbeOffsetMin)
			{
				m_fProbeOffset = m_fProbeOffsetMin;
				m_fProbeInitialOffset = m_fProbeOffsetMin;
				m_vec2InitialMeasurementPoint.y = payload->m_vec2CurrentTouch.y;
			}
		}

		if (m_bHorizontalSwipeMode)
		{
			float dx = payload->m_vec2CurrentTouch.x - m_vec2InitialMeasurementPoint.x;

			float range = m_fProbeRadiusMax - m_fProbeRadiusMin;

			m_fProbeRadius = m_fProbeRadiusInitial + dx * range;

			if (m_fProbeRadius > m_fProbeRadiusMax)
			{
				m_fProbeRadius = m_fProbeRadiusMax;
				m_fProbeRadiusInitial = m_fProbeRadiusMax;
				m_vec2InitialMeasurementPoint.x = payload->m_vec2CurrentTouch.x;
			}
			else if (m_fProbeRadius < m_fProbeRadiusMin)
			{ 
				m_fProbeRadius = m_fProbeRadiusMin;
				m_fProbeRadiusInitial = m_fProbeRadiusMin;
				m_vec2InitialMeasurementPoint.x = payload->m_vec2CurrentTouch.x;
			}
		}

		break;
	}
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_DISENGAGE:
	{
		m_bVerticalSwipeMode = m_bHorizontalSwipeMode = false;
		m_pController->hideScrollWheel();
	}
	default:
		break;
	}
}

void ProbeBehavior::activateProbe()
{
}
