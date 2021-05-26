#pragma once
#include "EraGrid.hpp"

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <string.h>
#include <map>
#include <functional>



#include "core/NetCDF.hpp"

class DataLoader
{
	typedef std::tuple<std::string, int, bool> datakey_t;
	typedef std::map<datakey_t, bool> dataStorage_t;

public:
	DataLoader();
	vtkSmartPointer<vtkImageData> loadData(std::string dataName, int time, bool era);
	RegScalarField3f* loadPreprocessedScalarField(std::string fieldName, int time);
	RegScalarField3f* loadRegScalarField3f(std::string fieldName, int time);
	RegVectorField3f* loadPreprocessedVectorField(std::string fieldName, int time);
	std::vector<RegScalarField3f*> LoadScalarFields(int time, std::vector<std::string> fieldNames, std::vector<bool> preprocessed);


	std::vector<float> loadFloatArray(std::string name, int time);
	RegScalarField3f* Load3dPressure(int time);


	std::string convertHoursToDate(int time);


private:
	dataStorage_t dataStorage;
	RegScalarField3f* PS3D;
	vtkSmartPointer<vtkImageData> loadedData;
	int hours;
	std::string dataName;
	bool era;
	//std::string srcPath = "C:\\Users\\lukas\\Documents\\BachelorThesisPrep\\GitProject\\P_Data\\";
	std::string srcPath = "..\\..\\..\\P_Data\\";

	//std::string preprocDirectoryPath = "C:\\Users\\lukas\\Documents\\BachelorThesisPrep\\GitProject\\P_Data\\Preprocessing\\";
	std::string preprocDirectoryPath = "..\\..\\..\\P_Data\\Preprocessing\\";

	vtkSmartPointer<vtkImageData> cache[10];
	std::vector<float> metaDataCache[3];//lon, lat, ps axis labels
	bool axesloaded;
	vtkSmartPointer<vtkImageData> loadvtkImageData();

};


