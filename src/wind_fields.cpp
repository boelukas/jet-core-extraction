#include "gradient.hpp"

#include "wind_fields.hpp"

WindFields::WindFields() {}

/*
		Computes the normalized wind direction.
*/
EraVectorField3f* WindFields::GetNormalizedWindDirectionEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega) {

	RegVectorField3f* wind_direction = new RegVectorField3f(Vec3i({ u->GetResolution()[0], u->GetResolution()[1], u->GetResolution()[2] }), u->GetDomain());

	size_t num_entries = (size_t)wind_direction->GetResolution()[0] * (size_t)wind_direction->GetResolution()[1] * (size_t)wind_direction->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
	for (int64_t linear_index = 0; linear_index < (int64_t)num_entries; linear_index++) {
		Vec3i coords = wind_direction->GetGridCoord(linear_index);
		Vec3f wind_dir = Vec3f({ u->GetVertexDataAt(coords),v->GetVertexDataAt(coords), omega->GetVertexDataAt(coords) / 100 });
		Vec3f wind_dir_norm = wind_dir / wind_dir.length();
		wind_direction->SetVertexDataAt(coords, wind_dir_norm);
	}

	EraVectorField3f* era = new EraVectorField3f(wind_direction, ps3d);
	return era;
}

EraScalarField3f* WindFields::GetWindMagnitudeEra(const size_t& time, const std::vector<float>& ps_axis_values, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature) {

	RegScalarField3f* w = Convert_pa_to_m(omega, temperature, time, ps_axis_values, ps3d);

	BoundingBox3d dom = u->GetDomain();

	RegScalarField3f* norm_wind_vector = new RegScalarField3f(Vec3i({ u->GetResolution()[0], u->GetResolution()[1],u->GetResolution()[2] }), u->GetDomain());
	int num_tuples = norm_wind_vector->GetResolution()[0] * norm_wind_vector->GetResolution()[1] * norm_wind_vector->GetResolution()[2];

#pragma omp parallel for schedule(dynamic,16)
	for (int linear_index = 0; linear_index < num_tuples; ++linear_index)
	{
		Vec3i grid_coord = norm_wind_vector->GetGridCoord(linear_index);
		float u_at_grid_coord = u->GetVertexDataAt(grid_coord);
		float v_at_grid_coord = v->GetVertexDataAt(grid_coord);
		float om_at_grid_coord = w->GetVertexDataAt(grid_coord);
		norm_wind_vector->SetVertexDataAt(grid_coord, std::sqrt(u_at_grid_coord * u_at_grid_coord + v_at_grid_coord * v_at_grid_coord + om_at_grid_coord * om_at_grid_coord));
	}
	EraScalarField3f* era = new EraScalarField3f(norm_wind_vector, ps3d);

	delete w;
	return era;
}

EraScalarField3f* WindFields::GetSmoothWindMagnitude(const size_t& time, const std::vector<float>& ps_axis_values, RegScalarField3f* ps3d, RegScalarField3f* u, RegScalarField3f* v, RegScalarField3f* omega, RegScalarField3f* temperature) {
	EraScalarField3f* wind_mag_era = GetWindMagnitudeEra(time, ps_axis_values, ps3d, u, v, omega, temperature);
	RegScalarField3f* field = wind_mag_era->GetField();
	RegScalarField3f* smooth = new RegScalarField3f(field->GetResolution(), field->GetDomain());

	int num_tuples = field->GetResolution()[0] * field->GetResolution()[1] * field->GetResolution()[2];
	int x_filter = 3;
	int y_filter = 3;
	int z_filter = 1;

#pragma omp parallel for schedule(dynamic,16)

	for (int linear_index = 0; linear_index < num_tuples; ++linear_index)
	{
		Vec3i grid_coord = field->GetGridCoord(linear_index);

		double avg = 0.0;
		for (int i = -x_filter; i <= x_filter; i++) {
			for (int j = -y_filter; j <= y_filter; j++) {
				for (int k = -z_filter; k <= z_filter; k++) {
					int x = grid_coord[0] + i;
					int y = grid_coord[1] + j;
					int z = grid_coord[2] + k;
					if (x > 0 && x < field->GetResolution()[0] && y > 0 && y < field->GetResolution()[1] && z > 0 && z < field->GetResolution()[2]) {
						avg += field->GetVertexDataAt(Vec3i({ x, y, z }));
					}
					else {
						x = std::max(0, std::min(field->GetResolution()[0] - 1, x));
						y = std::max(0, std::min(field->GetResolution()[1] - 1, y));
						z = std::max(0, std::min(field->GetResolution()[2] - 1, z));
						avg += field->GetVertexDataAt(Vec3i({ x, y, z }));

					}
				}
			}
		}

		smooth->SetVertexDataAt(grid_coord, (float)(avg / ((2.0 * x_filter + 1) * (2.0 * y_filter + 1) * (2.0 * z_filter + 1))));

	}
	delete field;
	delete wind_mag_era;
	return new EraScalarField3f(smooth, ps3d);;
}


EraVectorField3f* WindFields::GetWindMagnitudeGradientEra(const size_t& time, RegScalarField3f* ps3d, RegScalarField3f* temperature, RegScalarField3f* wind_magnitude) {
	Gradient g = Gradient();
	RegVectorField3f* grad = g.GradientVectorField(wind_magnitude, temperature);
	EraVectorField3f* era = new EraVectorField3f(grad, ps3d);
	return era;
}

/*
		Returns the world length of a position in lon, lat coordinates.
		lon_in meters can become negative due to rounding errors when lat is 90 or -90. Thats why the abs value is returned.
*/
Vec2d WindFields::GetWorldLengthOfDegreeInMeters(const double& lon, const double& lat)
{
	float pi = 3.1415926535897932384626433832795f;
	double phi = lat * pi / 180.0;	// geometric latitude in radiants
	double lat_in_meters = 111132.92 - 559.82 * cos(2 * phi) + 1.175 * cos(4 * phi) - 0.0023 * cos(6 * phi);
	double lon_in_meters = 111412.84 * cos(phi) - 93.5 * cos(3 * phi) + 0.118 * cos(5 * phi);
	return Vec2d({ std::abs(lon_in_meters), lat_in_meters });
}


RegScalarField3f* WindFields::Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, const size_t& time, const std::vector<float>& ps_axis_values, RegScalarField3f* ps3d) {
	RegScalarField3f* w_field = new RegScalarField3f(data->GetResolution(), data->GetDomain());
	size_t num_entries = (size_t)data->GetResolution()[0] * (size_t)data->GetResolution()[1] * (size_t)data->GetResolution()[2];
	float max = 0;
#pragma omp parallel for schedule(dynamic,16)
	for (int64_t linear_index = 0; linear_index < (int64_t)num_entries; linear_index++) {
		Vec3i coords = data->GetGridCoord(linear_index);

		float omega_pa = data->GetVertexDataAt(coords);
		float t = temp->GetVertexDataAt(coords);
		float p = ps3d->GetVertexDataAt(coords) * 100;
		float rgas = 287.058f; //J / (kg - K) = > m2 / (s2 K)
		float g = 9.80665f;// m / s2
		float rho = p / (rgas * t); //density = > kg / m3
		float w = -omega_pa / (rho * g); //array operation
		w_field->SetVertexDataAt(coords, w);
		if (std::abs(w) > max) { max = std::abs(w); }
	}
	return w_field;
}
