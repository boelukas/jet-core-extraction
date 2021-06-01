#pragma once
#include "EraGrid.hpp"
#include "LineCollection.hpp"
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
	static RegScalarField3f* loadRegScalarField3f(const std::string& fieldName, const size_t& time);
	static std::vector<RegScalarField3f*> loadScalarFields(const size_t& time, const std::vector<std::string>& fieldNames);
	static void loadLineCollection(const std::string& name, const size_t& time, const std::vector<std::string>& attributes, LineCollection& lines);
	static RegScalarField3f* compute_PS3D(const std::string& time, const Vec3i& resolution, const BoundingBox3d& domain);

	//Getters
	static std::string getSrcPath();
	static std::string getPreprocPath();
	static std::string getDataStartDate();
	static std::vector<std::string> collectTimes();
};
