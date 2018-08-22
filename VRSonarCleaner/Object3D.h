#pragma once

#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

#include <string>

class Object3D
{
public:
	Object3D(glm::vec3 pos, glm::quat orientation, glm::vec3 dimensions);
	virtual ~Object3D();

	void setPosition(glm::vec3 newPos);
	glm::vec3 getPosition();
	glm::vec3 getOriginalPosition();
	void setOrientation(glm::quat newOrientation);
	glm::quat getOrientation();
	glm::quat getOriginalOrientation();
	void setDimensions(glm::vec3 newScale);
	glm::vec3 getDimensions();
	glm::vec3 getOriginalDimensions();

	float getBoundingRadius();

	void resetPositionAndOrientation();

protected:
	void calculateBoundingRadius();

	glm::vec3 m_vec3OriginalPosition;        // Original Data Volume Position	
	glm::vec3 m_vec3Position;
	glm::quat m_qOriginalOrientation;        // Original Data Volume Orientation
	glm::quat m_qOrientation;
	glm::vec3 m_vec3OriginalDimensions;      // Original Data Volume Orientation
	glm::vec3 m_vec3Dimensions;
	float m_fBoundingRadius;

	bool m_bDirty;                           // a user flag to tell whether or not the transform has changed

private:

};

