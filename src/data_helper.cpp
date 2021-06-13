#include <filesystem>
#include <fstream>

#include "line_collection.hpp"
#include "time_helper.hpp"
#include "netcdf.hpp"

#include "data_helper.hpp"
/*
	Loads data from the source directory.
*/
RegScalarField3f* DataHelper::LoadRegScalarField3f(const std::string& field_name, const size_t& time) {
	std::string time_step = "P" + TimeHelper::ConvertHoursToDate(time, GetDataStartDate());
	std::string path = GetSrcPath() + time_step;;
	RegScalarField3f* field = NetCDF::ImportScalarField3f(path, field_name, "lon", "lat", "lev");
	if (field == NULL){
		std::cout << std::endl;
		std::cout << "The following field was not found in the data "<< time_step<<": " << field_name << std::endl;
		std::cout << "exiting" << std::endl;
		std::exit(2);
	}
	return field;
}
/*
	Loads a vector of scalar fields.
*/
std::vector<RegScalarField3f*> DataHelper::LoadScalarFields(const size_t& time, const std::vector<std::string>& field_names) {
	int n_fields = field_names.size();
	std::vector<RegScalarField3f*> fields(n_fields, NULL);

#pragma omp parallel for schedule(dynamic,16)
	for (int i = 0; i < n_fields; i++) {
			fields[i] = LoadRegScalarField3f(field_names[i], time);
	}
	return fields;
}

/*
	Returns the 3D pressure in (lon, lat, level) coordinates.
	Saves the max and min Pressure values in the mScalarRange fields of the Scalar field.
*/
RegScalarField3f* DataHelper::ComputePS3D(const std::string& time, const Vec3i& resolution, const BoundingBox3d& domain) {


	std::string path = GetSrcPath() + "P" + time;

	RegScalarField3f* pressure_3d = new RegScalarField3f(resolution, domain);

	std::vector<float> lev, hyam, hybm;
	if (!NetCDF::ImportFloatArray(path, "lev", lev)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hyam", hyam)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hybm", hybm)) return NULL;

	RegScalarField2f* pressure_2d = NetCDF::ImportScalarField2f(path, "PS", "lon", "lat");
	float min_pressure = 1000000;
	float max_pressure = -1;

	size_t num_entries = (size_t)pressure_3d->GetResolution()[0] * (size_t)pressure_3d->GetResolution()[1] * (size_t)pressure_3d->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
	for (long long linear_index = 0; linear_index < num_entries; linear_index++) {
		Vec3i coords = pressure_3d->GetGridCoord(linear_index);
		int i = coords[0];
		int j = coords[1];
		int k = coords[2];

		float pressure = hyam[(size_t)std::round(lev[k]) - 1] * 0.01 + (double)hybm[(size_t)std::round(lev[k]) - 1] * (double)pressure_2d->GetVertexDataAt(Vec2i({ i, j }));
		if (pressure < min_pressure) { min_pressure = pressure; }
		if (pressure > max_pressure) { max_pressure = pressure; }
		pressure_3d->SetVertexDataAt(coords, pressure);
	}

	pressure_3d->SetScalarRange(min_pressure, max_pressure);

	delete pressure_2d;
	return pressure_3d;
}
std::vector<std::string> DataHelper::CollectTimes() {
	namespace fs = std::filesystem;
	std::vector<std::string> times;
	for (const auto& file : fs::directory_iterator(GetSrcPath()))
	{
		std::string file_name = file.path().filename().string();
		if (file_name[0] == 'P' && file_name[1] != 'P' && file_name.size() == 12) {
			std::string time = file_name.substr(1, file_name.size() - 1);
			times.push_back(time);
		}
	}
	std::sort(times.begin(), times.end());
	return times;
}

std::string DataHelper::GetDataStartDate() {
	std::vector<std::string> time_steps = CollectTimes();
	if (time_steps.size() == 0) {
		std::cout << "No Data found" << std::endl;
	}
	return time_steps[0];
}

std::string DataHelper::GetPreprocPath() {
	std::ifstream settings_file("settings.txt");
	std::string line;
	std::getline(settings_file, line);
	std::string srcPath = line.substr(16, line.size() - 16);
	std::getline(settings_file, line);
	std::string dstPath = line.substr(14, line.size() - 14);
	return dstPath;
};

std::string DataHelper::GetSrcPath() {
	std::ifstream settings_file("settings.txt");
	std::string line;
	std::getline(settings_file, line);
	std::string srcPath = line.substr(16, line.size() - 16);
	return srcPath;
};

std::vector<float> DataHelper::GetPsAxis()
{
  int stepSize = 10;
  double PS_domain_min = 10.;
  int pressureResolution = 104;

  std::vector<float> pressure(pressureResolution);
  pressure[0] = PS_domain_min;
  for (size_t i = 1; i < pressureResolution; i++)
  {
    pressure[i] = PS_domain_min + i * stepSize;
  }
  return pressure;
}