#pragma once
#include "era_grid.hpp"
#include "line_collection.hpp"
#include <string.h>

class DataHelper
{
public:

	//Data loading functions
	static RegScalarField3f* LoadRegScalarField3f(const std::string& field_name, const size_t& time);
	static std::vector<RegScalarField3f*> LoadScalarFields(const size_t& time, const std::vector<std::string>& field_names);
	static RegScalarField3f* ComputePS3D(const std::string& time, const Vec3i& resolution, const BoundingBox3d& domain);

	//Getters
	static std::string GetSrcPath();
	static std::string GetPreprocPath();
	static std::string GetDataStartDate();
	static std::vector<std::string> CollectTimes();
  static std::vector<float> GetPsAxis();
};
