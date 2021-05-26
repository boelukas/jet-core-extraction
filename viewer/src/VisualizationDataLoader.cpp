#pragma once
#include <vtkImageImport.h>

#include "core/DataHelper.hpp"
#include "core/TimeHelper.hpp"
#include "core/Amira.hpp"

#include "VisualizationDataLoader.h"

VisualizationDataLoader::VisualizationDataLoader() {}

vtkSmartPointer<vtkImageData> VisualizationDataLoader::loadVtkImageData(const CACHESLOT& cacheslot, const std::string& dataName, const size_t& time) {
	/*
		Is called by the main window or the ActorFactory to get vtkImageData vor visualizing. This method keeps a cache up to date which can store data for volume, slice, iso and the tropopause.
	*/

	if (time != data_cache.time) {
		if (data_cache.volume.data != nullptr) {
			if (data_cache.volume.scalarFieldPtr != nullptr) {
				delete data_cache.volume.scalarFieldPtr;
				data_cache.volume.scalarFieldPtr = nullptr;
			}
			else {
				delete data_cache.volume.data->GetScalarPointer();
			}
			data_cache.volume.data = nullptr;
			data_cache.volume.name = "";
			if (data_cache.volume.scalarFieldPtr != nullptr) {
				delete data_cache.volume.scalarFieldPtr;
				data_cache.volume.scalarFieldPtr = nullptr;
			}
		}
		if (data_cache.slice.data != nullptr) {
			if (data_cache.slice.scalarFieldPtr != nullptr) {
				delete data_cache.slice.scalarFieldPtr;
				data_cache.slice.scalarFieldPtr = nullptr;
			}
			else {
				delete data_cache.slice.data->GetScalarPointer();
			}
			data_cache.slice.data = nullptr;
			data_cache.slice.name = "";
			if (data_cache.slice.scalarFieldPtr != nullptr) {
				delete data_cache.slice.scalarFieldPtr;
				data_cache.slice.scalarFieldPtr = nullptr;
			}
		}
		if (data_cache.iso.data != nullptr) {
			if (data_cache.iso.scalarFieldPtr != nullptr) {
				delete data_cache.iso.scalarFieldPtr;
				data_cache.iso.scalarFieldPtr = nullptr;
			}
			else {
				delete data_cache.iso.data->GetScalarPointer();
			}
			data_cache.iso.data = nullptr;
			data_cache.iso.name = "";
			if (data_cache.iso.scalarFieldPtr != nullptr) {
				delete data_cache.iso.scalarFieldPtr;
				data_cache.iso.scalarFieldPtr = nullptr;
			}
		}
		if (data_cache.tropo.data != nullptr) {
			if (data_cache.tropo.scalarFieldPtr != nullptr) {
				delete data_cache.tropo.scalarFieldPtr;
				data_cache.tropo.scalarFieldPtr = nullptr;
			}
			else {
				delete data_cache.tropo.data->GetScalarPointer();
			}
			data_cache.tropo.data = nullptr;
			data_cache.tropo.name = "";
			if (data_cache.tropo.scalarFieldPtr != nullptr) {
				delete data_cache.tropo.scalarFieldPtr;
				data_cache.tropo.scalarFieldPtr = nullptr;
			}
		}
		data_cache.time = time;
	}
	switch (cacheslot)
	{
	case VisualizationDataLoader::CACHESLOT::VOLUME:
		if (dataName == data_cache.volume.name) {
			return data_cache.volume.data;
		}
		else {
			if (data_cache.volume.data != nullptr) {
				if (data_cache.volume.scalarFieldPtr != nullptr) {
					delete data_cache.volume.scalarFieldPtr;
					data_cache.volume.scalarFieldPtr = nullptr;
				}
				else {
					delete data_cache.volume.data->GetScalarPointer();
				}
			}
			data_cache.volume.data = loadVtkImageData_helper(CACHESLOT::VOLUME, dataName, time);
			data_cache.volume.name = dataName;
			return data_cache.volume.data;
		}
		break;
	case VisualizationDataLoader::CACHESLOT::SLICE:
		if (dataName == data_cache.slice.name) {
			return data_cache.slice.data;
		}
		else {
			if (data_cache.slice.data != nullptr) {
				if (data_cache.slice.scalarFieldPtr != nullptr) {
					delete data_cache.slice.scalarFieldPtr;
					data_cache.slice.scalarFieldPtr = nullptr;
				}
				else {
					delete data_cache.slice.data->GetScalarPointer();
				}
			}
			data_cache.slice.data = loadVtkImageData_helper(CACHESLOT::SLICE, dataName, time);
			data_cache.slice.name = dataName;
			return data_cache.slice.data;
		}
		break;
	case VisualizationDataLoader::CACHESLOT::ISO:
		if (dataName == data_cache.iso.name) {
			return data_cache.iso.data;
		}
		else {
			if (data_cache.iso.data != nullptr) {
				if (data_cache.iso.scalarFieldPtr != nullptr) {
					delete data_cache.iso.scalarFieldPtr;
					data_cache.iso.scalarFieldPtr = nullptr;
				}
				else {
					delete data_cache.iso.data->GetScalarPointer();
				}
			}
			data_cache.iso.data = loadVtkImageData_helper(CACHESLOT::ISO, dataName, time);
			data_cache.iso.name = dataName;
			return data_cache.iso.data;
		}
		break;
	case VisualizationDataLoader::CACHESLOT::TROPO:
		if (dataName == data_cache.tropo.name) {
			return data_cache.tropo.data;
		}
		else {
			if (data_cache.tropo.data != nullptr) {
				if (data_cache.tropo.scalarFieldPtr != nullptr) {
					delete data_cache.tropo.scalarFieldPtr;
					data_cache.tropo.scalarFieldPtr = nullptr;
				}
				else {
					delete data_cache.tropo.data->GetScalarPointer();
				}
			}
			data_cache.tropo.data = loadVtkImageData_helper(CACHESLOT::TROPO, dataName, time);
			data_cache.tropo.name = dataName;
			return data_cache.tropo.data;
		}
		break;
	}
	return nullptr;

}
vtkSmartPointer<vtkImageData> VisualizationDataLoader::loadVtkImageData_helper(const CACHESLOT& slot, const std::string& dataName, const size_t& time)
/*
	Loads the data with name dataName and returns it.
*/
{
	if (dataName == "Wind Magnitude") {
		return loadPreprocessedVtkImageData(time, "windMagnitude_ps");
	}
	else if (dataName == "QGOmega") {
		RegScalarField3f* QC = DataHelper::loadQGOmega(time);
		vtkSmartPointer<vtkImageData> qcIm = convert_QGOmega_to_vtkImageData(QC);
		switch (slot)
		{
		case VisualizationDataLoader::CACHESLOT::VOLUME:
			data_cache.volume.scalarFieldPtr = QC;
			break;
		case VisualizationDataLoader::CACHESLOT::SLICE:
			data_cache.slice.scalarFieldPtr = QC;
			break;
		case VisualizationDataLoader::CACHESLOT::ISO:
			data_cache.iso.scalarFieldPtr = QC;
			break;
		case VisualizationDataLoader::CACHESLOT::TROPO:
			data_cache.tropo.scalarFieldPtr = QC;
			break;
		}
		return qcIm;
	}
	else {
		return loadPreprocessedVtkImageData(time, dataName);
	}
}
vtkSmartPointer<vtkImageData> VisualizationDataLoader::loadPreprocessedVtkImageData(const size_t& time, const std::string& fieldName) const {
	/*
		Loads data from the preprocessing directory.
	*/
	std::string p = DataHelper::getPreprocPath() + TimeHelper::convertHoursToDate(time, DataHelper::getDataStartDate()) + std::string("_") + fieldName;
	//RegScalarField3f* field = Amira::ImportScalarField3f(p.c_str());
	//vtkSmartPointer<vtkImageData> imData = convert_RegScalarField3f_to_vtkImageData(field);
	vtkSmartPointer<vtkImageData> imData = ImportScalarField3fAsVtkImageData(p.c_str());
	return imData;
}
vtkSmartPointer<vtkImageData> VisualizationDataLoader::ImportScalarField3fAsVtkImageData(const char* path) const{
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return NULL;
	}

	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return NULL;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(Amira::FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(Amira::FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(Amira::FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(Amira::FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return NULL;
	}

	if (NumComponents != 1)
	{
		printf("AmiraLoader: Not a scalar field!\n");
		fclose(fp);
		return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim * yDim * zDim * NumComponents;

		float* fltData = new float[NumToRead];
		const size_t ActRead = fread((void*)fltData, sizeof(float), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
			fclose(fp);
			//delete field;
			return NULL;
		}
		double spacing[] = { (xmax - xmin) / (xDim - 1.), (ymax - ymin) / (yDim - 1.), 1. };


		vtkSmartPointer<vtkImageImport> imageImport = vtkSmartPointer<vtkImageImport>::New();
		imageImport->SetDataSpacing(spacing);
		imageImport->SetDataOrigin(0, 0, 0);
		imageImport->SetWholeExtent(0, xDim - 1, 0, yDim - 1, 0, zDim - 1);
		imageImport->SetDataExtentToWholeExtent();
		imageImport->SetDataScalarTypeToFloat();
		imageImport->SetNumberOfScalarComponents(1);
		imageImport->SetImportVoidPointer((void*)fltData); // try to set (prt, 0) -> error. otherwise fltData is never deallocated
		imageImport->Update();
		vtkSmartPointer<vtkImageData> field = vtkSmartPointer(imageImport->GetOutput());
		fclose(fp);
		return field;
	}

	fclose(fp);
	return NULL;
}

vtkSmartPointer<vtkImageData> VisualizationDataLoader::convert_RegScalarField3f_to_vtkImageData(RegScalarField3f* field) const {
	float* field_data = (float*)(&field->GetData()[0]);

	double spacing[] = {
	(field->GetDomain().GetMax()[0] - field->GetDomain().GetMin()[0]) / (field->GetResolution()[0] - 1.),// (179.5 + 180)/719 = 0.5
	(field->GetDomain().GetMax()[1] - field->GetDomain().GetMin()[1]) / (field->GetResolution()[1] - 1.),// (90 + 90)/ 360 = 0.5
	1
	};
	double d1 = spacing[0];
	double d2 = spacing[1];

	vtkSmartPointer<vtkImageImport> imageImport = vtkSmartPointer<vtkImageImport>::New();
	imageImport->SetDataSpacing(spacing);
	imageImport->SetDataOrigin(0, 0, 0);
	imageImport->SetWholeExtent(0, field->GetResolution()[0] - 1, 0, field->GetResolution()[1] - 1, 0, field->GetResolution()[2] - 1);
	imageImport->SetDataExtentToWholeExtent();
	imageImport->SetDataScalarTypeToFloat();
	imageImport->SetNumberOfScalarComponents(1);
	imageImport->SetImportVoidPointer((void*)field_data);
	imageImport->Update();

	return vtkSmartPointer(imageImport->GetOutput());
}
vtkSmartPointer<vtkImageData> VisualizationDataLoader::convert_QGOmega_to_vtkImageData(RegScalarField3f* field) const {
	float* field_data = (float*)(&field->GetData()[0]);

	double spacing[] = {
	(field->GetDomain().GetMax()[0] - field->GetDomain().GetMin()[0]) / (field->GetResolution()[0] - 1.),// (179.5 + 180)/719 = 0.5
	(field->GetDomain().GetMax()[1] - field->GetDomain().GetMin()[1]) / (field->GetResolution()[1] - 1.),// (90 + 90)/ 360 = 0.5
	2.5
	};
	double d1 = spacing[0];
	double d2 = spacing[1];

	vtkSmartPointer<vtkImageImport> imageImport = vtkSmartPointer<vtkImageImport>::New();
	imageImport->SetDataSpacing(spacing);
	imageImport->SetDataOrigin(0, 0, 4);
	imageImport->SetWholeExtent(0, field->GetResolution()[0] - 1, 0, field->GetResolution()[1] - 1, 0, field->GetResolution()[2] - 1);
	imageImport->SetDataExtentToWholeExtent();
	imageImport->SetDataScalarTypeToFloat();
	imageImport->SetNumberOfScalarComponents(1);
	imageImport->SetImportVoidPointer((void*)field_data);
	imageImport->Update();

	return vtkSmartPointer(imageImport->GetOutput());
}
