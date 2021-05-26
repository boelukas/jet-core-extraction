#pragma once
#include "RegularGrid.hpp"

class Gradient
{
	/*
		Class for computing gradients of scalar fields. Also converts pa to m.
	*/
public:

	Gradient() {}
	/*
		Computes the gradient at position coord in the grid.
	*/
	Vec3f gradientAtGridCoord(const Vec3i& coord, RegScalarField3f* field, RegScalarField3f* temp) const {
		Vec3i clamped_grid_coords = ClampToPeriodicSphereGrid(coord, field->GetResolution());
		Vec3d domain_coords = field->GetCoordAt_InvertedZDomain(clamped_grid_coords);
		Vec2f domain_coords_in_m = GetWorldLengthOfDegreeInMeters(domain_coords[1]);
		Vec3f stepSizes = field->GetVoxelSize();

		Vec3f gradient;

		// Border case north and south pole: gradient is 0
		if (coord[1] == 0 || coord[1] == field->GetResolution()[1] - 1) {
			gradient = Vec3f({ 0, 0, 0 });
			return gradient;
		}
		else {
			Vec3i grid_coords_plus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 1,0,0 }), field->GetResolution());
			Vec3i grid_coords_minus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ -1,0,0 }), field->GetResolution());
			float data_plus = field->GetVertexDataAt(grid_coords_plus);
			float data_minus = field->GetVertexDataAt(grid_coords_minus);
			//gradient[0] = (data_plus - data_minus) / (2.f * stepSizes[0] * domain_coords_in_m[0]);
			gradient[0] = (data_plus - data_minus) / (2.f * stepSizes[0]);


			grid_coords_plus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,1,0 }), field->GetResolution());
			grid_coords_minus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,-1,0 }), field->GetResolution());
			data_plus = field->GetVertexDataAt(grid_coords_plus);
			data_minus = field->GetVertexDataAt(grid_coords_minus);
			//gradient[1] = (data_plus - data_minus) / (2.f * stepSizes[1] * domain_coords_in_m[1]);
			gradient[1] = (data_plus - data_minus) / (2.f * stepSizes[1]);

		}

		//Compute pressure gradient component
		if (clamped_grid_coords[2] == 0) {
			//Upward differentiation
			Vec3i gridCoordLevPlus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,1 }), field->GetResolution());
			Vec3i gridCoordLevMinus = clamped_grid_coords;
			float vertexDataLevPlus = field->GetVertexDataAt(gridCoordLevPlus);
			float vertexDataLevMinus = field->GetVertexDataAt(gridCoordLevMinus);
			float size_of_1hpa_in_m = Convert_pa_to_m(100, temp->GetVertexDataAt(gridCoordLevMinus), 1010 - gridCoordLevMinus[2] * 10);
			//gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (stepSizes[2] * size_of_1hpa_in_m);
			gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (stepSizes[2]);

		}
		else if (clamped_grid_coords[2] == field->GetResolution()[1] - 1) {
			//Downward differeintiation
			Vec3i gridCoordLevPlus = clamped_grid_coords;
			Vec3i gridCoordLevMinus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,-1 }), field->GetResolution());
			float vertexDataLevPlus = field->GetVertexDataAt(gridCoordLevPlus);
			float vertexDataLevMinus = field->GetVertexDataAt(gridCoordLevMinus);
			float size_of_1hpa_in_m = Convert_pa_to_m(100, temp->GetVertexDataAt(gridCoordLevMinus), 1010 - gridCoordLevMinus[2] * 10);
			//gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (stepSizes[2]*size_of_1hpa_in_m);
			gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (stepSizes[2]);

		}
		else {
			//Central Differentiation
			Vec3i gridCoordLevPlus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,1 }), field->GetResolution());
			Vec3i gridCoordLevMinus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,-1 }), field->GetResolution());
			float vertexDataLevPlus = field->GetVertexDataAt(gridCoordLevPlus);
			float vertexDataLevMinus = field->GetVertexDataAt(gridCoordLevMinus);
			float size_of_1hpa_in_m = Convert_pa_to_m(100, temp->GetVertexDataAt(gridCoordLevMinus), 1010 - gridCoordLevMinus[2] * 10);
			//gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (2.f * stepSizes[2] * size_of_1hpa_in_m);
			gradient[2] = (vertexDataLevPlus - vertexDataLevMinus) / (2.f * stepSizes[2]);

		}
		return gradient;
	}
	/*
		Computes the gradient of the sourceField and returns it as Vector Field.
	*/
	RegVectorField3f* gradientVectorField(RegScalarField3f* sourceField, RegScalarField3f* temp) const {
		RegVectorField3f* gradient = new RegVectorField3f(Vec3i({ sourceField->GetResolution()[0], sourceField->GetResolution()[1], sourceField->GetResolution()[2] }), sourceField->GetDomain());
		size_t numEntries = (size_t)gradient->GetResolution()[0] * (size_t)gradient->GetResolution()[1] * (size_t)gradient->GetResolution()[2];

#pragma omp parallel for schedule(dynamic,16)
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = gradient->GetGridCoord(linearIndex);
			gradient->SetVertexDataAt(coords, gradientAtGridCoord(coords, sourceField, temp));
		}
		return gradient;
	}
	/*
		Computes the normalized gradient of the sourceField and returns it as Vector Field.
	*/
	RegVectorField3f* normalizedGradientVectorField(RegScalarField3f* sourceField, RegScalarField3f* temp) const {
		RegVectorField3f* gradient = new RegVectorField3f(Vec3i({ sourceField->GetResolution()[0], sourceField->GetResolution()[1], sourceField->GetResolution()[2] }), sourceField->GetDomain());
		size_t numEntries = (size_t)gradient->GetResolution()[0] * (size_t)gradient->GetResolution()[1] * (size_t)gradient->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = gradient->GetGridCoord(linearIndex);
			Vec3f localGrad = gradientAtGridCoord(coords, sourceField, temp);
			gradient->SetVertexDataAt(coords, localGrad / localGrad.length());
		}
		return gradient;
	}

	/*
		Computes the magnitude of the gradient of the sourceField. Uses the l1 norm.
	*/
	RegScalarField3f* gradientMagnitude(RegScalarField3f* sourceField, RegScalarField3f* temp) const {
		RegScalarField3f* mag = new RegScalarField3f(Vec3i({ sourceField->GetResolution()[0], sourceField->GetResolution()[1], sourceField->GetResolution()[2] }), sourceField->GetDomain());
		size_t numEntries = (size_t)mag->GetResolution()[0] * (size_t)mag->GetResolution()[1] * (size_t)mag->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			Vec3i coords = mag->GetGridCoord(linearIndex);
			Vec3f localGrad = gradientAtGridCoord(coords, sourceField, temp);
			mag->SetVertexDataAt(coords, localGrad.length());
		}
		return mag;
	}

	/*
		Converts pa to m.
	*/
	float Convert_pa_to_m(const float& pa_to_convert, const float& temp, const float& height_in_hPa) const {
		float omega_pa = pa_to_convert;
		float t = temp;
		float p = height_in_hPa * 100; // because the values are in hPa

		float rgas = 287.058; //J / (kg - K) = > m2 / (s2 K)
		float g = 9.80665;// m / s2
		float rho = p / (rgas * t); //density = > kg / m3
		float w = omega_pa / (rho * g); //array operation
		return w;
	}
private:

	//Helper Functions
	Vec2d GetWorldLengthOfDegreeInMeters(const double& lat) const {
		float pi = 3.1415926535897932384626433832795f;
		double phi = lat * pi / 180.0;	// geometric latitude in radiants
		double lat_in_meters = 111132.92 - 559.82 * cos(2 * phi) + 1.175 * cos(4 * phi) - 0.0023 * cos(6 * phi);
		double lon_in_meters = 111412.84 * cos(phi) - 93.5 * cos(3 * phi) + 0.118 * cos(5 * phi);
		return Vec2d({ std::abs(lon_in_meters), lat_in_meters });
	}

	/*
		Clamps the grid coord to the bounding box.
		Assumption:
		1) -nlat < coord[1] < 2*nlat - 2
		2) -nlon <= coord[0]
	*/
	Vec2i ClampToPeriodicSphereGrid(const Vec2i& gridCoord, const Vec2i& resolution) const {
		Vec2i gridCoordClamped = gridCoord;
		int nlon = resolution[0];
		int nlat = resolution[1];

		if (gridCoord[1] >= nlat) {
			gridCoordClamped[1] = 2 * (nlat - 1) - gridCoord[1];
			// We go on the different side of the ball down. Thats why we need to shift the lon coordinate.
			gridCoordClamped[0] = gridCoord[0] + nlon / 2;
		}
		else if (gridCoord[1] < 0) {
			gridCoordClamped[1] = -gridCoord[1];
			gridCoordClamped[0] = gridCoord[0] + nlon / 2; // ??
		}
		gridCoordClamped[0] = (gridCoordClamped[0] + nlon) % nlon;

		return gridCoordClamped;
	}

	Vec3i ClampToPeriodicSphereGrid(const Vec3i& gridCoord, const Vec3i& resolution) const {
		int nlev = resolution[2];
		Vec2i lonLatClamped = ClampToPeriodicSphereGrid(Vec2i({ gridCoord[0], gridCoord[1] }),
			Vec2i({ resolution[0], resolution[1] }));
		Vec3i gridCoordClamped = gridCoord;
		gridCoordClamped[0] = lonLatClamped[0];
		gridCoordClamped[1] = lonLatClamped[1];
		gridCoordClamped[2] = std::min(std::max(0, gridCoord[2]), nlev - 1);

		return gridCoordClamped;
	}
	float l2norm(const Vec3f& v) const {
		return std::sqrt(std::pow(v[0], 2) + std::pow(v[1], 2) + std::pow(v[2], 2));
	}


};
