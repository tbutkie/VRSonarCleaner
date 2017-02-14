#ifndef ARCBALL_H
#define ARCBALL_H

/* Arcball, written by Bradley Smith, March 24, 2006
 * arcball.h is free to use and modify for any purpose, with no
 * restrictions of copyright or license.
 *
 * Using the arcball:
 *   Call arcball_setzoom after setting up the projection matrix.
 *
 *     The arcball, by default, will act as if a sphere with the given
 *     radius, centred on the origin, can be directly manipulated with
 *     the mouse. Clicking on a point should drag that point to rest under
 *     the current mouse position. eye is the position of the eye relative
 *     to the origin. up is unused.
 *
 *     Alternatively, pass the value: (-radius/|eye|)
 *     This puts the arcball in a mode where the distance the mouse moves
 *     is equivalent to rotation along the axes. This acts much like a
 *     trackball. (It is for this mode that the up vector is required,
 *     which must be a unit vector.)
 *
 *     You should call arcball_setzoom after use of gluLookAt.
 *     gluLookAt(eye.x,eye.y,eye.z, ?,?,?, up.x,up.y,up.z);
 *     The arcball derives its transformation information from the
 *     openGL projection and viewport matrices. (modelview is ignored)
 *
 *     If looking at a point different from the origin, the arcball will still
 *     act as if it centred at (0,0,0). (You can use this to translate
 *     the arcball to some other part of the screen.)
 *
 *   Call arcball_start with a mouse position, and the arcball will
 *     be ready to manipulate. (Call on mouse button down.)
 *   Call arcball_move with a mouse position, and the arcball will
 *     find the rotation necessary to move the start mouse position to
 *     the current mouse position on the sphere. (Call on mouse move.)
 *   Call arcball_rotate after resetting the modelview matrix in your
 *     drawing code. It will call glRotate with its current rotation.
 *   Call arcball_reset if you wish to reset the arcball rotation.
 */

#include <GL/glew.h>

#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/quaternion.hpp>

class Arcball
{
public:
	Arcball(bool usePlanar = false);
	~Arcball();

	void setZoom(float radius, glm::vec3 eye, glm::vec3 up);
	void start(int mx, int my);
	void move(int mx, int my);
	glm::mat4 getRotation();
	void setViewport(glm::vec4 vp);
	void setProjectionMatrix(glm::mat4 projMat);

private:
	glm::quat m_quatOrientation, m_quatLast, m_quatNext;

	// the distance from the origin to the eye
	GLfloat m_fZoom, m_fZoom_sq;
	// the radius of the arcball
	GLfloat m_fSphereRadius, m_fSphereRadius_sq;
	// the distance from the origin of the plane that intersects
	// the edge of the visible sphere (tangent to a ray from the eye)
	GLfloat m_fEdgeDistance;
	// whether we are using a sphere or plane
	bool m_bPlanar;
	GLfloat m_fPlaneDistance;

	glm::vec3 m_vec3Start;
	glm::vec3 m_vec3Current;
	glm::vec3 m_vec3Eye;
	glm::vec3 m_vec3Forward;
	glm::vec3 m_vec3Up;
	glm::vec3 m_vec3Out;

	glm::mat4 m_mat4Projection, m_mat4Model;
	glm::vec4 m_vec4Viewport;

private:
	glm::vec3 edge_coords(glm::vec3 m);
	glm::vec3 sphere_coords(GLdouble mx, GLdouble my);
	glm::vec3 planar_coords(GLdouble mx, GLdouble my);
};

#endif
