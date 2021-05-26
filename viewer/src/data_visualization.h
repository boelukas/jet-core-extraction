#pragma once
#include "VisualizationHelper.h"

class vtkVolume;
class vtkTextActor;
class vtkImageData;
class VisualizationDataLoader;

class DataVisualization
{

private:
	struct Data {
		vtkSmartPointer<vtkImageData> image_data = nullptr;
		double min_data_value = 1;
		double max_data_value = -1;
		int* dimensions = nullptr;
		std::string data_name = "Not_Initialized";
		size_t time = -1;
	};
	VisualizationDataLoader* op_;
	Data volume_data_;
	Data slice_data_;
	Data iso_data_;

	void load_volume_data(const std::string& dataName, const size_t& time);
	void load_slice_data(const std::string& dataName, const size_t& time);
	void load_iso_data(const std::string& dataName, const size_t& time);


public:
	DataVisualization(VisualizationDataLoader* op);

	vtkSmartPointer<vtkVolume> create_volume_actor(const std::string& dataName, const size_t& time, double minTransferVal, double maxTransferVal, const jet_vis::TRANSFERFUNCTION& tf);
	vtkSmartPointer<vtkActor> create_iso_actor(const std::string& dataName, const size_t& time, const double& level);
	vtkSmartPointer<vtkActor> create_slice_actor(const std::string& dataName, const size_t& time, const jet_vis::ORIENTATION& orientation, const int& location, const jet_vis::TRANSFERFUNCTION& tf, double minTransferVal, double maxTransferVal);

	Data get_volume_data() const {
		return volume_data_;
	}
	Data get_slice_data() const {
		return slice_data_;
	}
	Data get_iso_data() const {
		return iso_data_;
	}
};
