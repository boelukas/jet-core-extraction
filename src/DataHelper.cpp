#include <filesystem>
#include <fstream>

#include "jet-core-extraction/Amira.hpp"
#include "jet-core-extraction/LineCollection.hpp"
#include "jet-core-extraction/TimeHelper.hpp"
#include "jet-core-extraction/NetCDF.hpp"

#include "jet-core-extraction/DataHelper.hpp"

RegScalarField3f* DataHelper::loadPreprocessedScalarField(const std::string& fieldName, const size_t& time) {
	/*
		Loads data from the preprocessing directory.
	*/
	std::string p = getPreprocPath() + TimeHelper::convertHoursToDate(time, getDataStartDate()) + std::string("_") + fieldName;
	RegScalarField3f* field = Amira::ImportScalarField3f(p.c_str());
	return field;
}

RegScalarField3f* DataHelper::loadRegScalarField3f(const std::string& fieldName, const size_t& time) {
	/*
		Loads data from the source directory.
	*/
	std::string p;
	if (fieldName == "THE" || fieldName == "PV") {
		p = getSrcPath() + "S" + TimeHelper::convertHoursToDate(time, getDataStartDate());
	}
	else {
		p = getSrcPath() + "P" + TimeHelper::convertHoursToDate(time, getDataStartDate());
	}
	RegScalarField3f* field = NetCDF::ImportScalarField3f(p, fieldName, "lon", "lat", "lev");
	return field;
}
RegScalarField3f* DataHelper::loadQGOmega(const size_t& time) {
	/*
		Loads data from the QGOmega directory.
	*/
	std::string p = getSrcPath() + "B" + TimeHelper::convertHoursToDate(time, getDataStartDate());
	RegScalarField3f* field = NetCDF::ImportQGOmega(p, "OM_TOP");
	return field;
}

std::vector<float> DataHelper::loadFloatArray(const std::string& name) {
	/*
		Loads data from the preprocessing directory.
	*/
	std::string p = getPreprocPath() + name;
	return Amira::ImportStdVectorf(p.c_str());
}

void DataHelper::loadKernPaperJet(const size_t& time, LineCollection& jet_lines) {
	/*
		Loads data from the preprocessing directory.
	*/
	std::string p = getPreprocPath() + TimeHelper::convertHoursToDate(time, getDataStartDate()) + "_jet";
	//jet_lines.setData(Amira::ImportStdVectorf(p.c_str()));
	Amira::ImportLineGeometry("C:\\Users\\Lukas\\Desktop\\jetcores-kern-pv.am", jet_lines);
}
void DataHelper::loadLineCollection(const std::string& name, const size_t& time, const std::vector<std::string>& attributes, LineCollection& lines) {
	/*
		Loads data from the preprocessing directory.
	*/
	if (time < 0) { return; }
	if (name == "Jet") {
		std::string p = getPreprocPath() + TimeHelper::convertHoursToDate(time, getDataStartDate()) + "_jet";
		lines.Import(p.c_str(), attributes);
	}
	std::string fName = getPreprocPath() + std::to_string(TimeHelper::getYearFromHours(time, getDataStartDate()));
	size_t m = TimeHelper::getMonthFromHours(time, getDataStartDate());
	if (m < 10) {
		fName += "0";
	}
	fName += std::to_string(m);
	if (name == "Wcb") {
		lines.Import((fName + "_wcb").c_str(), attributes);
	}
	else if (name == "Wcb_dist") {
		lines.Import((fName + "_wcb_dist").c_str(), attributes);
	}
	else if (name == "Wcb_filtered_vert") {
		lines.Import((fName + "_wcb_filtered_vert").c_str(), attributes);
	}
	else if (name == "Wcb_points") {
		lines.Import((fName + "_wcb_points").c_str(), attributes);
	}
	else if (name == "Wcb_points_extended") {
		lines.Import((fName + "_wcb_points_extended").c_str(), attributes);
	}
}

RegScalarField3f* DataHelper::loadPS3D(const size_t& time) {
	return loadPreprocessedScalarField("PS3D", time);
}

EraScalarField3f* DataHelper::loadEraScalarField3f(const std::string& fieldName, const size_t& time) {
	/*
		Loads data from the preprocessing directory.
	*/
	std::string p = getPreprocPath() + TimeHelper::convertHoursToDate(time, getDataStartDate()) + std::string("_") + fieldName;
	RegScalarField3f* PS3D = loadPS3D(time);
	RegScalarField3f* field = Amira::ImportScalarField3f(p.c_str());
	return new EraScalarField3f(field, PS3D);
}

std::vector<RegScalarField3f*> DataHelper::loadScalarFields(const size_t& time, const std::vector<std::string>& fieldNames, const std::vector<bool>& preprocessed) {
	/*
		Loads a vector of scalar fields. In preprocessed it has to specified wether the field is in the folder preprocessed or not.
	*/
	int numberOfFields = fieldNames.size();
	std::vector<RegScalarField3f*> fields(numberOfFields, NULL);

#pragma omp parallel for schedule(dynamic,16)
	for (int i = 0; i < numberOfFields; i++) {
		if (preprocessed[i] == false) {
			fields[i] = loadRegScalarField3f(fieldNames[i], time);
		}
		else {
			fields[i] = loadPreprocessedScalarField(fieldNames[i], time);
		}
	}
	return fields;
}
/*
	Returns the 3D pressure in (lon, lat, level) coordinates.
	Saves the max and min Pressure values in the mScalarRange fields of the Scalar field.
*/
RegScalarField3f* DataHelper::compute_PS3D(const std::string& time, const Vec3i& resolution, const BoundingBox3d& domain) {


	std::string path = getSrcPath() + "P" + time;

	RegScalarField3f* pressure3D = new RegScalarField3f(resolution, domain);

	std::vector<float> lev, hyam, hybm;
	if (!NetCDF::ImportFloatArray(path, "lev", lev)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hyam", hyam)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hybm", hybm)) return NULL;

	RegScalarField2f* PS = NetCDF::ImportScalarField2f(path, "PS", "lon", "lat");
	float minPressure = 1000000;
	float maxPressure = -1;

	size_t numEntries = (size_t)pressure3D->GetResolution()[0] * (size_t)pressure3D->GetResolution()[1] * (size_t)pressure3D->GetResolution()[2];
#pragma omp parallel for schedule(dynamic,16)
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = pressure3D->GetGridCoord(linearIndex);
		int i = coords[0];
		int j = coords[1];
		int k = coords[2];

		float pressure = hyam[(size_t)std::round(lev[k]) - 1] * 0.01 + (double)hybm[(size_t)std::round(lev[k]) - 1] * (double)PS->GetVertexDataAt(Vec2i({ i, j }));
		if (pressure < minPressure) { minPressure = pressure; }
		if (pressure > maxPressure) { maxPressure = pressure; }
		pressure3D->SetVertexDataAt(coords, pressure);
	}

	pressure3D->SetScalarRange(minPressure, maxPressure);

	delete PS;
	return pressure3D;
}
std::vector<std::string> DataHelper::collectTimes() {
	namespace fs = std::filesystem;
	std::vector<std::string> times;
	for (const auto& entry : fs::directory_iterator(getSrcPath()))
	{
		std::string entryName = entry.path().filename().string();
		if (entryName[0] == 'P' && entryName[1] != 'P' && entryName.size() == 12) {
			std::string time = entryName.substr(1, entryName.size() - 1);
			times.push_back(time);
		}
	}
	std::sort(times.begin(), times.end());
	return times;
}

std::vector<std::string> DataHelper::collectWcbTimes() {
	std::vector<std::string> result;
	for (const auto& entry : std::filesystem::directory_iterator(getSrcPath()))
	{
		std::string entryName = entry.path().filename().string();
		if (entryName != "Preprocessing" && entryName.substr(0, 3) == "lcy") {
			std::string time = entryName.substr(4, entryName.size() - 1);
			result.push_back(time);
		}
	}
	return result;
}

std::string DataHelper::getDataStartDate() {
	std::vector<std::string> times = collectTimes();
	if (times.size() == 0) {
		std::cout << "No Data found" << std::endl;
	}
	return times[0];
}

std::string DataHelper::getPreprocPath() {
	std::ifstream infile("settings.txt");
	std::string line;
	std::getline(infile, line);
	std::string srcPath = line.substr(16, line.size() - 16);
	std::getline(infile, line);
	std::string dstPath = line.substr(14, line.size() - 14);
	return dstPath;
};

std::string DataHelper::getSrcPath() {
	std::ifstream infile("settings.txt");
	std::string line;
	std::getline(infile, line);
	std::string srcPath = line.substr(16, line.size() - 16);
	return srcPath;
};
