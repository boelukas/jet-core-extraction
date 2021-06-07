#pragma once
#include "regular_grid.hpp"

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
	Vec3f GradientAtGridCoord(const Vec3i& coord, RegScalarField3f* field, RegScalarField3f* temp) const {
		Vec3i clamped_grid_coords = ClampToPeriodicSphereGrid(coord, field->GetResolution());
		Vec3d domain_coords = field->GetCoordAtWithInvertedZDomain(clamped_grid_coords);
		Vec2f domain_coords_in_m = GetWorldLengthOfDegreeInMeters(domain_coords[1]);
		Vec3f step_sizes = field->GetVoxelSize();

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
			gradient[0] = (data_plus - data_minus) / (2.f * step_sizes[0]);


			grid_coords_plus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,1,0 }), field->GetResolution());
			grid_coords_minus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,-1,0 }), field->GetResolution());
			data_plus = field->GetVertexDataAt(grid_coords_plus);
			data_minus = field->GetVertexDataAt(grid_coords_minus);
			gradient[1] = (data_plus - data_minus) / (2.f * step_sizes[1]);

		}

		//Compute pressure gradient component
		if (clamped_grid_coords[2] == 0) {
			//Upward differentiation
			Vec3i grid_coord_lev_plus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,1 }), field->GetResolution());
			Vec3i grid_coord_lev_minus = clamped_grid_coords;
			float vertex_data_lev_plus = field->GetVertexDataAt(grid_coord_lev_plus);
			float vertex_data_lev_minus = field->GetVertexDataAt(grid_coord_lev_minus);
			float size_of_1hpa_in_m = ConvertPaToM(100, temp->GetVertexDataAt(grid_coord_lev_minus), 1010 - grid_coord_lev_minus[2] * 10);
			gradient[2] = (vertex_data_lev_plus - vertex_data_lev_minus) / (step_sizes[2]);

		}
		else if (clamped_grid_coords[2] == field->GetResolution()[1] - 1) {
			//Downward differeintiation
			Vec3i grid_coord_lev_plus = clamped_grid_coords;
			Vec3i grid_coord_lev_minus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,-1 }), field->GetResolution());
			float vertex_ata_lev_plus = field->GetVertexDataAt(grid_coord_lev_plus);
			float vertex_data_lev_minus = field->GetVertexDataAt(grid_coord_lev_minus);
			float size_of_1hpa_in_m = ConvertPaToM(100, temp->GetVertexDataAt(grid_coord_lev_minus), 1010 - grid_coord_lev_minus[2] * 10);
			gradient[2] = (vertex_ata_lev_plus - vertex_data_lev_minus) / (step_sizes[2]);

		}
		else {
			//Central Differentiation
			Vec3i grid_coord_lev_plus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,1 }), field->GetResolution());
			Vec3i grid_coord_lev_minus = ClampToPeriodicSphereGrid(clamped_grid_coords + Vec3i({ 0,0,-1 }), field->GetResolution());
			float vertex_ata_lev_plus = field->GetVertexDataAt(grid_coord_lev_plus);
			float vertex_data_lev_minus = field->GetVertexDataAt(grid_coord_lev_minus);
			float size_of_1hpa_in_m = ConvertPaToM(100, temp->GetVertexDataAt(grid_coord_lev_minus), 1010 - grid_coord_lev_minus[2] * 10);
			gradient[2] = (vertex_ata_lev_plus - vertex_data_lev_minus) / (2.f * step_sizes[2]);

		}
		return gradient;
	}
	/*
		Computes the gradient of the source_field and returns it as Vector Field.
	*/
	RegVectorField3f* GradientVectorField(RegScalarField3f* source_field, RegScalarField3f* temp) const {
		RegVectorField3f* gradient = new RegVectorField3f(Vec3i({ source_field->GetResolution()[0], source_field->GetResolution()[1], source_field->GetResolution()[2] }), source_field->GetDomain());
		size_t num_entries = (size_t)gradient->GetResolution()[0] * (size_t)gradient->GetResolution()[1] * (size_t)gradient->GetResolution()[2];

#pragma omp parallel for schedule(dynamic,16)
		for (long long linear_index = 0; linear_index < num_entries; linear_index++) {
			Vec3i coords = gradient->GetGridCoord(linear_index);
			gradient->SetVertexDataAt(coords, GradientAtGridCoord(coords, source_field, temp));
		}
		return gradient;
	}
	/*
		Computes the normalized gradient of the source_field and returns it as Vector Field.
	*/
	RegVectorField3f* NormalizedGradientVectorField(RegScalarField3f* source_field, RegScalarField3f* temp) const {
		RegVectorField3f* gradient = new RegVectorField3f(Vec3i({ source_field->GetResolution()[0], source_field->GetResolution()[1], source_field->GetResolution()[2] }), source_field->GetDomain());
		size_t num_entries = (size_t)gradient->GetResolution()[0] * (size_t)gradient->GetResolution()[1] * (size_t)gradient->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
		for (long long linear_index = 0; linear_index < num_entries; linear_index++) {
			Vec3i coords = gradient->GetGridCoord(linear_index);
			Vec3f local_grad = GradientAtGridCoord(coords, source_field, temp);
			gradient->SetVertexDataAt(coords, local_grad / local_grad.length());
		}
		return gradient;
	}

	/*
		Computes the magnitude of the gradient of the source_field. Uses the l1 norm.
	*/
	RegScalarField3f* GradientMagnitude(RegScalarField3f* source_field, RegScalarField3f* temp) const {
		RegScalarField3f* mag = new RegScalarField3f(Vec3i({ source_field->GetResolution()[0], source_field->GetResolution()[1], source_field->GetResolution()[2] }), source_field->GetDomain());
		size_t num_entries = (size_t)mag->GetResolution()[0] * (size_t)mag->GetResolution()[1] * (size_t)mag->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
		for (long long linear_index = 0; linear_index < num_entries; linear_index++) {
			Vec3i coords = mag->GetGridCoord(linear_index);
			Vec3f local_grad = GradientAtGridCoord(coords, source_field, temp);
			mag->SetVertexDataAt(coords, local_grad.length());
		}
		return mag;
	}

	/*
		Converts pa to m.
	*/
	float ConvertPaToM(const float& pa_to_convert, const float& temp, const float& height_in_hPa) const {
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
		1) -n_lat < coord[1] < 2*n_lat - 2
		2) -n_lon <= coord[0]
	*/
	Vec2i ClampToPeriodicSphereGrid(const Vec2i& grid_coord, const Vec2i& resolution) const {
		Vec2i grid_coord_clamped = grid_coord;
		int n_lon = resolution[0];
		int n_lat = resolution[1];

		if (grid_coord[1] >= n_lat) {
			grid_coord_clamped[1] = 2 * (n_lat - 1) - grid_coord[1];
			// We go on the different side of the ball down. Thats why we need to shift the lon coordinate.
			grid_coord_clamped[0] = grid_coord[0] + n_lon / 2;
		}
		else if (grid_coord[1] < 0) {
			grid_coord_clamped[1] = -grid_coord[1];
			grid_coord_clamped[0] = grid_coord[0] + n_lon / 2; // ??
		}
		grid_coord_clamped[0] = (grid_coord_clamped[0] + n_lon) % n_lon;

		return grid_coord_clamped;
	}

	Vec3i ClampToPeriodicSphereGrid(const Vec3i& grid_coord, const Vec3i& resolution) const {
		int n_lev = resolution[2];
		Vec2i lon_lat_clamped = ClampToPeriodicSphereGrid(Vec2i({ grid_coord[0], grid_coord[1] }),
			Vec2i({ resolution[0], resolution[1] }));
		Vec3i grid_coord_clamped = grid_coord;
		grid_coord_clamped[0] = lon_lat_clamped[0];
		grid_coord_clamped[1] = lon_lat_clamped[1];
		grid_coord_clamped[2] = std::min(std::max(0, grid_coord[2]), n_lev - 1);

		return grid_coord_clamped;
	}
};
