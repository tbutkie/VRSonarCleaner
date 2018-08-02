#include "IllustrativeDyePole.h"

IllustrativeDyePole::IllustrativeDyePole(float xLoc, float yLoc, float DepthBottom, float DepthTop)
{
	x = xLoc;
	y = yLoc;
	depthBottom = DepthBottom;
	depthTop = DepthTop;
	deleteMe = false;
}

IllustrativeDyePole::~IllustrativeDyePole()
{
	emitters.clear();
}

void IllustrativeDyePole::addEmitter(float DepthBottom, float DepthTop)
{
	IllustrativeParticleEmitter* tempPE = new IllustrativeParticleEmitter(x, y, (DepthTop - DepthBottom) * 0.5f);
	if (emitters.size() > 0)
		tempPE->changeColor(emitters.at(emitters.size()-1)->color+1);
	else
		tempPE->changeColor(0);
	emitters.push_back(tempPE);
}

void IllustrativeDyePole::addDefaultEmitter()
{
	IllustrativeParticleEmitter* tempPE = new IllustrativeParticleEmitter(x, y, (depthTop - depthBottom) * 0.5f);
	emitters.push_back(tempPE);	
}

void IllustrativeDyePole::deleteEmitter(int index)
{
	delete emitters.at(index);
	emitters.erase(emitters.begin()+index);
}

void IllustrativeDyePole::removeAllEmitters()
{
	for (int i=0;i<emitters.size();i++)
		delete emitters.at(i);
	emitters.clear();
}

void IllustrativeDyePole::changeEmitterColor(int emitterIndex, int color)
{
	emitters.at(emitterIndex)->changeColor(color);
}

void IllustrativeDyePole::changeEmitterSpread(int emitterIndex, float radius)
{
	emitters.at(emitterIndex)->changeSpread(radius);
}

void IllustrativeDyePole::changeEmitterRate(int emitterIndex, float msBetweenParticles)
{
	emitters.at(emitterIndex)->setRate(msBetweenParticles);
}

void IllustrativeDyePole::changeEmitterLifetime(int emitterIndex, std::chrono::milliseconds lifetime)
{
	emitters.at(emitterIndex)->setLifetime(lifetime);
}

void IllustrativeDyePole::changeEmitterTrailtime(int emitterIndex, std::chrono::milliseconds trailTime)
{
	emitters.at(emitterIndex)->setTrailTime(trailTime);
}

int IllustrativeDyePole::getNumEmitters()
{
	return static_cast<int>(emitters.size());
}

float IllustrativeDyePole::getBottomActualDepth()
{
	return depthBottom;
}

float IllustrativeDyePole::getTopActualDepth()
{
	return depthTop;
}

float IllustrativeDyePole::getActualDepthOf(float Z)
{
	return Z;
}

void IllustrativeDyePole::kill()
{
	deleteMe = true;
}

bool IllustrativeDyePole::shouldBeDeleted()
{
	return deleteMe;
}