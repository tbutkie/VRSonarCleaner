#include "IllustrativeDyePole.h"

IllustrativeDyePole::IllustrativeDyePole(float xLoc, float yLoc, float DepthBottom, float DepthTop, CoordinateScaler *Scaler)
{
	x = xLoc;
	y = yLoc;
	depthBottom = DepthBottom;
	depthTop = DepthTop;
	deleteMe = false;

	scaler = Scaler;
}

IllustrativeDyePole::~IllustrativeDyePole()
{
	emitters.clear();
}

void IllustrativeDyePole::addEmitter(float DepthBottom, float DepthTop)
{
	IllustrativeParticleEmitter* tempPE = new IllustrativeParticleEmitter(x, y, DepthBottom, DepthTop, scaler);
	if (emitters.size() > 0)
		tempPE->changeColor(emitters.at(emitters.size()-1)->getColor()+1);
	else
		tempPE->changeColor(0);
	emitters.push_back(tempPE);
}

void IllustrativeDyePole::addDefaultEmitter()
{
	IllustrativeParticleEmitter* tempPE = new IllustrativeParticleEmitter(x, y, depthBottom, depthTop, scaler);
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

void IllustrativeDyePole::changeEmitterLifetime(int emitterIndex, float lifetime)
{
	emitters.at(emitterIndex)->setLifetime(lifetime);
}

void IllustrativeDyePole::changeEmitterTrailtime(int emitterIndex, float trailTime)
{
	emitters.at(emitterIndex)->setTrailTime(trailTime);
}

int IllustrativeDyePole::getNumEmitters()
{
	return emitters.size();
}

void IllustrativeDyePole::drawSmall3D()
{
	glLineWidth(1);
	glColor3f(DYE_POLE_COLOR);
	glBegin(GL_LINES);
		glVertex3f(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthTop));
		glVertex3f(scaler->getScaledLonX(x), scaler->getScaledLatY(y), scaler->getScaledDepth(depthBottom));
	glEnd();

	for (int i=0;i<emitters.size();i++)
	{
		emitters.at(i)->drawSmall3D();
	}
}

float IllustrativeDyePole::getBottomActualDepth()
{
	return depthBottom;
	/*float interpolatedLayer = (-zBottom/dataset->dataBoundsMaxFakeDepth)*dataset->numDepths;
	int aboveLayer = floor((double)interpolatedLayer);
	int belowLayer = ceil((double)interpolatedLayer);
	float aboveDepth = dataset->getActualDepthAtLayer(aboveLayer);
	float belowDepth = dataset->getActualDepthAtLayer(belowLayer);
	float factor = interpolatedLayer - aboveLayer;
	float depth = factor*belowDepth + (1-factor)*aboveDepth;
	return depth;*/
}

float IllustrativeDyePole::getTopActualDepth()
{
	return depthTop;
	/*float interpolatedLayer = (-zTop/dataset->dataBoundsMaxFakeDepth)*dataset->numDepths;
	int aboveLayer = floor((double)interpolatedLayer);
	int belowLayer = ceil((double)interpolatedLayer);
	float aboveDepth = dataset->getActualDepthAtLayer(aboveLayer);
	float belowDepth = dataset->getActualDepthAtLayer(belowLayer);
	float factor = interpolatedLayer - aboveLayer;
	float depth = factor*belowDepth + (1-factor)*aboveDepth;
	return depth;*/
}

float IllustrativeDyePole::getActualDepthOf(float Z)
{
	return Z;
		//float interpolatedLayer = (-Z/dataset->dataBoundsMaxFakeDepth)*dataset->numDepths;
		//int aboveLayer = floor((double)interpolatedLayer);
		//int belowLayer = ceil((double)interpolatedLayer);
		//float aboveDepth = dataset->getActualDepthAtLayer(aboveLayer);
		//float belowDepth = dataset->getActualDepthAtLayer(belowLayer);
		//float factor = interpolatedLayer - aboveLayer;
		//float depth = factor*belowDepth + (1-factor)*aboveDepth;
		//return depth;
}

void IllustrativeDyePole::kill()
{
	deleteMe = true;
}

bool IllustrativeDyePole::shouldBeDeleted()
{
	return deleteMe;
}