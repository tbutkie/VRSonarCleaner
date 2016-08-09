#pragma once

#include <GL/glew.h>

#include "TrackedDevice.h"

class ViveController :
	public TrackedDevice
{
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~ViveController();

	bool BInit();

	void update();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual void systemButtonPressed();
	virtual void systemButtonUnpressed();
	bool isSystemButtonPressed();

	virtual void menuButtonPressed();
	virtual void menuButtonUnpressed();
	bool isMenuButtonPressed();

	virtual void gripButtonPressed();
	virtual void gripButtonUnpressed();
	bool isGripButtonPressed();

	virtual void triggerEngaged();
	virtual void triggerBeingPulled(float amount);
	virtual void triggerDisengaged();
	virtual void triggerClicked();
	virtual void triggerUnclicked();
	bool isTriggerEngaged();
	bool isTriggerClicked();
	float getHairTriggerThreshold();
	
	virtual void touchpadInitialTouch(float x, float y);
	virtual void touchpadTouch(float x, float y);
	virtual void touchpadUntouched();
	virtual void touchPadClicked(float x, float y);
	virtual void touchPadUnclicked(float x, float y);
	bool isTouchpadTouched();
	bool isTouchpadClicked();

	void renderModel(Matrix4 & matVP);

protected:
	struct ControllerComponent {
		uint32_t m_unComponentIndex;
		std::string m_strComponentName;
		CGLRenderModel *m_pComponentRenderModel;
		vr::HmdMatrix34_t m_mat3PoseTransform;
		bool m_bInitialized;
		bool m_bHasRenderModel;
		bool m_bStatic;
		bool m_bVisible;
		bool m_bTouched;
		bool m_bPressed;
		bool m_bScrolled;

		ControllerComponent()
			: m_unComponentIndex(0)
			, m_strComponentName("No name")
			, m_pComponentRenderModel(NULL)
			, m_mat3PoseTransform(vr::HmdMatrix34_t())
			, m_bInitialized(false)
			, m_bHasRenderModel(false)
			, m_bStatic(false)
			, m_bVisible(false)
			, m_bTouched(false)
			, m_bPressed(false)
			, m_bScrolled(false)
		{}
	};

	std::vector<ControllerComponent> m_vComponents;

	Vector4 transformTouchPointToModelCoords(Vector2 *pt);
	void insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b);
	
	uint32_t m_unStatePacketNum;

	bool m_bShowScrollWheel;
	bool m_bSystemButtonClicked;
	bool m_bMenuButtonClicked;
	bool m_bGripButtonClicked;
	bool m_bTouchpadTouched;
	bool m_bTouchpadClicked;
	Vector2 m_vec2TouchpadInitialTouchPoint;
	Vector2 m_vec2TouchpadCurrentTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
	float m_fHairTriggerThreshold; // how much trigger is pulled before being considered engaged
	uint32_t m_unTriggerAxis;
	uint32_t m_unTouchpadAxis;

	Icosphere m_TouchPointSphere;

	const Vector4 c_vec4TouchPadCenter;
	const Vector4 c_vec4TouchPadLeft;
	const Vector4 c_vec4TouchPadRight;
	const Vector4 c_vec4TouchPadTop;
	const Vector4 c_vec4TouchPadBottom;

	vr::VROverlayHandle_t m_pOverlayHandle;
};