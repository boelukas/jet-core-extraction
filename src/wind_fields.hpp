#pragma once
#include "era_grid.hpp"

class WindFields
{
	/*
		Class for computing wind related fields: The wind direction,  the wind magnitude and the gradient of the wind magnitude.
	*/
public:
	WindFields();

	EraVectorField3f* GetNormalizedWindDirectionEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega);
	EraScalarField3f* GetWindMagnitudeEra(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature);
	EraScalarField3f* GetSmoothWindMagnitude(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature);
	EraVectorField3f* GetWindMagnitudeGradientEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* temperature, RegScalarField3f* windForce);

private:
	RegScalarField3f* Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d);
	Vec2d GetWorldLengthOfDegreeInMeters(const double& lon, const double& lat);
};
