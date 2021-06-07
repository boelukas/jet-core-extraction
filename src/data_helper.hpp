#pragma once
#include "era_grid.hpp"
#include "line_collection.hpp"
#include <string.h>

class DataHelper
{
	/*
		This class is responsible for loading data. To read data from disc, Amira is used.
		The client is responsible for deleting RegScalarFields and EraScalarFields.
		This class will clean up after imageData.
	*/
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
};
