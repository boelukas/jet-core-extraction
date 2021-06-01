#pragma once
#include "EraGrid.hpp"

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
	EraVectorField3f* GetNormalizedWindDirectionEra(const size_t& time, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA);

	/*
		Computes the wind magnitude
	*/
	EraScalarField3f* GetWindMagnitudeEra(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA, RegScalarField3f* T);

	EraScalarField3f* getSmoothWindMagnitude(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA, RegScalarField3f* T);


	EraVectorField3f* GetWindMagnitudeGradientEra(const size_t& time, RegScalarField3f* PS3D, RegScalarField3f* T, RegScalarField3f* windForce);

private:

	RegScalarField3f* Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D);

	/*
		Returns the world length of a position in lon, lat coordinates.
		lon_in meters can become negative due to rounding errors when lat is 90 or -90. Thats why the abs value is returned.
	*/
	Vec2d GetWorldLengthOfDegreeInMeters(const double& lon, const double& lat);
};
