#pragma once
#include "era_grid.hpp"

class WindFields
{
	/*
		Class for computing wind related fields: The wind direction,  the wind magnitude and the gradient of the wind magnitude.
	*/
public:

	WindFields();

	/*
		Computes the normalized wind direction with no conversions.
	*/
	EraVectorField3f* GetNormalizedWindDirectionEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega);

	/*
		Computes the wind magnitude
	*/
	EraScalarField3f* GetWindMagnitudeEra(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature);

	EraScalarField3f* GetSmoothWindMagnitude(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature);


	EraVectorField3f* GetWindMagnitudeGradientEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* temperature, RegScalarField3f* windForce);

private:

	RegScalarField3f* Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* ps3d);

	/*
		Returns the world length of a position in lon, lat coordinates.
		lon_in meters can become negative due to rounding errors when lat is 90 or -90. Thats why the abs value is returned.
	*/
	Vec2d GetWorldLengthOfDegreeInMeters(const double& lon, const double& lat);
};
