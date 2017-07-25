#include "ProbeBehavior.h"

#include "shared/glm/gtc/matrix_transform.hpp" // for translate()

#include "Renderer.h"

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
	generateCylinder(16);

	GLubyte gray[4] = { 0x20, 0x20, 0x20, 0xFF };
	GLubyte white[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

	glGenTextures(1, &m_glProbeDiffTex);
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glProbeDiffTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &gray);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &m_glProbeSpecTex);
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glProbeSpecTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &white);

	glBindTexture(GL_TEXTURE_2D, 0);
}


ProbeBehavior::~ProbeBehavior()
{
}

glm::vec3 ProbeBehavior::getPosition()
{
	return glm::vec3((m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::vec3 ProbeBehavior::getLastPosition()
{
	return glm::vec3((m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::mat4 ProbeBehavior::getProbeToWorldTransform()
{
	return m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

glm::mat4 ProbeBehavior::getLastProbeToWorldTransform()
{
	return m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

void ProbeBehavior::update()
{
}

void ProbeBehavior::drawProbe(float length)
{
	if (!m_pController->readyToRender())
		return;

	if (m_bShowProbe)
	{	
		// Set color
		glm::vec4 color;
		if (m_pController->isTriggerClicked())
			color = glm::vec4(1.f, 0.f, 0.f, 1.f);
		else
			color = glm::vec4(1.f, 1.f, 1.f - m_pController->getTriggerPullAmount(), 0.75f);
		
		Renderer::RendererSubmission rsProbe;

		rsProbe.primitiveType = GL_TRIANGLES;
		rsProbe.shaderName = "lighting";
		rsProbe.VAO = m_glProbeVAO;
		rsProbe.diffuseTex = m_glProbeDiffTex;
		rsProbe.specularTex = m_glProbeSpecTex;
		rsProbe.specularExponent = 30.f;
		rsProbe.vertCount = m_nProbeVertices;
		rsProbe.modelToWorldTransform = m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(0.0025f, 0.0025f, length));
		rsProbe.indexType = GL_UNSIGNED_SHORT;

		Renderer::getInstance().addToDynamicRenderQueue(rsProbe);
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
				m_pController->setScrollWheelVisibility(true);
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
		m_pController->setScrollWheelVisibility(false);
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		activateProbe();
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		deactivateProbe();
		break;
	}
	default:
		break;
	}
}

void ProbeBehavior::generateCylinder(int numSegments)
{
	std::vector<glm::vec3> pts;
	std::vector<glm::vec3> norms;
	std::vector<glm::vec2> texUVs;
	std::vector<unsigned short> inds;

	// Front endcap
	pts.push_back(glm::vec3(0.f));
	norms.push_back(glm::vec3(0.f, 0.f, -1.f));
	texUVs.push_back(glm::vec2(0.5f, 0.5f));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		norms.push_back(glm::vec3(0.f, 0.f, -1.f));
		texUVs.push_back((glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f);

		if (i > 0)
		{
			inds.push_back(0);
			inds.push_back(pts.size() - 2);
			inds.push_back(pts.size() - 1);
		}
	}
	inds.push_back(0);
	inds.push_back(pts.size() - 1);
	inds.push_back(1);

	// Back endcap
	pts.push_back(glm::vec3(0.f, 0.f, 1.f));
	norms.push_back(glm::vec3(0.f, 0.f, 1.f));
	texUVs.push_back(glm::vec2(0.5f, 0.5f));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 1.f));
		norms.push_back(glm::vec3(0.f, 0.f, 1.f));
		texUVs.push_back((glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f);

		if (i > 0)
		{
			inds.push_back(pts.size() - (i + 2)); // ctr pt of endcap
			inds.push_back(pts.size() - 1);
			inds.push_back(pts.size() - 2);
		}
	}
	inds.push_back(pts.size() - (numSegments + 1));
	inds.push_back(pts.size() - (numSegments));
	inds.push_back(pts.size() - 1);

	// Shaft
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		pts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		norms.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		texUVs.push_back(glm::vec2((float)i / (float)(numSegments - 1), 0.f));

		pts.push_back(glm::vec3(sin(angle), cos(angle), 1.f));
		norms.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
		texUVs.push_back(glm::vec2((float)i / (float)(numSegments - 1), 1.f));

		if (i > 0)
		{
			inds.push_back(pts.size() - 4);
			inds.push_back(pts.size() - 3);
			inds.push_back(pts.size() - 2);

			inds.push_back(pts.size() - 2);
			inds.push_back(pts.size() - 3);
			inds.push_back(pts.size() - 1);
		}
	}
	inds.push_back(pts.size() - 2);
	inds.push_back(pts.size() - numSegments * 2);
	inds.push_back(pts.size() - 1);

	inds.push_back(pts.size() - numSegments * 2);
	inds.push_back(pts.size() - numSegments * 2 + 1);
	inds.push_back(pts.size() - 1);

	m_nProbeVertices = inds.size();

	glGenVertexArrays(1, &m_glProbeVAO);
	glGenBuffers(1, &m_glProbeVBO);
	glGenBuffers(1, &m_glProbeEBO);

	// Setup VAO
	glBindVertexArray(this->m_glProbeVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glProbeVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glProbeEBO);

	// Set the vertex attribute pointers
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
	glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(pts.size() * sizeof(glm::vec3)));
	glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)(pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3)));
	glBindVertexArray(0);

	// Fill buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glProbeVBO);
	// Buffer orphaning
	glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3) + texUVs.size() * sizeof(glm::vec2), 0, GL_STREAM_DRAW);
	// Sub buffer data for points, normals, textures...
	glBufferSubData(GL_ARRAY_BUFFER, 0, pts.size() * sizeof(glm::vec3), &pts[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3), norms.size() * sizeof(glm::vec3), &norms[0]);
	glBufferSubData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3) + norms.size() * sizeof(glm::vec3), texUVs.size() * sizeof(glm::vec2), &texUVs[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glProbeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), 0, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned short), &inds[0], GL_STREAM_DRAW);
}
