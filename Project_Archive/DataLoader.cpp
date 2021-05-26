#include "DataLoader.hpp"
#include "Math.hpp"
#include "Gradient.hpp"
#include <cmath>
#include <vtkFloatArray.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkFieldData.h>
#include <direct.h>
#include <filesystem>
#include <time.h>
#include <stdio.h>
#include <execution>


#include "Preprocessor.hpp"
#include "WindFields.hpp"
#include "Amira.hpp"


DataLoader::DataLoader() :hours(-1), PS3D(new RegScalarField3f(Vec3i({0,0,0}), BoundingBox3d()))
, dataStorage({}), axesloaded(false)
{
	metaDataCache[0] = loadFloatArray("lon", 0);
	metaDataCache[1] = loadFloatArray("lat", 0);
	metaDataCache[2] = loadFloatArray("PsAxisValues", 0);//Inefficient
	axesloaded = true;
}

/*
	Loads the data with name _dataName at time_time.
*/
vtkSmartPointer<vtkImageData> DataLoader::loadData(std::string _dataName, int _time, bool _era)
{
	//In case the variables didn't change since the last data was loaded, return the same.
	if (_dataName == dataName && _time == hours && _era == era && loadedData != NULL) {
		return loadedData;
	}
	//Only the time changed: Data are already in the cache
	if (_dataName == dataName && _era == era && loadedData != NULL) {
		loadedData = cache[_time];
		hours = _time;
		return loadedData;
	}
	if (!_era) {
		//Not handled case for now. Assumption all data are requested in era layout.
		return NULL;
	}
	/*
	Temporary way of visualizing the windForce. Time slider does not work this way
	*/
	/*
	if (_dataName == "WindForce") {
		Preprocessor p = Preprocessor(srcPath);
		WindFields wf = WindFields(this);
		loadedData = p.CreateImageData(wf.GetWindForce(_time), loadFloatArray("PsAxisValues", _time));
		hours = _time;
		dataName = _dataName;
		era = _era;
		return loadedData;
	}
	if (_dataName == "Gradient") {
		Preprocessor p = Preprocessor(srcPath);
		Gradient g = Gradient();
		WindFields wf = WindFields(this);
		//RegScalarField3f* U = loadPreprocessedScalarField("U", 0);

		loadedData = p.CreateImageData(g.gradientMagnitude(wf.GetWindForce(_time), loadPreprocessedScalarField("T", _time)), loadFloatArray("PsAxisValues", _time));
		hours = _time;
		dataName = _dataName;
		era = _era;
		return loadedData;
	}
	if (_dataName == "Debug") {
		Preprocessor p = Preprocessor(srcPath);
		WindFields wf = WindFields(this);
		RegScalarField3f* U = loadPreprocessedScalarField("V", 0);
		wf.convertToLatPerS(U, 0);
		loadedData = p.CreateImageData(U, loadFloatArray("PsAxisValues", _time));
		hours = _time;
		dataName = _dataName;
		era = _era;
		return loadedData;
	}
	*/
	
	dataName = _dataName;
	era = _era;
	hours = _time;
	
	loadedData = loadvtkImageData();

	return loadedData;

}
RegVectorField3f* DataLoader::loadPreprocessedVectorField(std::string fieldName, int time) {
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	std::string p = preprocDirectoryPath + convertHoursToDate(time) + std::string("_") + fieldName + ".vti";

	reader->SetFileName(p.c_str());
	imData = reader->GetOutput();
	reader->Update();

	//Get the dimensions
	int* dim = imData->GetDimensions();

	//Get the domain
	vtkSmartPointer<vtkFieldData> d = imData->GetFieldData();
	vtkFloatArray* min = (vtkFloatArray*)d->GetAbstractArray("MinDomain");
	float min1 = min->GetValue(0);
	float min2 = min->GetValue(1);
	float min3 = min->GetValue(2);
	vtkFloatArray* max = (vtkFloatArray*)d->GetAbstractArray("MaxDomain");
	float max1 = max->GetValue(0);
	float max2 = max->GetValue(1);
	float max3 = max->GetValue(2);

	BoundingBox3d domain = BoundingBox3d(Vec3d({ min1,min2,min3 }), Vec3d({ max1, max2, max3 }));
	RegVectorField3f* field = new RegVectorField3f(Vec3i({ dim[0], dim[1], dim[2] }), domain);

	Vec3f* rawdata = field->GetData().data();

	size_t numTuples = dim[0] * dim[1] * dim[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numTuples; linearIndex++)
	{
		Vec3i gridCoord = field->GetGridCoord(linearIndex);
		Vec3f v = Vec3f({ 
			imData->GetScalarComponentAsFloat(gridCoord[0], gridCoord[1], gridCoord[2], 0),
			imData->GetScalarComponentAsFloat(gridCoord[0], gridCoord[1], gridCoord[2], 1),
			imData->GetScalarComponentAsFloat(gridCoord[0], gridCoord[1], gridCoord[2], 2)
			});
		field->SetVertexDataAt(gridCoord,v);
	}
	return field;
}
/*
	Reads 10 time steps in parallel for the selected dataName and saves them in the cache. 
	The cache is an array with 10 entries and is cleared and reloaded when the dataName changes.
*/
vtkSmartPointer<vtkImageData> DataLoader::loadvtkImageData() {

	int cacheSize = 10;
	auto f = [](int cacheLocation, std::string path, vtkSmartPointer<vtkImageData> readCache[]) {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(path.c_str());
		readCache[cacheLocation] = reader->GetOutput();
		reader->Update();
	};
	std::vector<std::thread > thread_pool;
	thread_pool.reserve(cacheSize);
	for (int i = 0; i < cacheSize; i++) {
		std::string p = preprocDirectoryPath + convertHoursToDate(i) + std::string("_") + dataName + ".vti";
		thread_pool.push_back(std::thread(f, i, p, cache ));
	}
	for (int i = 0; i < cacheSize; i++) {
		thread_pool[i].join();
	}

	return cache[hours];
}
/*
	Loads one of the in the preprocessing produced files.
*/
RegScalarField3f* DataLoader::loadPreprocessedScalarField(std::string fieldName, int time) {

	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	std::string p = preprocDirectoryPath + convertHoursToDate(time) + std::string("_") + fieldName + ".vti";
	reader->SetFileName(p.c_str());
	vtkSmartPointer<vtkImageData> imData = reader->GetOutput();
	reader->Update();

	//Get the dimensions
	int* dim = imData->GetDimensions();

	//Get the domain
	vtkSmartPointer<vtkFieldData> d = imData->GetFieldData();
	vtkFloatArray* min = (vtkFloatArray*)d->GetAbstractArray("MinDomain");
	float min1 = min->GetValue(0);
	float min2 = min->GetValue(1);
	float min3 = min->GetValue(2);
	vtkFloatArray* max = (vtkFloatArray*)d->GetAbstractArray("MaxDomain");
	float max1 = max->GetValue(0);
	float max2 = max->GetValue(1);
	float max3 = max->GetValue(2);

	BoundingBox3d domain = BoundingBox3d(Vec3d({ min1,min2,min3 }), Vec3d({ max1, max2, max3 }));
	RegScalarField3f* field = new RegScalarField3f(Vec3i({ dim[0], dim[1], dim[2] }), domain);

	float* rawdata = field->GetData().data();
	float* data_image = (float*)(imData->GetScalarPointer(0, 0, 0));

	size_t numTuples = dim[0] * dim[1] * dim[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numTuples; linearIndex++)
	{
		Vec3i gridCoord = field->GetGridCoord(linearIndex);
		field->SetVertexDataAt(gridCoord, data_image[linearIndex]);
	}
	return field;
}

RegScalarField3f* DataLoader::loadRegScalarField3f(std::string fieldName, int time) {
	std::string p;
	if (fieldName == "THE" || fieldName == "PV") {
		p = srcPath + "S" + convertHoursToDate(time);
	}
	else {
		p = srcPath + "P" + convertHoursToDate(time);
	}
	RegScalarField3f* field = NetCDF::ImportScalarField3f(p, fieldName, "lon", "lat", "lev");
	return field;
}


std::vector<float> DataLoader::loadFloatArray(std::string name, int time) {
	if (axesloaded) {
		if (name == "lon") { return metaDataCache[0]; }
		if (name == "lat") { return metaDataCache[1]; }
		if (name == "PsAxisValues") { return metaDataCache[2]; }

	}
	else {
		std::vector<float> vec;
		std::string date = convertHoursToDate(time);
		std::string p = srcPath + "P" + date;
		if (name == "PsAxisValues") {
			vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
			p = preprocDirectoryPath + date + std::string("_") + "OMEGA" + ".vti";
			reader->SetFileName(p.c_str());
			vtkSmartPointer<vtkImageData> temp = reader->GetOutput();
			reader->Update();
			vtkSmartPointer<vtkFieldData> d = temp->GetFieldData();
			vtkFloatArray* axisVals = (vtkFloatArray*)d->GetAbstractArray("PsAxisValues");
			size_t vecLength = axisVals->GetSize();
			for (long long i = 0; i < vecLength; i++) {
				vec.push_back(axisVals->GetValue(i));
			}
			return vec;
		}
		else if (!NetCDF::ImportFloatArray(p, name, vec)) {
			return std::vector<float>(-1, -1);
		}
		else {
			return vec;
		}
	}
}

RegScalarField3f* DataLoader::Load3dPressure(int time) {
	return loadPreprocessedScalarField("PS3D", time);
}

/*
	Loads a vector of scalar fields. In preprocessed it has to specified wether the field is in the folder preprocessed or not.
*/
std::vector<RegScalarField3f*> DataLoader::LoadScalarFields(int time, std::vector<std::string> fieldNames, std::vector<bool> preprocessed) {
	int numberOfFields = fieldNames.size();
	std::vector<RegScalarField3f*> fields(numberOfFields, NULL);
	
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
	Works only in summertime. In winter a -1 needs to be added in the marked line.
*/
std::string DataLoader::convertHoursToDate(int _hours) {
	int startYear = 2016;
	int startMonth = 9;
	int startDay = 1;
	int startHour = 0;
	char out[30];
	time_t rawtime;
	time(&rawtime);
	struct tm* stm = localtime(&rawtime);
	stm->tm_year = startYear - 1900;
	stm->tm_mon = startMonth - 1;
	stm->tm_mday = startDay;
	stm->tm_hour = startHour;
	stm->tm_min = 0;
	stm->tm_sec = 0;
	stm->tm_hour += (_hours); // Could be buggy there. In summer time this works, winter time, need to subtract there 1 hour.
	mktime(stm);
	strftime(out, 30, "%Y%m%d_%H", stm);
	return std::string(out);
}