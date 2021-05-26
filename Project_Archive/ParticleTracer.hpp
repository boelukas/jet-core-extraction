#pragma once
#include "Math.hpp"
#include "Integrator.hpp"
#include "DataLoader_Amira.hpp"
#include "WindFields.hpp"
#include <vector>

class TracingIntegrator : public vIntegrator3f {
public:

	TracingIntegrator(DataLoader_Amira* _dl, BoundingBox3d _borders)
		:vIntegrator3f(_borders),
		dl(_dl)
	{
		WindFields wf = WindFields(dl);
		wind = wf.GetWindDirectionEra(0,dl->Load3dPressure(0), dl->loadRegScalarField3f("U", 0), dl->loadRegScalarField3f("V", 0), dl->loadRegScalarField3f("OMEGA", 0));
	}
	virtual Vec3f sample(const Vec3f& pos) const
	{
		return wind->Sample(pos);
	}
private:
	DataLoader_Amira* dl;
	EraVectorField3f* wind;
	//BoundingBox3d borders;
};
class ParticleTracer
{
public:
	ParticleTracer(DataLoader_Amira* _dl, BoundingBox3d _borders):dl(_dl), borders(_borders) {}
	void traceParticle(Vec3f pos, float timeStep, float endTime, std::vector<Vec3f>& curve) 
	{
		TracingIntegrator ti = TracingIntegrator(dl, borders);
		ti.tangentCurve(pos, timeStep, endTime, curve);

	}
private:
	DataLoader_Amira* dl;
	BoundingBox3d borders;

};
