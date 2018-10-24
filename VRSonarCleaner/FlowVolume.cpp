#include "FlowVolume.h"

#include "Renderer.h"
#include <Eigen\Dense>

using namespace std::chrono_literals;

FlowVolume::FlowVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(1.f))
	, m_msLoopTime(35s)
	, m_bParticleSystemUpdating(false)
	, m_fFlowRoomMinTime(std::numeric_limits<float>::max())
	, m_fFlowRoomMaxTime(std::numeric_limits<float>::min())
	, m_pParticleSystem(NULL)
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	for (auto fg : flowGrids)
		addFlowGrid(fg, useZInsteadOfDepth);
		
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
	
	m_pParticleSystem = new IllustrativeParticleSystem(m_vpFlowGrids);
}

FlowVolume::~FlowVolume()
{

}

void FlowVolume::addFlowGrid(std::string fileName, bool useZInsteadOfDepth)
{
	// Check if flowgrid already exists with that name
	removeFlowGrid(fileName);

	FlowGrid* tempFG = new FlowGrid(fileName.c_str(), useZInsteadOfDepth);
	m_vpFlowGrids.push_back(tempFG);
	add(tempFG);

	if (tempFG->m_fMinTime < m_fFlowRoomMinTime)
		m_fFlowRoomMinTime = tempFG->m_fMinTime;
	if (tempFG->m_fMaxTime > m_fFlowRoomMaxTime)
		m_fFlowRoomMaxTime = tempFG->m_fMaxTime;

	if (m_pParticleSystem)
	{
		m_pParticleSystem->m_vpFlowGridCollection = m_vpFlowGrids;
	}
}

void FlowVolume::removeFlowGrid(std::string fileName)
{
	auto fgPresent = std::find_if(m_vpFlowGrids.begin(), m_vpFlowGrids.end(), [&fileName](FlowGrid* fg) { return strcmp(fg->getName(), fileName.c_str()) == 0; });

	if (fgPresent != m_vpFlowGrids.end())
	{
		if (m_pParticleSystem)
		{
			m_pParticleSystem->m_vpFlowGridCollection = m_vpFlowGrids;
			for (auto &p : m_pParticleSystem->m_vpParticles)
				if (p->m_pFlowGrid == (*fgPresent))
					p->m_pFlowGrid = NULL;
		}

		remove(*fgPresent);
		m_vpFlowGrids.erase(fgPresent);
	}
}

glm::vec3 FlowVolume::getFlowWorldCoords(glm::vec3 pt_WorldCoords)
{
	glm::vec3 domainPt = convertToRawDomainCoords(pt_WorldCoords);

	for (auto &fg : m_vpFlowGrids)
	{
		float u, v, w;
		if (fg->getUVWat(domainPt.x, domainPt.y, domainPt.z, 0, &u, &v, &w))
			return convertToWorldCoords(glm::dvec3(domainPt) + glm::dvec3(u, v, w)) - pt_WorldCoords;		
	}
		return glm::vec3(0.f);
}

float FlowVolume::getLambda2(glm::vec3 pt_WorldCoords)
{
	glm::vec3 domainPt = convertToRawDomainCoords(pt_WorldCoords);

	glm::vec3 xMinPos(glm::max(domainPt.x - 1.f, static_cast<float>(getMinXDataBound())), domainPt.y, domainPt.z);
	glm::vec3 xMaxPos(glm::min(domainPt.x + 1.f, static_cast<float>(getMaxXDataBound())), domainPt.y, domainPt.z);
	glm::vec3 yMinPos(domainPt.x, glm::max(domainPt.y - 1.f, static_cast<float>(getMinYDataBound())), domainPt.z);
	glm::vec3 yMaxPos(domainPt.x, glm::min(domainPt.y + 1.f, static_cast<float>(getMaxYDataBound())), domainPt.z);
	glm::vec3 zMinPos(domainPt.x, domainPt.y, glm::max(domainPt.z - 1.f, static_cast<float>(getMinZDataBound())));
	glm::vec3 zMaxPos(domainPt.x, domainPt.y, glm::min(domainPt.z + 1.f, static_cast<float>(getMaxZDataBound())));

	float uxmin, uxmax, uymin, uymax, uzmin, uzmax, vxmin, vxmax, vymin, vymax, vzmin, vzmax, wxmin, wxmax, wymin, wymax, wzmin, wzmax;

	std::vector<float> lambdas;

	for (auto &fg : m_vpFlowGrids)
	{
		fg->getUVWat(xMinPos.x, xMinPos.y, xMinPos.z, 0, &uxmin, &vxmin, &wxmin);
		fg->getUVWat(xMaxPos.x, xMaxPos.y, xMaxPos.z, 0, &uxmax, &vxmax, &wxmax);
		fg->getUVWat(yMinPos.x, yMinPos.y, yMinPos.z, 0, &uymin, &vymin, &wymin);
		fg->getUVWat(yMaxPos.x, yMaxPos.y, yMaxPos.z, 0, &uymax, &vymax, &wymax);
		fg->getUVWat(zMinPos.x, zMinPos.y, zMinPos.z, 0, &uzmin, &vzmin, &wzmin);
		fg->getUVWat(zMaxPos.x, zMaxPos.y, zMaxPos.z, 0, &uzmax, &vzmax, &wzmax);

		Eigen::MatrixXf jacobian = Eigen::MatrixXf(3, 3);

		jacobian(0, 0) = (uxmax - uxmin) / glm::distance(xMinPos, xMaxPos);
		jacobian(0, 1) = (vxmax - vxmin) / glm::distance(xMinPos, xMaxPos);
		jacobian(0, 2) = (wxmax - wxmin) / glm::distance(xMinPos, xMaxPos);

		jacobian(1, 0) = (uymax - uymin) / glm::distance(yMinPos, yMaxPos);
		jacobian(1, 1) = (vymax - vymin) / glm::distance(yMinPos, yMaxPos);
		jacobian(1, 2) = (wymax - wymin) / glm::distance(yMinPos, yMaxPos);

		jacobian(2, 0) = (uzmax - uzmin) / glm::distance(zMinPos, zMaxPos);
		jacobian(2, 1) = (vzmax - vzmin) / glm::distance(zMinPos, zMaxPos);
		jacobian(2, 2) = (wzmax - wzmin) / glm::distance(zMinPos, zMaxPos);
		

		Eigen::MatrixXf s = (jacobian + jacobian.transpose()) / 2.f;
		Eigen::MatrixXf omega = (jacobian - jacobian.transpose()) / 2.f;

		Eigen::MatrixXf vorticity = s*s + omega*omega;

		Eigen::EigenSolver<Eigen::MatrixXf> es(vorticity);
		//std::cout << es.eigenvalues() << std::endl;
		
		for (int i = 0; i < 3; ++i)
			lambdas.push_back(es.eigenvalues()[i].real());
	}

	std::sort(lambdas.begin(), lambdas.end());
	return lambdas[1];
}

void FlowVolume::recalcVolumeBounds()
{
	glm::vec3 minCoords(std::numeric_limits<float>::max());
	glm::vec3 maxCoords(std::numeric_limits<float>::min());

	for (auto fg : m_vpFlowGrids)
	{
		if (fg)
		{
			if (fg->getXMin() < minCoords.x)
				minCoords.x = fg->getXMin();
			if (fg->getXMax() > maxCoords.x)
				maxCoords.x = fg->getXMax();

			if (fg->getYMin() < minCoords.y)
				minCoords.y = fg->getYMin();
			if (fg->getYMax() < maxCoords.y)
				maxCoords.y = fg->getYMax();

			float zMin = fg->m_bUsesZInsteadOfDepth ? fg->getMinDepth() : -fg->getMinDepth();
			float zMax = fg->m_bUsesZInsteadOfDepth ? fg->getMaxDepth() : -fg->getMaxDepth();

			if (zMin < minCoords.z)
				minCoords.z = zMin;
			if (zMax < maxCoords.z)
				maxCoords.z = zMax;
		}
	}

	//setCustomBounds(minCoords, maxCoords);
}

IllustrativeParticleEmitter* FlowVolume::placeDyeEmitterWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToRawDomainCoords(pos);

	printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos);
	tmp->setRate(0.f);
	tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
	m_pParticleSystem->m_vpDyePots.push_back(tmp);

	return tmp;
}

bool FlowVolume::removeDyeEmitterClosestToWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToRawDomainCoords(pos);

	printf("Deleting Dye Pot Closest to:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = m_pParticleSystem->getDyePotClosestTo(innerPos.x, innerPos.y, innerPos.z);
	if (tmp)
	{
		m_pParticleSystem->m_vpDyePots.erase(std::remove(m_pParticleSystem->m_vpDyePots.begin(), m_pParticleSystem->m_vpDyePots.end(), tmp), m_pParticleSystem->m_vpDyePots.end());
		delete tmp;
	}
	return false;
}

void FlowVolume::particleSystemIntegrateEuler()
{
	m_pParticleSystem->setEulerIntegration();
}

void FlowVolume::particleSystemIntegrateRK4()
{
	m_pParticleSystem->setRK4Integration();
}

void FlowVolume::update()
{
	// update the data volume first
	DataVolume::update();

	//update time
	auto tick = std::chrono::high_resolution_clock::now();
	auto timeSinceLast = tick - m_tpLastTimeUpdate;
	if (timeSinceLast > 20ms)
	{
		m_tpLastTimeUpdate = tick;
		float currentTime = m_fFlowRoomTime;
		float minTime = m_fFlowRoomMinTime;
		float maxTime = m_fFlowRoomMaxTime;
		float timeRange = maxTime - minTime;

		if (timeRange > 0.000001f)
		{
			if (true) ///playing v paused
			{
				float factorToAdvance = std::chrono::duration<float, std::milli>(timeSinceLast).count() / std::chrono::duration<float, std::milli>(m_msLoopTime).count();
				float newTime = currentTime + (factorToAdvance * timeRange);
				if (newTime > maxTime)
					newTime = minTime;
				m_fFlowRoomTime = newTime;
			}
		}//end if timerange greater than zero
		else //no time range
		{
			//playing = false;
		}
	}//end if need update

	if (m_Future.valid() && m_Future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
	{
		m_pParticleSystem->prepareForRender();

		m_bParticleSystemUpdating = false;
	}

	 //m_pParticleSystem->update(m_fFlowRoomTime);
	if (!m_bParticleSystemUpdating)
	{
		m_Future = std::async(std::launch::async, [&] { m_pParticleSystem->update(m_fFlowRoomTime); });
		m_bParticleSystemUpdating = true;
	}
}

void FlowVolume::draw()
{
	if (m_pParticleSystem->getIndexCount() < 2)
		return;

	Renderer::RendererSubmission rs;

	rs.glPrimitiveType = GL_LINES;
	rs.shaderName = "flat";	
	rs.VAO = m_pParticleSystem->getVAO();
	rs.vertCount = m_pParticleSystem->getIndexCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.hasTransparency = true;
	rs.transparencySortPosition = getTransformVolume()[3];
	rs.modelToWorldTransform = getTransformRawDomainToVolume();

	Renderer::getInstance().addToDynamicRenderQueue(rs);
}

void FlowVolume::setParticleVelocityScale(float velocityScale)
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			fg->m_fIllustrativeParticleVelocityScale = velocityScale;
}

float FlowVolume::getParticleVelocityScale()
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			return fg->m_fIllustrativeParticleVelocityScale;
	
	return -1.f;
}
