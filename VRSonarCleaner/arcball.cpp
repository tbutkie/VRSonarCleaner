/* Arcball, written by Bradley Smith, March 24, 2006
 * arcball.cpp is free to use and modify for any purpose, with no
 * restrictions of copyright or license.
 *
 * See arcball.h for usage details.
 */


#include "arcball.h"

#include <shared/glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <shared/glm/gtc/matrix_transform.hpp> // glm::unproject

Arcball::Arcball(bool usePlanar)
	: m_fZoom(1.f)
	, m_fZoom_sq(1.f)
	, m_fSphereRadius(1.f)
	, m_fSphereRadius_sq(1.f)
	, m_fEdgeDistance(1.f)
	, m_bPlanar(usePlanar)
	, m_fPlaneDistance(0.5f)	
	, m_vec3Start(glm::vec3(0.f, 0.f, 1.f))
	, m_vec3Current(glm::vec3(0.f, 0.f, 1.f))
	, m_vec3Eye(glm::vec3(0.f, 0.f, 1.f))
	, m_vec3Forward(glm::vec3(0.f, 0.f, 1.f))
	, m_vec3Up(glm::vec3(0.f, 1.f, 0.f))
	, m_vec3Out(glm::vec3(1.f, 0.f, 0.f))
	, m_vec4Viewport(glm::vec4(0.f, 0.f, 640.f, 480.f))
{
}

Arcball::~Arcball()
{
}

void Arcball::setZoom(float radius, glm::vec3 eye, glm::vec3 up)
{
	m_vec3Eye = eye; // store eye vector
	m_fZoom_sq = glm::dot(m_vec3Eye, m_vec3Eye);
	m_fZoom = sqrt(m_fZoom_sq); // store eye distance
	m_fSphereRadius = radius; // sphere radius
	m_fSphereRadius_sq = m_fSphereRadius * m_fSphereRadius;
	m_vec3Forward = m_vec3Eye * (1.f / m_fZoom); // distance to eye
	m_fEdgeDistance = m_fSphereRadius_sq / m_fZoom; // plane of visible edge
  
	if(m_fSphereRadius <= 0.f) // trackball mode
	{
		m_bPlanar = true;
		m_vec3Up = up;
		m_vec3Out = glm::cross(m_vec3Forward, m_vec3Up);
		m_fPlaneDistance = (0.f - m_fSphereRadius) * m_fZoom;
	} else
		m_bPlanar = false;
}

// affect the arcball's orientation on openGL
void Arcball::rotate()
{ 
	glMultMatrixf(glm::value_ptr(glm::mat4_cast(m_quatOrientation))); 
}

// find the intersection with the plane through the visible edge
glm::vec3 Arcball::edge_coords(glm::vec3 m)
{
	// find the intersection of the edge plane and the ray
	float t = (m_fEdgeDistance - m_fZoom) / glm::dot(m_vec3Forward, m);
	glm::vec3 a = m_vec3Eye + (m*t);

	// find the direction of the eye-axis from that point
	// along the edge plane
	glm::vec3 c = (m_vec3Forward * m_fEdgeDistance) - a;

	// find the intersection of the sphere with the ray going from
	// the plane outside the sphere toward the eye-axis.
	float ac = glm::dot(a, c);
	float c2 = glm::dot(c, c);
	float q = (0.f - ac - sqrt( glm::dot(ac, ac) - c2*(glm::dot(a, a)- m_fSphereRadius_sq) ) ) / c2;
  
	return glm::normalize(a+(c*q));
}

// find the intersection with the sphere
glm::vec3 Arcball::sphere_coords(GLdouble mx, GLdouble my)
{
	glm::vec3 window(mx, my, 0.f);

	glm::vec3 coords_unProj = glm::unProject(window, m_mat4Model, m_mat4Projection, m_vec4Viewport);
	glm::vec3 m = coords_unProj - m_vec3Eye;
  
	// mouse position represents ray: eye + t*m
	// intersecting with a sphere centered at the origin
	GLfloat a = glm::dot(m, m);
	GLfloat b = glm::dot(m_vec3Eye, m);
	GLfloat root = (b*b) - a*(m_fZoom_sq - m_fSphereRadius_sq);
	if(root <= 0) return edge_coords(m);
	GLfloat t = (0.f - b - sqrt(root)) / a;
	return glm::normalize(m_vec3Eye +(m*t));
}

// get intersection with plane for "trackball" style rotation
glm::vec3 Arcball::planar_coords(GLdouble mx, GLdouble my)
{
	glm::vec3 window(mx, my, 0.f);
	
	glm::vec3 coords_unProj = glm::unProject(window, m_mat4Model, m_mat4Projection, m_vec4Viewport);
	glm::vec3 m = coords_unProj - m_vec3Eye;
	// intersect the point with the trackball plane
	GLfloat t = (m_fPlaneDistance - m_fZoom) / glm::dot(m_vec3Forward, m);
	glm::vec3 d = m_vec3Eye + m*t;

	return glm::vec3(glm::dot(d, m_vec3Up), glm::dot(d, m_vec3Out),0.f);
}

// begin arcball rotation
void Arcball::start(int mx, int my)
{
	// saves a copy of the current rotation for comparison
	m_quatLast = m_quatOrientation;
	if(m_bPlanar) m_vec3Start = planar_coords((GLdouble)mx,(GLdouble)my);
	else m_vec3Start = sphere_coords((GLdouble)mx,(GLdouble)my);
}

// update current arcball rotation
void Arcball::move(int mx, int my)
{
	if(m_bPlanar)
	{
		m_vec3Current = planar_coords((GLdouble)mx,(GLdouble)my);
		if(m_vec3Current == m_vec3Start) return;
    
		// d is motion since the last position
		glm::vec3 d = m_vec3Current - m_vec3Start;
    
		GLfloat angle = d.length() * 0.5f;
		GLfloat cosa = cos( angle );
		GLfloat sina = sin( angle );
		// p is perpendicular to d
		glm::vec3 p = glm::normalize((m_vec3Out*d.x)-(m_vec3Up*d.y)) * sina;

		m_quatNext = glm::quat(cosa, p.x, p.y, p.z);
		m_quatOrientation = m_quatNext * m_quatLast;
		// planar style only ever relates to the last point
		m_quatLast = m_quatOrientation;
		m_vec3Start = m_vec3Current;
    
	} else {

		m_vec3Current = sphere_coords((GLdouble)mx,(GLdouble)my);
		if(m_vec3Current == m_vec3Start)
		{ // avoid potential rare divide by tiny
			m_quatOrientation = m_quatLast;
			return;
		}

		// use a dot product to get the angle between them
		// use a cross product to get the vector to rotate around
		GLfloat cos2a = glm::dot(m_vec3Start, m_vec3Current);
		GLfloat sina = sqrt((1.f - cos2a)*0.5f);
		GLfloat cosa = sqrt((1.f + cos2a)*0.5f);
		glm::vec3 cross = glm::normalize(glm::cross(m_vec3Start, m_vec3Current)) * sina;
		m_quatNext = glm::quat(cosa, cross.x, cross.y, cross.z);

		// update the rotation matrix
		m_quatOrientation = m_quatNext * m_quatLast;
	}
}

void Arcball::getViewport()
{
	glGetFloatv(GL_VIEWPORT, glm::value_ptr(m_vec4Viewport));
}

void Arcball::getProjectionMatrix()
{
	glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(m_mat4Projection));
}
