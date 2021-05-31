#include "jet-core-extraction/Gradient.hpp"

#include "jet-core-extraction/WindFields.hpp"

WindFields::WindFields() {}

EraVectorField3f* WindFields::GetNormalizedWindDirectionEra(const size_t& time, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA) {

	RegVectorField3f* windDirection = new RegVectorField3f(Vec3i({ U->GetResolution()[0], U->GetResolution()[1], U->GetResolution()[2] }), U->GetDomain());

	size_t numEntries = (size_t)windDirection->GetResolution()[0] * (size_t)windDirection->GetResolution()[1] * (size_t)windDirection->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = windDirection->GetGridCoord(linearIndex);
		Vec3f v = Vec3f({ U->GetVertexDataAt(coords),V->GetVertexDataAt(coords), OMEGA->GetVertexDataAt(coords) / 100 });
		Vec3f v_n = v / v.length();
		windDirection->SetVertexDataAt(coords, v_n);
	}

	EraVectorField3f* era = new EraVectorField3f(windDirection, PS3D);
	return era;
}

EraScalarField3f* WindFields::GetWindMagnitudeEra(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA, RegScalarField3f* T) {

	RegScalarField3f* W = Convert_pa_to_m(OMEGA, T, time, psAxisValues, PS3D);

	BoundingBox3d dom = U->GetDomain();

	RegScalarField3f* normWindVector = new RegScalarField3f(Vec3i({ U->GetResolution()[0], U->GetResolution()[1],U->GetResolution()[2] }), U->GetDomain());
	int numTuples = normWindVector->GetResolution()[0] * normWindVector->GetResolution()[1] * normWindVector->GetResolution()[2];

#pragma omp parallel for schedule(dynamic,16)
	for (int linearIndex = 0; linearIndex < numTuples; ++linearIndex)
	{
		Vec3i gridCoord = normWindVector->GetGridCoord(linearIndex);
		float u = U->GetVertexDataAt(gridCoord);
		float v = V->GetVertexDataAt(gridCoord);
		float om = W->GetVertexDataAt(gridCoord);
		normWindVector->SetVertexDataAt(gridCoord, std::sqrt(u * u + v * v + om * om));

	}
	EraScalarField3f* era = new EraScalarField3f(normWindVector, PS3D);

	delete W;
	return era;
}

EraScalarField3f* WindFields::getSmoothWindMagnitude(const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D, RegScalarField3f* U, RegScalarField3f* V, RegScalarField3f* OMEGA, RegScalarField3f* T) {
	EraScalarField3f* windMagEra = GetWindMagnitudeEra(time, psAxisValues, PS3D, U, V, OMEGA, T);
	RegScalarField3f* field = windMagEra->GetField();
	RegScalarField3f* smooth = new RegScalarField3f(field->GetResolution(), field->GetDomain());

	int numTuples = field->GetResolution()[0] * field->GetResolution()[1] * field->GetResolution()[2];
	int x_filter = 3;
	int y_filter = 3;
	int z_filter = 1;

#pragma omp parallel for schedule(dynamic,16)

	for (int linearIndex = 0; linearIndex < numTuples; ++linearIndex)
	{
		Vec3i gridCoord = field->GetGridCoord(linearIndex);

		double avg = 0.0;
		for (int i = -x_filter; i <= x_filter; i++) {
			for (int j = -y_filter; j <= y_filter; j++) {
				for (int k = -z_filter; k <= z_filter; k++) {
					int x = gridCoord[0] + i;
					int y = gridCoord[1] + j;
					int z = gridCoord[2] + k;
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

		smooth->SetVertexDataAt(gridCoord, avg / ((2 * x_filter + 1) * (2 * y_filter + 1) * (2 * z_filter + 1)));

	}
	delete field;
	delete windMagEra;
	return new EraScalarField3f(smooth, PS3D);;
}


EraVectorField3f* WindFields::GetWindMagnitudeGradientEra(const size_t& time, RegScalarField3f* PS3D, RegScalarField3f* T, RegScalarField3f* windForce) {
	Gradient g = Gradient();
	RegVectorField3f* grad = g.gradientVectorField(windForce, T);
	EraVectorField3f* era = new EraVectorField3f(grad, PS3D);
	return era;
}


void WindFields::convertToLatPerS(RegScalarField3f* field, const size_t& time, const std::vector<float>& vecLat, const std::vector<float>& vecLon) {
	size_t numEntries = (size_t)field->GetResolution()[0] * (size_t)field->GetResolution()[1] * (size_t)field->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = field->GetGridCoord(linearIndex);
		float lon = vecLon[coords[0]];
		float lat = vecLat[coords[1]];
		Vec2d meters = GetWorldLengthOfDegreeInMeters(lon, lat);
		float lat_in_m = meters[1];
		float speed_in_m = field->GetVertexDataAt(coords);
		float speed_in_lat = 1. / lat_in_m * speed_in_m;
		field->SetVertexDataAt(coords, speed_in_lat);
	}

}


void WindFields::convertToLongPerS(RegScalarField3f* field, const size_t& time, const std::vector<float>& vecLat, const std::vector<float>& vecLon) {
	size_t numEntries = (size_t)field->GetResolution()[0] * (size_t)field->GetResolution()[1] * (size_t)field->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = field->GetGridCoord(linearIndex);
		float lon = vecLon[coords[0]];
		float lat = vecLat[coords[1]];
		if (lat == -90 || lat == 90) { field->SetVertexDataAt(coords, 0); continue; }

		Vec2d meters = GetWorldLengthOfDegreeInMeters(lon, lat);
		float lon_in_m = meters[0];
		float speed_in_m = field->GetVertexDataAt(coords);
		float speed_in_lon = 1. / lon_in_m * speed_in_m;
		field->SetVertexDataAt(coords, speed_in_lon);
	}
}


Vec2d WindFields::GetWorldLengthOfDegreeInMeters(const double& lon, const double& lat)
{
	float pi = 3.1415926535897932384626433832795f;
	double phi = lat * pi / 180.0;	// geometric latitude in radiants
	double lat_in_meters = 111132.92 - 559.82 * cos(2 * phi) + 1.175 * cos(4 * phi) - 0.0023 * cos(6 * phi);
	double lon_in_meters = 111412.84 * cos(phi) - 93.5 * cos(3 * phi) + 0.118 * cos(5 * phi);
	return Vec2d({ std::abs(lon_in_meters), lat_in_meters });
}


RegScalarField3f* WindFields::Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, const size_t& time, const std::vector<float>& psAxisValues, RegScalarField3f* PS3D) {
	RegScalarField3f* W = new RegScalarField3f(data->GetResolution(), data->GetDomain());
	size_t numEntries = (size_t)data->GetResolution()[0] * (size_t)data->GetResolution()[1] * (size_t)data->GetResolution()[2];
	float max = 0;
#pragma omp parallel for schedule(dynamic,16)
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = data->GetGridCoord(linearIndex);

		float omega_pa = data->GetVertexDataAt(coords);
		float t = temp->GetVertexDataAt(coords);
		float p = PS3D->GetVertexDataAt(coords) * 100;
		float rgas = 287.058; //J / (kg - K) = > m2 / (s2 K)
		float g = 9.80665;// m / s2
		float rho = p / (rgas * t); //density = > kg / m3
		float w = -omega_pa / (rho * g); //array operation
		W->SetVertexDataAt(coords, w);
		if (std::abs(w) > max) { max = std::abs(w); }
	}
	return W;
}
