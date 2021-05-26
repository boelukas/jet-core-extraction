                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         #include "Preprocessor.hpp"

#include <filesystem>
#include <iostream>
#include <vtkFloatArray.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkFieldData.h>
#include <execution>

namespace fs = std::filesystem;


Preprocessor::Preprocessor(std::string _srcPath)
	:srcPath(_srcPath),
	mode(REPLACE_EXISTING),
	destPath(_srcPath + "\\Preprocessing")
{
	fs::create_directory(destPath);
}

bool Preprocessor::process(EPreProcMode m) {
	mode = m;
	bool t1 = loadAndResampleToEra();

	return t1;
}

bool Preprocessor::loadAndResampleToEra() {
	std::cout << "Resampling data to era..." << std::endl;
	int numFiles = std::distance(std::filesystem::directory_iterator(srcPath), std::filesystem::directory_iterator{}) - 1;
	double progress = 0;


	for (const auto& entry : fs::directory_iterator(srcPath))
	{
		//Progress Bar
		std::cout << "[";
		int pos = 50 * progress/numFiles;
		for (int i = 0; i < 50; ++i) {
			if (i <= pos) std::cout << "#";
			else std::cout << " ";
		}
		int p = progress / numFiles * 100.0;
		std::cout << "] " << p << " %\r";
		std::cout.flush();
		progress++;
		//End Progress Bar

		if (entry.is_directory()) { continue; }
		std::string entryPath = entry.path().string();
		std::string entryName = entry.path().filename().string();
		std::string time = entryName.substr(1, entryName.size() - 1);
		RegScalarField3f* PS = Get3dPressure(time);
		std::vector<float> lev;
		NetCDF::ImportFloatArray(srcPath, "lev", lev);
		vtkSmartPointer<vtkImageData> PS_vtkImData = CreateImageData(PS, lev);
		vecImData fields;
		if (entry.path().filename().string()[0] == 'P')
		{
			vecStr fieldsToBeWritten;
			for (int i = 0; i < pfields.size(); i++) {
				if (mode == KEEP_EXISTING && fs::exists(createFileName(entryName, pfields[i]))) { continue; }
				fieldsToBeWritten.push_back(pfields[i]);
				RegScalarField3f* data = NetCDF::ImportScalarField3f(entryPath, pfields[i], "lon", "lat", "lev");
				EraGrid<float> era = EraGrid<float>(data, PS);
				int pressureResolution = (int)std::ceil((PS->GetScalarRange()[1] - PS->GetScalarRange()[0]) / stepSize);

				Vec3d min_dom = Vec3d({data->GetDomain().GetMin()[0], data->GetDomain().GetMin()[1], PS->GetScalarRange()[1]});
				Vec3d max_dom = Vec3d({data->GetDomain().GetMax()[0], data->GetDomain().GetMax()[1], PS->GetScalarRange()[0]});
				BoundingBox3d resample_domain = BoundingBox3d(min_dom, max_dom);

				RegScalarField3f* data_era = new RegScalarField3f(Vec3i({ data->GetResolution()[0],data->GetResolution()[1], pressureResolution }), resample_domain);
				era.Resample(*data_era);
				std::vector<float> psAxisValues = era.GetPressureAxisValues(*data_era);

				min_dom[2] = psAxisValues[psAxisValues.size()-1];
				max_dom[2] = psAxisValues[0];
				data_era->SetDomain(BoundingBox3d(min_dom, max_dom));

				fields.push_back(CreateImageData(data_era, psAxisValues));
				delete data_era;
				delete data;
			}
			writeFields(fields, entry.path().filename().string(), fieldsToBeWritten);
		}
		else {
			vecStr fieldsToBeWritten;
			for (int i = 0; i < sfields.size(); i++) {
				if (mode == KEEP_EXISTING && fs::exists(createFileName(entryName, sfields[i]))) { continue; }
				fieldsToBeWritten.push_back(sfields[i]);
				RegScalarField3f* data = NetCDF::ImportScalarField3f(entryPath, sfields[i], "lon", "lat", "lev");
				EraGrid<float> era = EraGrid<float>(data, PS);
				int pressureResolution = (int)std::ceil((PS->GetScalarRange()[1] - PS->GetScalarRange()[0]) / stepSize);

				Vec3d min_dom = Vec3d({ data->GetDomain().GetMin()[0], data->GetDomain().GetMin()[1], PS->GetScalarRange()[1] });
				Vec3d max_dom = Vec3d({ data->GetDomain().GetMax()[0], data->GetDomain().GetMax()[1], PS->GetScalarRange()[0] });
				BoundingBox3d resample_domain = BoundingBox3d(min_dom, max_dom);

				RegScalarField3f* data_era = new RegScalarField3f(Vec3i({ data->GetResolution()[0],data->GetResolution()[1], pressureResolution }), resample_domain);
				era.Resample(*data_era);
				std::vector<float> psAxisValues = era.GetPressureAxisValues(*data_era);

				min_dom[2] = psAxisValues[psAxisValues.size() - 1];
				max_dom[2] = psAxisValues[0];
				data_era->SetDomain(BoundingBox3d(min_dom, max_dom));

				fields.push_back(CreateImageData(data_era, psAxisValues));
				delete data_era;
				delete data;
			}
			writeFields(fields, entry.path().filename().string(), fieldsToBeWritten);
		}
		// Write custom fields here
		vecStr fieldsToBeWritten;
		vecImData customFieldImData;
		if (mode == REPLACE_EXISTING || !fs::exists(createFileName(entryName, custom_fields[0]))) {
			fieldsToBeWritten.push_back(custom_fields[0]);
			std::string path = srcPath + "P" + time;
			customFieldImData.push_back(PS_vtkImData);
		}

		writeFields(customFieldImData, entry.path().filename().string(), fieldsToBeWritten);

		delete PS;
		
	}
	//endl for progress bar
	std::cout<<std::endl;
	std::cout << "Resampling data to era...Done" << std::endl;
	return true;
}
void Preprocessor::writeFields(vecImData fields, std::string entryName, vecStr fieldNames) {
	size_t numEntries = fields.size();
	std::vector<vtkSmartPointer<vtkXMLImageDataWriter>> dataWriter;
	for (long long i = 0; i < numEntries; i++) {
		dataWriter.push_back(vtkSmartPointer<vtkXMLImageDataWriter>::New());
		dataWriter[i]->SetFileName(createFileName(entryName, fieldNames[i]));
		dataWriter[i]->SetInputData(fields[i]);
	}
	auto f = [](int i, vtkSmartPointer<vtkXMLImageDataWriter> w) {
		w->Write();
	};
	std::vector<std::thread > thread_pool;
	thread_pool.reserve(numEntries);
	for (long long i = 0; i < numEntries; i++) {
		thread_pool.push_back(std::thread(f, i, dataWriter[i]));
		//std::cout << "end Writing" << std::endl;
	}
	for (long long i = 0; i < numEntries; i++) {
		thread_pool[i].join();
	}

}
const char* Preprocessor::createFileName(std::string entryName, std::string fieldName) {
	std::string time = entryName.substr(1, entryName.size() - 1);
	std::string t = destPath + "\\" + time + "_" + fieldName + ".vti";
	return t.c_str();
}

/*
	Returns the 3D pressure in (lon, lat, level) coordinates.
	Saves the max and min Pressure values in the mScalarRange fields of the Scalar field.
*/
RegScalarField3f* Preprocessor::Get3dPressure(std::string time) {
	std::string path = srcPath + "P" + time;

	
	RegScalarField3f* U = NetCDF::ImportScalarField3f(path, "U", "lon", "lat", "lev");//Eastwards wind

	RegScalarField3f* pressure3D = new RegScalarField3f(Vec3i({ U->GetResolution()[0], U->GetResolution()[1], U->GetResolution()[2] }), U->GetDomain());

	std::vector<float> lev, hyam, hybm;
	if (!NetCDF::ImportFloatArray(path, "lev", lev)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hyam", hyam)) return NULL;
	if (!NetCDF::ImportFloatArray(path, "hybm", hybm)) return NULL;

	RegScalarField2f* PS = NetCDF::ImportScalarField2f(path, "PS", "lon", "lat");
	float minPressure = 1000000;
	float maxPressure = -1;

	size_t numEntries = pressure3D->GetResolution()[0] * pressure3D->GetResolution()[1] * pressure3D->GetResolution()[2];
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = pressure3D->GetGridCoord(linearIndex);
		int i = coords[0];
		int j = coords[1];
		int k = coords[2];

		float pressure = hyam[lev[k] - 1] * 0.01 + hybm[lev[k] - 1] * PS->GetVertexDataAt(Vec2i({ i, j }));
		if (pressure < minPressure) { minPressure = pressure; }
		if (pressure > maxPressure) { maxPressure = pressure; }
		pressure3D->SetVertexDataAt(coords, pressure);
	}

	pressure3D->SetScalarRange(minPressure, maxPressure);

	delete U;
	delete PS;
	return pressure3D;
}

/*
	Gets the vtkImageData for the value of the fields of the class.
*/
vtkSmartPointer<vtkImageData> Preprocessor::CreateImageData(RegScalarField3f* data, std::vector<float> pressureAxisValues)
{

	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	imageData->SetDimensions(data->GetResolution().ptr());

	vtkSmartPointer<vtkFieldData> fd = vtkSmartPointer<vtkFieldData>::New();
	vtkSmartPointer<vtkFloatArray> minDom = vtkSmartPointer<vtkFloatArray>::New();
	minDom->SetName("MinDomain");
	minDom->SetNumberOfComponents(1);
	minDom->SetNumberOfValues(3);
	minDom->SetValue(0, data->GetDomain().GetMin()[0]);
	minDom->SetValue(1, data->GetDomain().GetMin()[1]);
	minDom->SetValue(2, data->GetDomain().GetMin()[2]);
	vtkSmartPointer<vtkFloatArray> maxDom = vtkSmartPointer<vtkFloatArray>::New();
	maxDom->SetName("MaxDomain");
	maxDom->SetNumberOfComponents(1);
	maxDom->SetNumberOfValues(3);
	maxDom->SetValue(0, data->GetDomain().GetMax()[0]);
	maxDom->SetValue(1, data->GetDomain().GetMax()[1]);
	maxDom->SetValue(2, data->GetDomain().GetMax()[2]);
	vtkSmartPointer<vtkFloatArray> psAxis = vtkSmartPointer<vtkFloatArray>::New();
	psAxis->SetName("PsAxisValues");
	size_t numAxisValues = pressureAxisValues.size();
	psAxis->SetNumberOfComponents(1);
	psAxis->SetNumberOfValues(numAxisValues);
#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long i = 0; i < numAxisValues; i++) {
		psAxis->SetValue(i, pressureAxisValues[i]);
	}

	fd->AddArray(minDom);
	fd->AddArray(maxDom);
	fd->AddArray(psAxis);
	imageData->SetFieldData(fd);

	double spacing[] = {
	(data->GetDomain().GetMax()[0] - data->GetDomain().GetMin()[0]) / (data->GetResolution()[0] - 1.),// (179.5 + 180)/719 = 0.5
	(data->GetDomain().GetMax()[1] - data->GetDomain().GetMin()[1]) / (data->GetResolution()[1] - 1.),// (90 + 90)/ 360 = 0.5
	1
	};

	imageData->SetSpacing(spacing);
	imageData->AllocateScalars(VTK_FLOAT, 1);

	float* data_field = (float*)(imageData->GetScalarPointer(0, 0, 0));

	// fill image data

	vtkSmartPointer<vtkFloatArray> floatArray = vtkSmartPointer<vtkFloatArray>::New();
	size_t numTuples = data->GetResolution()[0] * data->GetResolution()[1] * data->GetResolution()[2];

#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic,16)
#endif
	for (long long linearIndex = 0; linearIndex < numTuples; linearIndex++)
	{
		Vec3i gridCoord = data->GetGridCoord(linearIndex);
		float value = data->GetVertexDataAt(gridCoord);
		data_field[linearIndex] = value;
	}
	return imageData;

}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            