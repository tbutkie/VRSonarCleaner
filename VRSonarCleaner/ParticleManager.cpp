#include "ParticleManager.h"

#include <algorithm>
#include <iterator>
#include <iostream>

#include "Icosphere.h"

// CTOR
ParticleManager::ParticleManager()
{
	_init();
}

void ParticleManager::_init()
{
	// populate free particle index pool
	for (int i = MAX_NUM_PARTICLES - 1; i >= 0; --i)
	{
		m_rParticles[i].m_iID = i;
	}
	
	// initialize dynamic buffer
	glm::vec3 initBufferVals;
	std::fill_n(m_rvec3DynamicBuffer, MAX_NUM_PARTICLES, initBufferVals);
}

void ParticleManager::_initGL()
{
	// ------------------------------ DATA GENERATION ------------------------------
	Icosphere sphere(3);
	std::vector<glm::vec3> verts = sphere.getVertices();
	std::vector<unsigned int> indices = sphere.getIndices();

	m_glPrimIndCount = indices.size();

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texture;
	};

	std::vector<Vertex> vertices;
	for (auto const &v : verts)
	{
		Vertex tempVert;
		tempVert.position = v;
		tempVert.normal = v;
		tempVert.texture = glm::vec2(0.f);
	}

	// ------------------------------ VBO CREATION AND DATA TRANSFER ------------------------------
	glGenBuffers(1, &m_glVBO);
	glGenBuffers(1, &m_glEBO);
	glGenBuffers(1, &m_glUBO);

	// Bind buffer for vertex info and fill it
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_glPrimIndCount * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// Bind buffer for instance info and fill it
	glBindBuffer(GL_ARRAY_BUFFER, m_glUBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_NUM_PARTICLES * sizeof(glm::vec3), &m_rvec3DynamicBuffer[0], GL_STREAM_DRAW);



	// ------------------------------ VAO SETUP ------------------------------
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(m_glVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glEBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glUBO);

	// ------------------------------ VAO PER-VERTEX ATTRIBUTES ------------------------------
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 0);
	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, texture)));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 0);

	// ------------------------------ VAO PER-INSTANCE ATTRIBUTES ------------------------------
	// Instance base location attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (GLvoid*)(sizeof(glm::vec3) * 0));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);
}

void ParticleManager::add(ParticleSystem * ps)
{
	// check if we have enough free particles available for this system
	if (ps->m_nParticles > (MAX_NUM_PARTICLES - m_iLiveParticleCount))
	{
		std::cerr << "Not enough free particles to add particle system!" << std::endl;
		std::cerr << "\t" << ps->m_nParticles << " requested, " << (MAX_NUM_PARTICLES - m_iLiveParticleCount) << " available" << std::endl;
		return;
	}

	// add to the group of particle systems we will manage
	m_vpParticleSystems.push_back(ps);

	for (int i = 0; i < ps->m_nParticles; ++i)
	{
		ps->setParticleDefaults(m_rParticles[m_iLiveParticleCount++]);
	}
}

// does not call delete on particle system pointer
void ParticleManager::remove(ParticleSystem * ps)
{
	// get the indices of the freed particles
	std::vector<int> freedParticleIndices = ps->releaseParticles();

	// swap dead particles for live ones at the end of the array index
	for (auto const &i : freedParticleIndices)
	{
		Particle dead = m_rParticles[i]; // get dead particle
		m_rParticles[i] = m_rParticles[m_iLiveParticleCount - 1]; // replace dead particle with last live particle
		m_rParticles[m_iLiveParticleCount - 1] = dead; // put the dead particle at the end of the array
		m_iLiveParticleCount--; // decrement number of live particles
	}

	// remove particle system
	std::remove(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps);
}

bool ParticleManager::exists(ParticleSystem * ps) const
{
	return std::find(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps) != m_vpParticleSystems.end();
}

void ParticleManager::update(float time)
{
	for (auto &ps : m_vpParticleSystems)
		if (!ps->update(time))
			remove(ps);

	// update data for UBO
	for (int i = 0; i < m_iLiveParticleCount; ++i)
		m_rvec3DynamicBuffer[i] = m_rParticles[i].m_vec3Pos;
	
	// update UBO
	//glBindBuffer(GL_ARRAY_BUFFER, m_glUBO); 
	//glBufferData(GL_ARRAY_BUFFER, MAX_NUM_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, m_iLiveParticleCount * sizeof(glm::vec3), &m_rvec3DynamicBuffer[0]);
}

void ParticleManager::render()
{
	glBindVertexArray(m_glVAO);
	glDrawElementsInstanced(GL_TRIANGLES, // rendering triangle primitives
		m_glPrimIndCount,				  // number of indices to be used in rendering
		GL_UNSIGNED_INT,				  // indices array type is unsigned int
		(GLvoid*)0,                       // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
		m_iLiveParticleCount);			  // number of instances to render
	glBindVertexArray(0);
}
