#pragma once
#include "regular_grid.hpp"

template<typename TValueType>
class EraGrid {
public:
	using TDomainCoord = Vec<double, 3>;

	EraGrid(RegularGrid<TValueType, 3>* field, RegScalarField3f* ps) :
		field_(field),
		ps_(ps)
	{
	}
	/*
		Samples the 3dPressure at position coord, which can be a non grid point location, with double indeces.
		It returns the interpolated value of field_ for the level at (coor[0], coord[1]) at which the pressure is equal to seachedPS = coord[3].
	*/
	virtual TValueType Sample(const TDomainCoord& coord) const
	{
		double i = coord[0];
		double j = coord[1];
		double k = coord[2];
		int i_down = (int)std::floor(i);
		i_down = std::min(std::max(0, i_down), field_->GetResolution()[0] - 1);
		int i_up = (int)std::ceil(i);
		i_up = std::min(std::max(0, i_up), field_->GetResolution()[0] - 1);
		int j_down = (int)std::floor(j);
		j_down = std::min(std::max(0, j_down), field_->GetResolution()[1] - 1);
		int j_up = (int)std::ceil(j);
		j_up = std::min(std::max(0, j_up), field_->GetResolution()[1] - 1);

		TValueType v_id_jd = BinarySearch(i_down, j_down, k);
		TValueType v_iu_jd = BinarySearch(i_up, j_down, k);
		TValueType v_id_ju = BinarySearch(i_down, j_up, k);
		TValueType v_iu_ju = BinarySearch(i_up, j_up, k);

		TValueType v_front_face = LinearInterpolate(i_down, i_up, v_id_jd, v_iu_jd, i);
		TValueType v_rear_face = LinearInterpolate(i_down, i_up, v_id_ju, v_iu_ju, i);

		TValueType v = LinearInterpolate(j_down, j_up, v_front_face, v_rear_face, j);

		return v;
	}
	RegularGrid<TValueType, 3>* GetField()const {
		return field_;
	}
private:
	RegularGrid<TValueType, 3>* field_;
	RegScalarField3f* ps_;

	/*
		Returns the value of field_ at level k. Where k is the level where the 3D pressure is equal to searched_ps at the grid coordinates(i,j).
	*/
	TValueType BinarySearch(const int& i, const int& j, const double& searched_ps) const {
		//search level for which at position (i,j) the 3d Pressure is searched_ps
		double level = 0.;
		int l = 0;
		int r = ps_->GetResolution()[2] - 1;
		//Border Cases: if there is no pressure p at position (i,j) with min_p <= p <= max_p where min_p and max_p are the smallest and largest value in this column
		//we set level to 0 or to the max level.
		if (searched_ps <= ps_->GetVertexDataAt(Vec3i({ i, j, 0 }))) {
			level = 0.0;
		}
		else if (searched_ps >= ps_->GetVertexDataAt(Vec3i({ i, j, ps_->GetResolution()[2] - 1 }))) {
			level = ps_->GetResolution()[2] - 1.0;
		}
		else {
			//Binary search with integrated linear interpolation
			while (l < r) {
				int m = l + (r - l) / 2;
				float ps_atm = ps_->GetVertexDataAt(Vec3i({ i, j, m }));
				if (ps_atm == searched_ps) {
					level = m;
					break;
				}
				if (l + 1 == r) {
					double psl = (double)ps_->GetVertexDataAt(Vec3i({ i, j, l }));
					double psr = (double)ps_->GetVertexDataAt(Vec3i({ i, j, r }));

					level = l + ((searched_ps - psl) / (psr - psl));
					break;
				}
				if (ps_atm < searched_ps) {
					l = m;
				}
				else {
					r = m;
				}
			}
		}

		int lev_down = (int)std::floor(level);
		int lev_up = (int)std::ceil(level);

		TValueType val_down = field_->GetVertexDataAt(Vec3i({ i, j, lev_down }));
		TValueType val_up = field_->GetVertexDataAt(Vec3i({ i, j, lev_up }));

		TValueType interpol_value = LinearInterpolate(lev_down, lev_up, val_down, val_up, level);

		return interpol_value;
	}
	/*
		Performs linear interpolation with the assumption that x2-x1 = 1.
	*/
	TValueType LinearInterpolate(const double& x1, const double& x2, const TValueType& y1, const TValueType& y2, double x) const {
		return TValueType(y1 + (y2 - y1) * (float)(x - x1));
	}
};

typedef EraGrid<Vec3f> EraVectorField3f;
typedef EraGrid<float> EraScalarField3f;
