/* Arcball, written by Bradley Smith, March 24, 2006
 * arcball.cpp is free to use and modify for any purpose, with no
 * restrictions of copyright or license.
 *
 * See arcball.h for usage details.
 */


#include "arcball.h"
#include <GL\gl.h>
#include <GL\glu.h>

GLfloat ab_quat[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLfloat ab_last[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLfloat ab_next[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

// the distance from the origin to the eye
GLfloat ab_zoom = 1.0;
GLfloat ab_zoom2 = 1.0;
// the radius of the arcball
GLfloat ab_sphere = 1.0;
GLfloat ab_sphere2 = 1.0;
// the distance from the origin of the plane that intersects
// the edge of the visible sphere (tangent to a ray from the eye)
GLfloat ab_edge = 1.0;
// whether we are using a sphere or plane
bool ab_planar = false;
GLfloat ab_planedist = 0.5;

glm::vec3 ab_start = glm::vec3(0,0,1);
glm::vec3 ab_curr = glm::vec3(0,0,1);
glm::vec3 ab_eye = glm::vec3(0,0,1);
glm::vec3 ab_eyedir = glm::vec3(0,0,1);
glm::vec3 ab_up = glm::vec3(0,1,0);
glm::vec3 ab_out = glm::vec3(1,0,0);

GLdouble ab_glp[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLdouble ab_glm[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
int ab_glv[4] = {0,0,640,480};

void arcball_setzoom(float radius, glm::vec3 eye, glm::vec3 up)
{
  ab_eye = eye; // store eye vector
  ab_zoom2 = glm::dot(ab_eye, ab_eye);
  ab_zoom = sqrt(ab_zoom2); // store eye distance
  ab_sphere = radius; // sphere radius
  ab_sphere2 = glm::dot(ab_sphere, ab_sphere);
  ab_eyedir = ab_eye * (1.f / ab_zoom); // distance to eye
  ab_edge = ab_sphere2 / ab_zoom; // plane of visible edge
  
  if(ab_sphere <= 0.0) // trackball mode
  {
    ab_planar = true;
    ab_up = up;
    ab_out = glm::cross( ab_eyedir, ab_up );
    ab_planedist = (0.f - ab_sphere) * ab_zoom;
  } else
    ab_planar = false;
    
  glGetDoublev(GL_PROJECTION_MATRIX,ab_glp);
  glGetIntegerv(GL_VIEWPORT,ab_glv);
}

// affect the arcball's orientation on openGL
void arcball_rotate() { glMultMatrixf(ab_quat); }

// convert the quaternion into a rotation matrix
static void quaternion(GLfloat* q, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  GLfloat x2 = x*x;
  GLfloat y2 = y*y;
  GLfloat z2 = z*z;
  GLfloat xy = x*y;
  GLfloat xz = x*z;
  GLfloat yz = y*z;
  GLfloat wx = w*x;
  GLfloat wy = w*y;
  GLfloat wz = w*z;

  q[0] = 1 - 2*y2 - 2*z2;
  q[1] = 2*xy + 2*wz;
  q[2] = 2*xz - 2*wy;
  
  q[4] = 2*xy - 2*wz;
  q[5] = 1 - 2*x2 - 2*z2;
  q[6] = 2*yz + 2*wx;
  
  q[8] = 2*xz + 2*wy;
  q[9] = 2*yz - 2*wx;
  q[10]= 1 - 2*x2 - 2*y2;
}

// reset the rotation matrix
static void quatidentity(GLfloat* q)
{ q[0]=1;  q[1]=0;  q[2]=0;  q[3]=0;
  q[4]=0;  q[5]=1;  q[6]=0;  q[7]=0;
  q[8]=0;  q[9]=0;  q[10]=1; q[11]=0;
  q[12]=0; q[13]=0; q[14]=0; q[15]=1; }

// copy a rotation matrix
static void quatcopy(GLfloat* dst, GLfloat* src)
{ dst[0]=src[0]; dst[1]=src[1]; dst[2]=src[2];
  dst[4]=src[4]; dst[5]=src[5]; dst[6]=src[6];
  dst[8]=src[8]; dst[9]=src[9]; dst[10]=src[10]; }

// multiply two rotation matrices
static void quatnext(GLfloat* dest, GLfloat* left, GLfloat* right)
{
  dest[0] = left[0]*right[0] + left[1]*right[4] + left[2] *right[8];
  dest[1] = left[0]*right[1] + left[1]*right[5] + left[2] *right[9];
  dest[2] = left[0]*right[2] + left[1]*right[6] + left[2] *right[10];
  dest[4] = left[4]*right[0] + left[5]*right[4] + left[6] *right[8];
  dest[5] = left[4]*right[1] + left[5]*right[5] + left[6] *right[9];
  dest[6] = left[4]*right[2] + left[5]*right[6] + left[6] *right[10];
  dest[8] = left[8]*right[0] + left[9]*right[4] + left[10]*right[8];
  dest[9] = left[8]*right[1] + left[9]*right[5] + left[10]*right[9];
  dest[10]= left[8]*right[2] + left[9]*right[6] + left[10]*right[10];
}

// find the intersection with the plane through the visible edge
static glm::vec3 edge_coords(glm::vec3 m)
{
  // find the intersection of the edge plane and the ray
  float t = (ab_edge - ab_zoom) / glm::dot(ab_eyedir, m);
  glm::vec3 a = ab_eye + (m*t);
  // find the direction of the eye-axis from that point
  // along the edge plane
  glm::vec3 c = (ab_eyedir * ab_edge) - a;

  // find the intersection of the sphere with the ray going from
  // the plane outside the sphere toward the eye-axis.
  float ac = glm::dot(a, c);
  float c2 = glm::dot(c, c);
  float q = (0.f - ac - sqrt( glm::dot(ac, ac) - c2*(glm::dot(a, a)-ab_sphere2) ) ) / c2;
  
  return glm::normalize(a+(c*q));
}

// find the intersection with the sphere
static glm::vec3 sphere_coords(GLdouble mx, GLdouble my)
{
  GLdouble ax,ay,az;

  gluUnProject(mx,my,0,ab_glm,ab_glp,ab_glv,&ax,&ay,&az);
  glm::vec3 m = glm::vec3((float)ax,(float)ay,(float)az) - ab_eye;
  
  // mouse position represents ray: eye + t*m
  // intersecting with a sphere centered at the origin
  GLfloat a = glm::dot(m, m);
  GLfloat b = glm::dot(ab_eye, m);
  GLfloat root = (b*b) - a*(ab_zoom2 - ab_sphere2);
  if(root <= 0) return edge_coords(m);
  GLfloat t = (0.f - b - sqrt(root)) / a;
  return glm::normalize(ab_eye+(m*t));
}

// get intersection with plane for "trackball" style rotation
static glm::vec3 planar_coords(GLdouble mx, GLdouble my)
{
  GLdouble ax,ay,az;

  gluUnProject(mx,my,0,ab_glm,ab_glp,ab_glv,&ax,&ay,&az);
  glm::vec3 m = glm::vec3((float)ax,(float)ay,(float)az) - ab_eye;
  // intersect the point with the trackball plane
  GLfloat t = (ab_planedist - ab_zoom) / glm::dot(ab_eyedir, m);
  glm::vec3 d = ab_eye + m*t;

  return glm::vec3(glm::dot(d, ab_up), glm::dot(d, ab_out),0.f);
}

// reset the arcball
void arcball_reset()
{
  quatidentity(ab_quat);
  quatidentity(ab_last);
}

// begin arcball rotation
void arcball_start(int mx, int my)
{
  // saves a copy of the current rotation for comparison
  quatcopy(ab_last,ab_quat);
  if(ab_planar) ab_start = planar_coords((GLdouble)mx,(GLdouble)my);
  else ab_start = sphere_coords((GLdouble)mx,(GLdouble)my);
}

// update current arcball rotation
void arcball_move(int mx, int my)
{
  if(ab_planar)
  {
    ab_curr = planar_coords((GLdouble)mx,(GLdouble)my);
    if(ab_curr == ab_start) return;
    
    // d is motion since the last position
	glm::vec3 d = ab_curr - ab_start;
    
    GLfloat angle = d.length() * 0.5f;
    GLfloat cosa = cos( angle );
    GLfloat sina = sin( angle );
    // p is perpendicular to d
	glm::vec3 p = glm::normalize((ab_out*d.x)-(ab_up*d.y)) * sina;

    quaternion(ab_next,p.x,p.y,p.z,cosa);
    quatnext(ab_quat,ab_last,ab_next);
    // planar style only ever relates to the last point
    quatcopy(ab_last,ab_quat);
    ab_start = ab_curr;
    
  } else {

    ab_curr = sphere_coords((GLdouble)mx,(GLdouble)my);
    if(ab_curr == ab_start)
    { // avoid potential rare divide by tiny
      quatcopy(ab_quat, ab_last);
      return;
    }

    // use a dot product to get the angle between them
    // use a cross product to get the vector to rotate around
    GLfloat cos2a = glm::dot(ab_start, ab_curr);
    GLfloat sina = sqrt((1.f - cos2a)*0.5f);
    GLfloat cosa = sqrt((1.f + cos2a)*0.5f);
	glm::vec3 cross = glm::normalize(glm::cross(ab_start, ab_curr)) * sina;
    quaternion(ab_next,cross.x,cross.y,cross.z,cosa);

    // update the rotation matrix
    quatnext(ab_quat,ab_last,ab_next);
  }
}
