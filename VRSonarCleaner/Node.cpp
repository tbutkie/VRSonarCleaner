#include "Node.h"

#include "DebugDrawer.h"

Node::Node(glm::vec3 initialPosition, glm::quat initialOrientation, glm::vec3 initialScale)
	: m_vec3Position(initialPosition)
	, m_qOrientation(initialOrientation)
	, m_vec3Scale(initialScale)
	, m_bDirty(true)
{
}


Node::~Node()
{
}

void Node::setPosition(glm::vec3 newPos)
{
	m_vec3Position = newPos;
	m_bDirty = true;
}

glm::vec3 Node::getPosition()
{
	return m_vec3Position;
}

void Node::setOrientation(glm::quat newOrientation)
{
	m_qOrientation = newOrientation;
	m_bDirty = true;
}

glm::quat Node::getOrientation()
{
	return m_qOrientation;
}

void Node::setScale(glm::vec3 newScale)
{
	m_vec3Scale = newScale;
	m_bDirty = true;
}

void Node::setScale(float newUniformScale)
{
	m_vec3Scale = glm::vec3(newUniformScale);
	m_bDirty = true;
}

void Node::setScaleX(float newXScale)
{
	m_vec3Scale.x = newXScale;
	m_bDirty = true;
}

void Node::setScaleY(float newYScale)
{
	m_vec3Scale.y = newYScale;
	m_bDirty = true;
}

void Node::setScaleZ(float newZScale)
{
	m_vec3Scale.z = newZScale;
	m_bDirty = true;
}

glm::vec3 Node::getScale()
{
	return m_vec3Scale;
}

float Node::getScaleX()
{
	return m_vec3Scale.x;
}

float Node::getScaleY()
{
	return m_vec3Scale.y;
}

float Node::getScaleZ()
{
	return m_vec3Scale.z;
}

glm::mat4 Node::getModelToWorldTransform()
{
	if (m_bDirty)
	{
		// M = T * R * S
		m_mat4ModelToWorld = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(glm::mat4(), m_vec3Scale);
		m_bDirty = false;
	}

	return m_mat4ModelToWorld;
}

void Node::drawAxes()
{
	DebugDrawer::getInstance().setTransform(getModelToWorldTransform());
	DebugDrawer::getInstance().drawTransform(0.1f); // 10 cm at scale = unity
}

bool Node::isDirty()
{
	return m_bDirty;
}
