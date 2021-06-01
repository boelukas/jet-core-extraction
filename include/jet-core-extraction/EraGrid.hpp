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
	RegularGrid<TValueType, 3>* GetField()const {
		return mField;
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
