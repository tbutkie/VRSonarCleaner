#include "CosmoVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

CosmoVolume::CosmoVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth, bool fgFile)
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
		addFlowGrid(fg);
		
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
	
	m_pParticleSystem = new IllustrativeParticleSystem(m_vpCosmoGrids);
}

CosmoVolume::~CosmoVolume()
{

}

void CosmoVolume::addFlowGrid(std::string fileName)
{
	FILE *inputFile;
	printf("Opening binary 3-vector file: %s\n", fileName);

	inputFile = fopen(fileName.c_str(), "rb");

	if (inputFile == NULL)
	{
		printf("Unable to open binary input file!\n");
		return;
	}

	setName(fileName);

	checkNewPosition(glm::dvec3(m_fXMin, m_fYMin, m_fZMin));
	checkNewPosition(glm::dvec3(m_fXMax, m_fYMax, m_fZMax));

	init();

	for (int i = 0; i < m_nZCells; i++)
		setDepthValue(i, static_cast<float>(i) * m_fZCellSize);

	for (int i = 0; i < m_nTimesteps; i++)
	{
		setTimeValue(i, 1.f);
	}

	int index4d;
	float tempU, tempV, tempW;
	for (int x = 0; x < m_nXCells; x++)
	{
		for (int y = 0; y < m_nYCells; y++)
		{
			for (int z = 0; z < m_nZCells; z++)
			{
				for (int t = 0; t < m_nTimesteps; t++)
				{
					index4d = (t*m_nXYZCells) + (z*m_nXYCells) + (y*m_nXCells) + x;
					fread(&tempU, sizeof(float), 1, inputFile);
					fread(&tempV, sizeof(float), 1, inputFile);
					fread(&tempW, sizeof(float), 1, inputFile);
					setIsWaterValue(x, y, z, t, true);
					setCellValue(x, y, z, t, tempU, tempV, tempW);
				}//end for z
			}//end for z
		}//end for y
	}//end for x

	fclose(inputFile);

	m_bLoaded = true;

	printf("Imported FlowGrid from binary file %s\n", filename);
}

void CosmoVolume::removeFlowGrid(std::string fileName)
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

glm::vec3 CosmoVolume::getFlowWorldCoords(glm::vec3 pt_WorldCoords)
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

void CosmoVolume::recalcVolumeBounds()
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

IllustrativeParticleEmitter* CosmoVolume::placeDyeEmitterWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToRawDomainCoords(pos);

	printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos);
	tmp->setRate(0.f);
	tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
	m_pParticleSystem->m_vpDyePots.push_back(tmp);

	return tmp;
}

bool CosmoVolume::removeDyeEmitterClosestToWorldCoords(glm::vec3 pos)
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

void CosmoVolume::particleSystemIntegrateEuler()
{
	m_pParticleSystem->setEulerIntegration();
}

void CosmoVolume::particleSystemIntegrateRK4()
{
	m_pParticleSystem->setRK4Integration();
}

void CosmoVolume::update()
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

void CosmoVolume::draw()
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

void CosmoVolume::setParticleVelocityScale(float velocityScale)
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			fg->m_fIllustrativeParticleVelocityScale = velocityScale;
}

float CosmoVolume::getParticleVelocityScale()
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			return fg->m_fIllustrativeParticleVelocityScale;
	
	return -1.f;
}