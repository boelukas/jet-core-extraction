#pragma once
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include "core/RegularGrid.hpp"

class VisualizationDataLoader
{
	/*
		This class is responsible for loading data. To read data from disc, Amira is used.
		The client is responsible for deleting RegScalarFields and EraScalarFields.
		This class will clean up after imageData.
	*/
public:
	enum class CACHESLOT { VOLUME, SLICE, ISO, TROPO };

	VisualizationDataLoader();

	//Data loading functions
	vtkSmartPointer<vtkImageData> ImportScalarField3fAsVtkImageData(const char* path) const;
	vtkSmartPointer<vtkImageData> loadVtkImageData(const CACHESLOT& cacheslot, const std::string& dataName, const size_t& time);
	vtkSmartPointer<vtkImageData> loadPreprocessedVtkImageData(const size_t& time, const std::string& fieldName) const;

	//Converter functions
	vtkSmartPointer<vtkImageData> convert_RegScalarField3f_to_vtkImageData(RegScalarField3f* field) const;
	vtkSmartPointer<vtkImageData> convert_QGOmega_to_vtkImageData(RegScalarField3f* field) const;

private:
	//Helper functions
	vtkSmartPointer<vtkImageData> loadVtkImageData_helper(const CACHESLOT& slot, const std::string& dataName, const size_t& time);

	//Viewer data cache
	class Volume {
	public:
		Volume() :name("Not_Initialized"), data(nullptr), scalarFieldPtr(nullptr) {}
		std::string name;
		vtkSmartPointer<vtkImageData> data;
		RegScalarField3f* scalarFieldPtr;
	};
	class Slice {
	public:
		Slice() :name("Not_Initialized"), data(nullptr), scalarFieldPtr(nullptr) {}
		std::string name;
		vtkSmartPointer<vtkImageData> data;
		RegScalarField3f* scalarFieldPtr;
	};
	class Iso {
	public:
		Iso() :name("Not_Initialized"), data(nullptr), scalarFieldPtr(nullptr) {}
		std::string name;
		vtkSmartPointer<vtkImageData> data;
		RegScalarField3f* scalarFieldPtr;
	};
	class Tropo {
	public:
		Tropo() :name("Not_Initialized"), data(nullptr), scalarFieldPtr(nullptr) {}
		std::string name;
		vtkSmartPointer<vtkImageData> data;
		RegScalarField3f* scalarFieldPtr;
	};
	class Cache {
	public:
		Cache() :volume(Volume()), slice(Slice()), iso(Iso()), tropo(Tropo()), time(-1) {}
		Volume volume;
		Slice slice;
		Iso iso;
		Tropo tropo;
		size_t time;
	} data_cache;

	vtkSmartPointer<vtkImageData> cache[10];
};
