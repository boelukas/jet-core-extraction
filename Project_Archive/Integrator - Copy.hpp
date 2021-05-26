#pragma once
#include "RegularGrid.hpp"
#include "DataLoader_Amira.hpp"

class WindFields
{

private:
	DataLoader_Amira* dl;
public:	

	WindFields(DataLoader_Amira* _dl):dl(_dl){}

	RegVectorField3f* GetWindDirection(int time) 
	{
		RegScalarField3f* U = dl->loadPreprocessedScalarField("U", time);
		RegScalarField3f* V = dl->loadPreprocessedScalarField("V", time);
		RegScalarField3f* OMEGA = dl->loadPreprocessedScalarField("OMEGA", time);
		convertToLongPerS(U, time);
		convertToLatPerS(V, time);

		RegVectorField3f* windDirection = new RegVectorField3f(Vec3i({ U->GetResolution()[0], U->GetResolution()[1], U->GetResolution()[2] }), U->GetDomain());

		size_t numEntries = windDirection->GetResolution()[0] * windDirection->GetResolution()[1] * windDirection->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = windDirection->GetGridCoord(linearIndex);
			windDirection->SetVertexDataAt(coords, Vec3f({ U->GetVertexDataAt(coords),V->GetVertexDataAt(coords), OMEGA->GetVertexDataAt(coords) }));
		}

		delete U;
		delete V;
		delete OMEGA;
		return windDirection;
	}
	/*
		Should get the magnitude of the vector (u,v,w) where all values are in m/s
		Currenty gets the magnitude of the wind direction vector, which is wrong because the values are in(Long/s, Lat/s and Pa/s)
	*/
	RegScalarField3f* GetWindForce(int time)
	{
		/*
		RegScalarField3f* U = dl->loadPreprocessedScalarField("U", time);
		RegScalarField3f* V = dl->loadPreprocessedScalarField("V", time);
		RegScalarField3f* OMEGA = dl->loadPreprocessedScalarField("OMEGA", time);
		RegScalarField3f* T = dl->loadPreprocessedScalarField("T", time);
		*/
		RegVectorField3f* windDirection = GetWindDirection(time);

		RegScalarField3f* normWindVector = new RegScalarField3f(Vec3i({ windDirection->GetResolution()[0], windDirection->GetResolution()[1],windDirection->GetResolution()[2] }), windDirection->GetDomain());
		int numTuples = windDirection->GetResolution()[0] * windDirection->GetResolution()[1] * windDirection->GetResolution()[2];

#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (int linearIndex = 0; linearIndex < numTuples; ++linearIndex)
		{
			Vec3i gridCoord = normWindVector->GetGridCoord(linearIndex);
			Vec3f w = windDirection->GetVertexDataAt(gridCoord);
			float u = w[0];
			float v = w[1];
			float om = w[2];
			normWindVector->SetVertexDataAt(gridCoord, std::sqrt(u * u + v * v + om * om));

		}
		return normWindVector;
	}

	void convertToLatPerS(RegScalarField3f* field, int time) {
		std::vector<float> vecLat = dl->loadFloatArray("lat", time);
		std::vector<float> vecLong = dl->loadFloatArray("lon", time);
		size_t numEntries = field->GetResolution()[0] * field->GetResolution()[1] * field->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = field->GetGridCoord(linearIndex);
			float lon = vecLong[coords[0]];
			float lat = vecLat[coords[1]];
			Vec3d meters = GetWorldLengthOfDegreeInMeters(lon, lat);
			float lat_in_m = meters[1];
			float speed_in_m = field->GetVertexDataAt(coords);
			float speed_in_lat = lat / lat_in_m * speed_in_m;
			field->SetVertexDataAt(coords, speed_in_lat);
		}

	}

	void convertToLongPerS(RegScalarField3f* field, int time) {
		std::vector<float> vecLat = dl->loadFloatArray("lat", time);
		std::vector<float> vecLong = dl->loadFloatArray("lon", time);
		size_t numEntries = field->GetResolution()[0] * field->GetResolution()[1] * field->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = field->GetGridCoord(linearIndex);
			float lon = vecLong[coords[0]];
			float lat = vecLat[coords[1]];
			Vec3d meters = GetWorldLengthOfDegreeInMeters(lon, lat);
			float lon_in_m = meters[0];
			float speed_in_m = field->GetVertexDataAt(coords);
			float speed_in_lon = lon / lon_in_m * speed_in_m;
			field->SetVertexDataAt(coords, speed_in_lon);
		}
	}

	Vec2d GetWorldLengthOfDegreeInMeters(double lon, double lat)
	{
		float pi = 3.1415926535897932384626433832795f;
		double phi = lat * pi / 180.0;	// geometric latitude in radiants
		double lat_in_meters = 111132.92 - 559.82 * cos(2 * phi) + 1.175 * cos(4 * phi) - 0.0023 * cos(6 * phi);
		double lon_in_meters = 111412.84 - cos(phi) - 93.5 * cos(3 * phi) + 0.118 * cos(5 * phi);
		return Vec2d({ lon_in_meters, lat_in_meters });
	}

	void Convert_pa_to_m(RegScalarField3f* data, RegScalarField3f* temp, RegScalarField3f* PS) {

		size_t numEntries = data->GetResolution()[0] * data->GetResolution()[1] * data->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = data->GetGridCoord(linearIndex);

			float omega_pa = data->GetVertexDataAt(coords);
			float t = temp->GetVertexDataAt(coords);
			float p = PS->GetVertexDataAt(coords);

			float rgas = 287.058; //J / (kg - K) = > m2 / (s2 K)
			float g = 9.80665;// m / s2
			float rho = p / (rgas * t); //density = > kg / m3
			float w = -omega_pa / (rho * g); //array operation
			data->SetVertexDataAt(coords, w);
		}

	}

	

};
