#include "ArcBall.h"

#include <gtx/intersect.hpp>
#include <sstream>

#include "Renderer.h"
#include "DataLogger.h"

//------------------------------------------------------------------------------
ArcBall::ArcBall(DataVolume *dataVolume)
	: m_pDataVolume(dataVolume)
	, m_pmat4Projection(NULL)
	, m_pmat4View(NULL)
	, m_vec3PivotPoint(dataVolume->getPosition())
	, m_fTranslationTime(0.1f)
	, m_bDragging(false)
{
	reset();
}

//------------------------------------------------------------------------------
ArcBall::~ArcBall()
{
}

void ArcBall::reset()
{
	glm::quat qOne(1.0, 0.0, 0.0, 0.0);
	glm::vec3 vZero(0.0, 0.0, 0.0);

	mVDown = vZero;
	mVNow = vZero;
	mQDown = qOne;
	mQNow = qOne;


	m_bDragging = false;
}

void ArcBall::update()
{
	calculateRadius();

	m_pDataVolume->setOrientation(getOrientation() * m_pDataVolume->getOriginalOrientation());

	float timeElapsed = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_tpStartTrans).count();

	if (timeElapsed <= m_fTranslationTime)
	{
		float ratio = timeElapsed / m_fTranslationTime;
		glm::vec3 transDir = m_vec3EndTransPos - m_vec3StartTransPos;
		m_pDataVolume->setPosition(m_vec3StartTransPos + transDir * ratio);
		Renderer::getInstance().drawConnector(m_vec3PivotPoint, m_vec3PivotPoint - transDir * (1.f - ratio), 0.005f, glm::vec4(0.7f, 0.7f, 0.2f, 0.7f));
	}
}

void ArcBall::draw()
{
	if (m_bDragging)
		Renderer::getInstance().drawConnector(m_vec3PivotPoint, m_pDataVolume->getPosition(), 0.005f, glm::vec4(0.7f));
}

void ArcBall::setProjection(glm::mat4 * proj)
{
	m_pmat4Projection = proj;
}

void ArcBall::setView(glm::mat4 * view)
{
	m_pmat4View = view;
}

void ArcBall::setViewport(glm::ivec4 & vp)
{
	m_ivec4Viewport = vp;

	mCenter = glm::vec3(vp[2] - vp[0], vp[3] - vp[1], 0.f) * 0.5f;
}

void ArcBall::calculateRadius()
{
	float tableRad = m_pDataVolume->getBoundingRadius();

	glm::vec3 right = glm::normalize(glm::inverse(*m_pmat4View)[0]);
	glm::vec3 tableRadPt = m_vec3PivotPoint + right * tableRad;

	glm::vec3 pt = glm::project(tableRadPt, *m_pmat4View, *m_pmat4Projection, m_ivec4Viewport);
	mRadius = pt.x - (m_ivec4Viewport[2] - m_ivec4Viewport[0]) * 0.5f;
}

//------------------------------------------------------------------------------
glm::vec3 ArcBall::mouseOnSphere(const glm::vec3& tscMouse)
{
  glm::vec3 ballMouse;

  // (m - C) / R
  ballMouse.x = (tscMouse.x - mCenter.x) / mRadius;
  ballMouse.y = (tscMouse.y - mCenter.y) / mRadius;

  glm::float_t mag = glm::dot(ballMouse, ballMouse);
  if (mag > 1.0)
  {
    // Since we are outside of the sphere, map to the visible boundary of
    // the sphere.
    ballMouse *= 1.0 / sqrtf(mag);
    ballMouse.z = 0.0;
  }
  else
  {
    // We are not at the edge of the sphere, we are inside of it.
    // Essentially, we are normalizing the vector by adding the missing z
    // component.
    ballMouse.z = sqrtf(1.0 - mag);
  }

  return ballMouse;
}

//------------------------------------------------------------------------------
void ArcBall::beginDrag(const glm::vec2& msc)
{
  // The next two lines are usually a part of end drag. But end drag introduces
  // too much statefullness, so we are shortcircuiting it.
  mQDown      = mQNow;

  // Normal 'begin' code.
  mVDown      = (mScreenToTCS * glm::vec4(msc.x, msc.y, 0.0f, 1.0));

  m_vec3StartRotateVec = m_pDataVolume->getPosition() - m_vec3PivotPoint;

  m_bDragging = true;

  if (DataLogger::getInstance().logging())
  {
	  std::stringstream ss;

	  ss << "Arcball Begin" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
	  ss << "\t";
	  ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
	  ss << ";";
	  ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";

	  DataLogger::getInstance().logMessage(ss.str());
  }
}

//------------------------------------------------------------------------------
void ArcBall::drag(const glm::vec2& msc)
{
  // Regular drag code to follow...
  mVNow       = (mScreenToTCS * glm::vec4(msc.x, msc.y, 0.0, 1.0));
  mVSphereFrom= mouseOnSphere(mVDown);
  mVSphereTo  = mouseOnSphere(mVNow);

  // Construct a quaternion from two points on the unit sphere.
  mQDrag = quatFromUnitSphere(mVSphereFrom, mVSphereTo); 
  mQNow = mQDrag * mQDown;

  // Perform complex conjugate (think inverse rotation matrix)
  //glm::quat q = mQNow;
  //q.x = -q.x;
  //q.y = -q.y;
  //q.z = -q.z;
  //q.w =  q.w;

  if (m_pDataVolume->getPosition() != m_vec3PivotPoint)
  {
	  glm::vec3 newVec = glm::rotate(mQDrag, glm::normalize(m_vec3StartRotateVec));
	  m_pDataVolume->setPosition(m_vec3PivotPoint + newVec * glm::length(m_vec3StartRotateVec));
  }
}

void ArcBall::endDrag()
{
	m_bDragging = false;

	if (DataLogger::getInstance().logging())
	{
		std::stringstream ss;

		ss << "Arcball End" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";

		DataLogger::getInstance().logMessage(ss.str());
	}
}

void ArcBall::translate(const glm::vec2 & mouseScreenCoords)
{
	glm::vec3 nearPt(mouseScreenCoords, 0.f);
	glm::vec3 farPt(mouseScreenCoords, 1.f);

	glm::vec3 nearPtWorld = glm::unProject(nearPt, *m_pmat4View, *m_pmat4Projection, m_ivec4Viewport);
	glm::vec3 farPtWorld = glm::unProject(farPt, *m_pmat4View, *m_pmat4Projection, m_ivec4Viewport);

	glm::vec3 rayDir = glm::normalize(farPtWorld - nearPtWorld);
	glm::vec3 planeNorm = glm::normalize(glm::vec3(glm::inverse(*m_pmat4View)[3]) - m_vec3PivotPoint);
	float intersectionDist;

	glm::intersectRayPlane(nearPtWorld, rayDir, m_vec3PivotPoint, planeNorm, intersectionDist);

	glm::vec3 offsetVec = m_vec3PivotPoint - (nearPtWorld + rayDir * intersectionDist);

	m_vec3StartTransPos = m_pDataVolume->getPosition();
	m_vec3EndTransPos = m_pDataVolume->getPosition() + offsetVec;
	m_tpStartTrans = std::chrono::high_resolution_clock::now();

	if (DataLogger::getInstance().logging())
	{
		std::stringstream ss;

		ss << "Translate" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		ss << "\t";
		ss << "vol-pos:\"" << m_pDataVolume->getPosition().x << "," << m_pDataVolume->getPosition().y << "," << m_pDataVolume->getPosition().z << "\"";
		ss << ";";
		ss << "vol-quat:\"" << m_pDataVolume->getOrientation().x << "," << m_pDataVolume->getOrientation().y << "," << m_pDataVolume->getOrientation().z << "," << m_pDataVolume->getOrientation().w << "\"";
		ss << ";";
		ss << "target-pos:\"" << m_vec3EndTransPos.x << "," << m_vec3EndTransPos.y << "," << m_vec3EndTransPos.z << "\"";

		DataLogger::getInstance().logMessage(ss.str());
	}
}

//------------------------------------------------------------------------------
glm::quat ArcBall::quatFromUnitSphere(const glm::vec3& from, const glm::vec3& to)
{
  glm::quat q;
  q.x = from.y*to.z - from.z*to.y;
  q.y = from.z*to.x - from.x*to.z;
  q.z = from.x*to.y - from.y*to.x;
  q.w = from.x*to.x + from.y*to.y + from.z*to.z;
  return q;
}

//------------------------------------------------------------------------------
glm::quat ArcBall::getOrientation() const
{
  return mQNow;
}
