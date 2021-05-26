#pragma once
#include "core/RegularGrid.hpp"
#include "core/NetCDF.hpp"
#include "core/EraGrid.hpp"


#include <string.h>
#include <functional>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class Preprocessor
{
	typedef std::vector<std::string>  vecStr;
	typedef std::vector<vtkSmartPointer<vtkImageData>> vecImData;
public:
	enum EPreProcMode
	{
		REPLACE_EXISTING,
		KEEP_EXISTING
	};
	Preprocessor(std::string srcPath);
	bool process(EPreProcMode mode);
	bool loadAndResampleToEra();
	RegScalarField3f* Get3dPressure(std::string path);
	vtkSmartPointer<vtkImageData> CreateImageData(RegScalarField3f* data, std::vector<float> pressureAxisValues);


private:
	void writeFields(vecImData fields, std::string entryName, vecStr fieldNames);

	const char* createFileName(std::string entryName, std::string fieldName);
	std::string srcPath;
	std::string destPath;
	EPreProcMode mode;
	vecStr pfields = { "Q", "T", "OMEGA","LWC", "IWC", "RWC", "SWC", "U", "V"};
	vecStr sfields = { "PV", "THE" };
	vecStr custom_fields = { "PS3D" };
	//Step size for computing the 3D pressure. This will be the stepsize of the pressure axis.
	const int stepSize = 10;
};


