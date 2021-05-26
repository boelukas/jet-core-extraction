#pragma once

#include "Math.hpp"
#include <algorithm>

static float PI = 3.1415926535897932384626433832795f;

template<typename vin, typename vout>
class Integrator
{
public:
	virtual vout sample(const vin& pos) const = 0;
};

template<typename vec>
class vIntegrator : public Integrator<vec, vec>
{
public:	
	BoundingBox3d borders;
	vIntegrator(BoundingBox3d _borders):borders(_borders){}
	// takes a 4th-order Runge-Kutta step
	vec stepRK4(const vec& pos, float dt) const {

		vec k1 = sample(pos);
		vec k2 = sample(pos + k1 * (dt / 2));
		vec k3 = sample(pos + k2 * (dt / 2));
		vec k4 = sample(pos + k3 * dt);
		return pos + (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6) * dt;
	}

	// takes a step with the selected integrator
	vec step(const vec& pos, float dt) const {
			return stepRK4(pos, dt);
	}

	// traces a tangent curve and returns all particle positions
	void tangentCurve(vec pos, float dt, float tau, std::vector<vec>& curve) const
	{
		curve.clear();
		curve.push_back(pos);
		float t = dt;
		int i = 0;
		while (t <= tau) {
			vec newPos = step(curve[i], dt);
			vec result = newPos;
			for (int i = 0; i < 3; i++) {
				if (newPos[i] < borders.GetMin()[i]) {
					result[i] = borders.GetMin()[i];
				}
				else if(newPos[i] > borders.GetMax()[i]){
					result[i] = borders.GetMax()[i];
				}
			}
			curve.push_back(result);
			t += dt;
			i++;
		}
		float lastStep = tau - (t - dt);
		t = t - dt + lastStep;
		curve.push_back(step(curve[i], lastStep));
	}

};

// ============================================================
// vector field in 2D
class vIntegrator3f : public vIntegrator<Vec3f>
{
public:
	vIntegrator3f(BoundingBox3d box) :vIntegrator(box){}
};