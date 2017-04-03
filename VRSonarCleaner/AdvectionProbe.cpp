#include "AdvectionProbe.h"

#include "DebugDrawer.h"

AdvectionProbe::AdvectionProbe(ViveController* controller, FlowVolume* flowVolume)
	: ProbeBehavior(controller, flowVolume)
	, m_pFlowVolume(flowVolume)
{
}


AdvectionProbe::~AdvectionProbe()
{
}

void AdvectionProbe::update()
{
	if (!m_pController->readyToRender())
		return;

	float sphereRad = m_pDataVolume->getDimensions().y * 0.25f;
	DebugDrawer::getInstance().setTransform(glm::translate(glm::mat4(), m_pDataVolume->getPosition()));
	DebugDrawer::getInstance().drawSphere(sphereRad, 3, glm::vec4(0.f, 0.f, 1.f, 0.25f));

	glm::vec3 cursorPos(getPose()[3]);
	glm::vec3 spherePos(m_pDataVolume->getPosition());
	glm::vec3 vecCursorSphere(spherePos - cursorPos);
	float connectorLength = glm::length(vecCursorSphere) - sphereRad;
	glm::vec3 probePos(cursorPos + glm::normalize(vecCursorSphere) * connectorLength);
	DebugDrawer::getInstance().setTransformDefault();
	DebugDrawer::getInstance().drawLine(cursorPos, probePos, glm::vec4(0.f, 0.7f, 0.f, 0.5f));

	// Draw an 'X' at the probe point
	{
		glm::vec3 x = glm::normalize(glm::cross(probePos - spherePos, glm::vec3(glm::mat3_cast(m_pDataVolume->getOrientation())[1])));
		glm::vec3 y = glm::normalize(glm::cross(x, probePos - spherePos));

		float crossSize = sphereRad / 16.f;

		DebugDrawer::getInstance().drawLine(probePos - crossSize * x, probePos + crossSize * x, glm::vec4(1.f, 1.f, 0.f, 0.75f));
		DebugDrawer::getInstance().drawLine(probePos - crossSize * y, probePos + crossSize * y, glm::vec4(1.f, 1.f, 0.f, 0.75f));
	}
}

void AdvectionProbe::activateProbe()
{
}

void AdvectionProbe::deactivateProbe()
{
}



// Taken from http://stackoverflow.com/questions/5883169/intersection-between-a-line-and-a-sphere
glm::vec3 AdvectionProbe::lineSphereIntersection(glm::vec3 linePoint0, glm::vec3 linePoint1, glm::vec3 circleCenter, float circleRadius)
{
	// http://www.codeproject.com/Articles/19799/Simple-Ray-Tracing-in-C-Part-II-Triangles-Intersec

	float cx = circleCenter.x;
	float cy = circleCenter.y;
	float cz = circleCenter.z;

	float px = linePoint0.x;
	float py = linePoint0.y;
	float pz = linePoint0.z;

	float vx = linePoint1.x - px;
	float vy = linePoint1.y - py;
	float vz = linePoint1.z - pz;

	float A = vx * vx + vy * vy + vz * vz;
	float B = 2.f * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
	float C = px * px - 2.f * px * cx + cx * cx + py * py - 2.f * py * cy + cy * cy +
		pz * pz - 2.f * pz * cz + cz * cz - circleRadius * circleRadius;

	// discriminant
	float D = B * B - 4.f * A * C;

	if (D < 0.f)
		return glm::vec3(0.f);

	float t1 = (-B - sqrtf(D)) / (2.f * A);

	glm::vec3 solution1(linePoint0.x * (1.f - t1) + t1 * linePoint1.x,
		linePoint0.y * (1.f - t1) + t1 * linePoint1.y,
		linePoint0.z * (1.f - t1) + t1 * linePoint1.z);

	if (D == 0.f)
		return solution1;

	float t2 = (-B + sqrtf(D)) / (2.f * A);
	glm::vec3 solution2(linePoint0.x * (1.f - t2) + t2 * linePoint1.x,
		linePoint0.y * (1.f - t2) + t2 * linePoint1.y,
		linePoint0.z * (1.f - t2) + t2 * linePoint1.z);

	// prefer a solution that's on the line segment itself

	if (abs(t1 - 0.5f) < abs(t2 - 0.5f))
		return solution1;

	return solution2;
}
