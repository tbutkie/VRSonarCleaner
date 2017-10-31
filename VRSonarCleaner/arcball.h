#ifndef IAUNS_ARC_BALL_H
#define IAUNS_ARC_BALL_H

#include <stdint.h>
#include <chrono>

#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>

#include "BehaviorBase.h"
#include "DataVolume.h"

/// A reimplementation of Ken Shoemake's arcball camera. SCIRun 4's camera
/// system is based off of Ken's code. The Code appears in Graphics Gems 4, 
/// III.1.
/// Unless specified otherwise, all calculations and variables stored in this
/// class are relative to the target coordinate system (TCS) for which there is
/// a transformation from screen space to TCS given by the screenToTCS
/// constructor parameter.
/// If the screenToTCS parameter in the constructor is left as the identity
/// matrix then all values are given in screen coordinates.
/// Screen coordinates are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is the
/// center of the screen.
class ArcBall : public BehaviorBase
{
public:
  /// \param center         Center of the arcball in TCS (screen coordinates if 
  ///                       screenToTCS = identity). Generally this will 
  ///                       always be (0,0,0). But you may move the center
  ///                       in and out of the screen plane to various effect.
  /// \param radius         Radius in TCS. For screen coordinates, a good
  ///                       default is 0.75.
  /// \param screenToTCS    Transformation from screen coordinates
  ///                       to TCS. \p center and \p radius are given in TCS.
  ArcBall(DataVolume *dataVolume);
  virtual ~ArcBall();

  void reset();

  void update();

  void draw();

  void setProjection(glm::mat4 *proj);
  void setView(glm::mat4 *view);
  void setViewport(glm::ivec4 &vp);

  void calculateRadius();
  
  /// Initiate an arc ball drag given the mouse click in screen coordinates.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void beginDrag(const glm::vec2& mouseScreenCoords);

  /// Informs the arcball when the mouse has been dragged.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void drag(const glm::vec2& mouseScreenCoords);

  void endDrag();

  void translate(const glm::vec2& mouseScreenCoords);

  /// Retrieves the current transformation in TCS.
  /// Obtains full transformation of object in question. If the arc ball is 
  /// being used to control camera rotation, then this will contain all
  /// concatenated camera transformations. The current state of the camera
  /// is stored in the quaternions mQDown and mQNow. mMatNow is calculated
  /// from mQNow.
  glm::quat getOrientation() const;

private:

  /// Calculates our position on the ArcBall from 2D mouse position.
  /// \param tscMouse   TSC coordinates of mouse click.
  glm::vec3 mouseOnSphere(const glm::vec3& tscMouse);

  /// Construct a unit quaternion from two points on the unit sphere.
  static glm::quat quatFromUnitSphere(const glm::vec3& from, const glm::vec3& to);

  glm::vec3     mCenter;        ///< Center of the arcball in target coordinate system.
  glm::float_t  mRadius;        ///< Radius of the arcball in target coordinate system.

  glm::quat     mQNow;          ///< Current state of the rotation taking into account mouse.
                                ///< Essentially QDrag * QDown (QDown is a applied first, just
                                ///< as in matrix multiplication).
  glm::quat     mQDown;         ///< State of the rotation since mouse down.
  glm::quat     mQDrag;         ///< Dragged transform. Knows nothing of any prior 
                                ///< transformations.

  glm::vec3     mVNow;          ///< Most current TCS position of mouse (during drag).
  glm::vec3     mVDown;         ///< TCS position of mouse when the drag was begun.
  glm::vec3     mVSphereFrom;   ///< vDown mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.
  glm::vec3     mVSphereTo;     ///< vNow mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.

  /// Transform from screen coordinates to the target coordinate system.
  glm::mat4     mScreenToTCS;

  DataVolume *m_pDataVolume;

  bool m_bDragging;

  glm::vec3 m_vec3PivotPoint;	///< World space pivot point

  glm::mat4 *m_pmat4Projection;
  glm::mat4 *m_pmat4View;
  glm::ivec4 m_ivec4Viewport;

  glm::vec3 m_vec3StartRotatePos;
  glm::vec3 m_vec3StartRotateVec;
  glm::vec3 m_vec3StartTransPos, m_vec3EndTransPos;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_tpStartTrans;
  float m_fTranslationTime;
};

#endif
