#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"
#include "VectorFieldGenerator.h"
#include <unordered_map>

class FlowFieldCurator :
	public BehaviorBase
{
public:
	FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol, int gridRes = 32);
	~FlowFieldCurator();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	FlowVolume* m_pFlowVolume;
	VectorFieldGenerator* m_pVFG;

	struct ControlPoint {
		glm::vec3 pos_raw; // control point position in R^3 bounded by [-1, 1]
		glm::vec3 pos; // control point position in flowgrid format: R^3 bounded by [1, 32]
		glm::vec3 pos_world; // control point world space position
		glm::vec3 dir; // control point direction in R^3 bounded by [-1, 1]
		glm::vec3 end; // end of the control point
		glm::vec3 end_world; // end of the control point in world space coords
		glm::vec3 lamda; // solved lambda coefficients from VecFieldGen in R^3 bounded by [-1, 1]
	};

	std::unordered_map<std::string, ControlPoint> m_vCPs;

	glm::vec3* m_pvec3MovingPt;

	glm::mat4 m_mat4CPOffsetTransform;
	glm::mat4 m_mat4CPOffsetTransformInv;

	glm::vec3 m_vec3ControllerToMovingPt;

	int m_iGridRes;

private:
	bool loadRandomFlowGrid();
	bool loadFlowGridFromCurrentCPs();
	void loadMetaFile(std::string metaFileName);
	void recalculateRawPositions();
};

