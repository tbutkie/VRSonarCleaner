#pragma once

#include <GL/glew.h>

#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/quaternion.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>

class Node
{
public:
	Node(glm::vec3 initialPosition = glm::vec3(0.f), glm::quat initialOrientation = glm::quat(), glm::vec3 initialScale = glm::vec3(1.f));
	virtual ~Node();

	void setPosition(glm::vec3 newPos);
	glm::vec3 getPosition();
	void setOrientation(glm::quat newOrientation);
	glm::quat getOrientation();
	void setScale(glm::vec3 newScale);
	void setScale(float newUniformScale);
	void setScaleX(float newXScale);
	void setScaleY(float newYScale);
	void setScaleZ(float newZScale);
	glm::vec3 getScale();
	float getScaleX();
	float getScaleY();
	float getScaleZ();

	glm::mat4 getModelToWorldTransform();

	void drawAxes(float size = 1.f);

	bool isDirty();

private:
	glm::vec3 m_vec3Position;
	glm::quat m_qOrientation;
	glm::vec3 m_vec3Scale;

	glm::mat4 m_mat4ModelToWorld;

	bool m_bDirty; // a user flag to tell whether or not the node transform has changed

};

