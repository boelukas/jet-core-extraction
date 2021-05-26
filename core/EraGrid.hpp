#pragma once
#include "RegularGrid.hpp"

template<typename TValueType>
class EraGrid {
public:
	using TDomainCoord = Vec<double, 3>;

	EraGrid(RegularGrid<TValueType, 3>* field, RegScalarField3f* ps) :
		mField(field),
		mPs(ps)
	{
	}
	/*
		Samples the 3dPressure at position coord, which can be a non grid point location, with double indeces.
		It returns the interpolated value of mField for the level at (coor[0], coord[1]) at which the pressure is equal to seachedPS = coord[3].
	*/
	virtual TValueType Sample(const TDomainCoord& coord) const
	{
		double i = coord[0];
		double j = coord[1];
		double k = coord[2];
		int i_down = (int)std::floor(i);
		i_down = std::min(std::max(0, i_down), mField->GetResolution()[0] - 1);
		int i_up = (int)std::ceil(i);
		i_up = std::min(std::max(0, i_up), mField->GetResolution()[0] - 1);
		int j_down = (int)std::floor(j);
		j_down = std::min(std::max(0, j_down), mField->GetResolution()[1] - 1);
		int j_up = (int)std::ceil(j);
		j_up = std::min(std::max(0, j_up), mField->GetResolution()[1] - 1);

		TValueType v_id_jd = binarySearch(i_down, j_down, k);
		TValueType v_iu_jd = binarySearch(i_up, j_down, k);
		TValueType v_id_ju = binarySearch(i_down, j_up, k);
		TValueType v_iu_ju = binarySearch(i_up, j_up, k);

		TValueType v_frontFace = linearInterpolate(i_down, i_up, v_id_jd, v_iu_jd, i);
		TValueType v_rearFace = linearInterpolate(i_down, i_up, v_id_ju, v_iu_ju, i);

		TValueType v = linearInterpolate(j_down, j_up, v_frontFace, v_rearFace, j);

		return v;
	}
	/*
		Takes a RegularGrid with axis (long, lat, level) and returns a new grid with axis (long, lat, pressure[hPa]).
	*/
	void Resample(RegularGrid<TValueType, 3>& output) const
	{
		std::vector<float> psAxisValues = GetPressureAxisValues(output);

		size_t numEntries = (size_t)output.GetResolution()[0] * output.GetResolution()[1] * output.GetResolution()[2];

		//#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
//#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			//typename RegularGrid<TValueType, 3>::TGridCoord coords = mPs->GetGridCoord(linearIndex); //TGridCoord
			Vec3i coords = output.GetGridCoord(linearIndex);
			int i = coords[0];
			int j = coords[1];
			int k = coords[2];

			TValueType val = Sample(Vec3f({ (float)i, (float)j, psAxisValues[k] }));

			output.SetVertexDataAt(Vec3i({ i, j, output.GetResolution()[2] - 1 - k }), val);
		}

	}
	void Resample_Amira(RegularGrid<TValueType, 3>& output, const double& ps_step_size) const
	{
		double ps_domain_min = output.GetDomain().GetMin()[2];
		int ps_resolution = output.GetResolution()[2];

		std::vector<float> psAxisValues = GetPressureAxisValues_AmiraPreprocessing(ps_domain_min, ps_step_size, ps_resolution);

		size_t numEntries = (size_t)output.GetResolution()[0] * output.GetResolution()[1] * output.GetResolution()[2];

		//#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
//#endif
		for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
			//typename RegularGrid<TValueType, 3>::TGridCoord coords = mPs->GetGridCoord(linearIndex); //TGridCoord
			Vec3i coords = output.GetGridCoord(linearIndex);
			int i = coords[0];
			int j = coords[1];
			int k = coords[2];

			TValueType val = Sample(Vec3f({ (float)i, (float)j, psAxisValues[k] }));

			output.SetVertexDataAt(Vec3i({ i, j, output.GetResolution()[2] - 1 - k }), val);
		}

	}
	/*
		Returns the values of the pressure axis as float vector. The number of samples is specified by the dimension[2] of the output grid.
		To get a higher resolution, just increase the dimension[2] value in the output.
	*/
	std::vector<float> GetPressureAxisValues(const RegularGrid<TValueType, 3>& field) const
	{
		int numSamples = field.GetResolution()[2];
		std::vector<float> psAxisValues;
		psAxisValues.reserve(numSamples);
		double minPS = mPs->GetScalarRange()[0];
		double maxPS = mPs->GetScalarRange()[1];

		int roundedStep = std::round((maxPS - minPS) / numSamples);

		for (int i = 0; i < field.GetResolution()[2]; i++) {
			psAxisValues.push_back((i + 1) * roundedStep);
		}
		return psAxisValues;

	}
	std::vector<float> GetPressureAxisValues_AmiraPreprocessing(const double& domain_min, const double& step, const int& resolution) const
	{
		std::vector<float> psAxisValues;
		psAxisValues.reserve(resolution);
		double minPS = domain_min;
		psAxisValues.push_back(minPS);
		for (int i = 1; i < resolution; i++) {
			psAxisValues.push_back(minPS + i * step);
		}
		return psAxisValues;
	}

	void SetField(RegularGrid<TValueType, 3>* field)const {
		mField = field;
	}
	RegularGrid<TValueType, 3>* GetField()const {
		return mField;
	}
	RegScalarField3f* Get3DPS() const {
		return mPs;
	}
private:
	RegularGrid<TValueType, 3>* mField;
	RegScalarField3f* mPs;

	/*
		Returns the value of mField at level k^. Where k^ is the level where the 3D pressure is equal to searchedPS at the grid coordinates(i,j).
	*/
	TValueType binarySearch(const int& i, const int& j, const double& searchedPS) const {
		//search level for which at position (i,j) the 3d Pressure is searchedPS
		double level = 0.;
		int l = 0;
		int r = mPs->GetResolution()[2] - 1;
		//Border Cases: if there is no pressure p at position (i,j) with min_p <= p <= max_p where min_p and max_p are the smallest and largest value in this column
		//we set level to 0 or to the max level.
		if (searchedPS <= mPs->GetVertexDataAt(Vec3i({ i, j, 0 }))) {
			level = 0.0;
		}
		else if (searchedPS >= mPs->GetVertexDataAt(Vec3i({ i, j, mPs->GetResolution()[2] - 1 }))) {
			level = mPs->GetResolution()[2] - 1;
		}
		else {
			//Binary search with integrated linear interpolation
			while (l < r) {
				int m = l + (r - l) / 2;
				float psAtm = mPs->GetVertexDataAt(Vec3i({ i, j, m }));
				if (psAtm == searchedPS) {
					level = m;
					break;
				}
				if (l + 1 == r) {
					float psl = mPs->GetVertexDataAt(Vec3i({ i, j, l }));
					float psr = mPs->GetVertexDataAt(Vec3i({ i, j, r }));

					level = l + ((searchedPS - psl) / (psr - psl));
					break;
				}
				if (psAtm < searchedPS) {
					l = m;
				}
				else {
					r = m;
				}
			}
		}

		int lev_down = std::floor(level);
		int lev_up = std::ceil(level);

		TValueType val_down = mField->GetVertexDataAt(Vec3i({ i, j, lev_down }));
		TValueType val_up = mField->GetVertexDataAt(Vec3i({ i, j, lev_up }));

		TValueType interpol_value = linearInterpolate(lev_down, lev_up, val_down, val_up, level);

		return interpol_value;
	}
	/*
		Performs linear interpolation with the assumption that x2-x1 = 1.
	*/
	TValueType linearInterpolate(const double& x1, const double& x2, const TValueType& y1, const TValueType& y2, double x) const {
		return (y1 + (y2 - y1) * (x - x1));
	}
};

typedef EraGrid<Vec3f> EraVectorField3f;
typedef EraGrid<float> EraScalarField3f;
