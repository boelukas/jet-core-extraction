#pragma once
#include <vtkNamedColors.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkFlyingEdges3D.h>
#include <vtkConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageMapToColors.h>
#include <vtkProperty.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkInformationVector.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkActor.h>

#include "Colors.h"
#include "VisualizationDataLoader.h"

#include "data_visualization.h"

DataVisualization::DataVisualization(VisualizationDataLoader* _op) :op_(_op)
{
}
/*
	Functions to load the data for the actors and store some meta inforations.
*/
void DataVisualization::load_volume_data(const std::string& dataName, const size_t& time) {
	volume_data_.data_name = dataName;
	volume_data_.time = time;
	volume_data_.image_data = op_->loadVtkImageData(VisualizationDataLoader::CACHESLOT::VOLUME, dataName, time);
	volume_data_.min_data_value = volume_data_.image_data->GetScalarRange()[0];
	volume_data_.max_data_value = volume_data_.image_data->GetScalarRange()[1];
	volume_data_.dimensions = volume_data_.image_data->GetDimensions();
}
void DataVisualization::load_slice_data(const std::string& dataName, const size_t& time) {
	slice_data_.data_name = dataName;
	slice_data_.time = time;
	slice_data_.image_data = op_->loadVtkImageData(VisualizationDataLoader::CACHESLOT::SLICE, dataName, time);
	slice_data_.min_data_value = slice_data_.image_data->GetScalarRange()[0];
	slice_data_.max_data_value = slice_data_.image_data->GetScalarRange()[1];
	slice_data_.dimensions = slice_data_.image_data->GetDimensions();
}
void DataVisualization::load_iso_data(const std::string& dataName, const size_t& time) {
	iso_data_.data_name = dataName;
	iso_data_.time = time;
	iso_data_.image_data = op_->loadVtkImageData(VisualizationDataLoader::CACHESLOT::ISO, dataName, time);
	iso_data_.min_data_value = iso_data_.image_data->GetScalarRange()[0];
	iso_data_.max_data_value = iso_data_.image_data->GetScalarRange()[1];
	iso_data_.dimensions = iso_data_.image_data->GetDimensions();
}
vtkSmartPointer<vtkVolume> DataVisualization::create_volume_actor(const std::string& dataName, const size_t& time, double minTransferValue, double maxTransferValue, const jet_vis::TRANSFERFUNCTION& tf) {
	/*
		Visualizes the data as a volume with a vtkOpenGLGPUVolumeRayCastMapper. MinTransferValue and MaxTransferValue set the visible scalar range of the volume.
	*/
	load_volume_data(dataName, time);
	if (maxTransferValue < minTransferValue) {
		minTransferValue = volume_data_.min_data_value;
		maxTransferValue = volume_data_.max_data_value;
	}

	vtkNew<vtkNamedColors> colors;

	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

	//If eps is too small, vtk will throw an error that the resolution of the opacity table is not supported. 1e-5 throws this error.
	double eps = (volume_data_.max_data_value - volume_data_.min_data_value) * 1e-4;
	opacityTransferFunction->AddPoint(minTransferValue, 0.0);
	opacityTransferFunction->AddPoint(maxTransferValue - eps, 0.8);
	opacityTransferFunction->AddPoint(maxTransferValue, 0.0);

	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = Colors::CreateColorTransferFunction(tf, 100, minTransferValue, maxTransferValue);

	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(opacityTransferFunction);


	vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputData(volume_data_.image_data);


	vtkNew<vtkVolume> volume;
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	return volume;
}
vtkSmartPointer<vtkActor> DataVisualization::create_iso_actor(const std::string& dataName, const size_t& time, const double& level) {
	/*
		Visualized the data as iso surface. The level is the iso level which is displayed.
	*/
	load_iso_data(dataName, time);
	vtkSmartPointer<vtkFlyingEdges3D> mc = vtkSmartPointer<vtkFlyingEdges3D>::New();
	mc->SetInputData(iso_data_.image_data);
	mc->SetValue(0, level);

	vtkSmartPointer<vtkConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkConnectivityFilter>::New();
	connectivityFilter->SetInputConnection(mc->GetOutputPort());
	//connectivityFilter->SetExtractionModeToLargestRegion();
	connectivityFilter->Update();


	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//mapper->SetInputConnection(connectivityFilter->GetOutputPort());
	mapper->SetInputConnection(mc->GetOutputPort());
	mapper->SetScalarVisibility(false);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	return actor;
}
vtkSmartPointer<vtkActor> DataVisualization::create_slice_actor(const std::string& dataName, const size_t& time, const jet_vis::ORIENTATION& orientation, const int& location, const jet_vis::TRANSFERFUNCTION& tf, double minTransferValue, double maxTransferValue) {
	/*
		Visualizes the data as slice through the image data. The orientation describes the orientation of the slide and the location its location in grid coordinates(not scaled with the spacing).
		The vtkImageSlice is transformed to a polydata with a vtkImageDataGeometryFilter.
	*/
	load_slice_data(dataName, time);
	if (maxTransferValue < minTransferValue) {
		minTransferValue = slice_data_.min_data_value;
		maxTransferValue = slice_data_.max_data_value;
	}
	vtkSmartPointer<vtkLookupTable> lut;
	if (dataName == "QGOmega") {
		lut = Colors::CreateLookupTable(tf, minTransferValue, maxTransferValue);
		//lut = Colors::CreateLookupTable(tf, -2.0, 1.3);
		lut->SetBelowRangeColor(0.0, 0.0, 0.0, 1.0);
		lut->SetUseBelowRangeColor(true);
	}
	else {
		lut = Colors::CreateLookupTable(tf, minTransferValue, maxTransferValue);
	}

	vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap->SetInputData(slice_data_.image_data);
	colorMap->SetLookupTable(lut);
	colorMap->Update();

	static double axialElements[16] = {
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1 };
	static double sagittalElements[16] = {
			   0, 0,-1, 0,
			   1, 0, 0, 0,
			   0,-1, 0, 0,
			   0, 0, 0, 1 };
	static double coronalElements[16] = {
				  1, 0, 0, 0,
				  0, 0, 1, 0,
				  0,-1, 0, 0,
				  0, 0, 0, 1 };

	// Set the slice orientation
	vtkSmartPointer<vtkMatrix4x4> resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
	switch (orientation) {
	case jet_vis::ORIENTATION::X:
		resliceAxes->DeepCopy(sagittalElements);
		resliceAxes->SetElement(0, 3, location * 0.5);
		resliceAxes->SetElement(1, 3, 0);
		resliceAxes->SetElement(2, 3, 0);
		break;
	case jet_vis::ORIENTATION::Y:
		resliceAxes->DeepCopy(coronalElements);
		resliceAxes->SetElement(0, 3, 0);
		resliceAxes->SetElement(1, 3, location * 0.5);
		resliceAxes->SetElement(2, 3, 0);
		break;
	case jet_vis::ORIENTATION::Z:
		resliceAxes->DeepCopy(axialElements);
		resliceAxes->SetElement(0, 3, 0);
		resliceAxes->SetElement(1, 3, 0);
		if (dataName == "QGOmega") {
			resliceAxes->SetElement(2, 3, location * 2.5 + 4);
		}
		else {
			resliceAxes->SetElement(2, 3, location);
		}
		break;
	}

	vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
	reslice->SetInputConnection(colorMap->GetOutputPort());
	reslice->SetOutputDimensionality(2);
	reslice->SetResliceAxes(resliceAxes);
	reslice->SetInterpolationModeToLinear();

	vtkSmartPointer<vtkImageDataGeometryFilter> imageDataGeometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	vtkSmartPointer<vtkInformationVector> v = vtkSmartPointer<vtkInformationVector>::New();
	imageDataGeometryFilter->SetInputConnection(reslice->GetOutputPort());
	imageDataGeometryFilter->Update();

	vtkSmartPointer<vtkPolyData> polyData = imageDataGeometryFilter->GetOutput();
	vtkSmartPointer<vtkPoints> polyPoints = polyData->GetPoints();
	for (long long i = 0; i < polyPoints->GetNumberOfPoints(); i++)
	{
		double* p = polyPoints->GetPoint(i);
		double x = p[0];
		double y = p[1];
		double z = p[2];
		switch (orientation) {
		case jet_vis::ORIENTATION::X:
			p[1] = x;
			p[2] = -y;
			p[0] = location * 0.5;
			polyPoints->SetPoint(i, p);
			break;
		case jet_vis::ORIENTATION::Y:
			p[2] = -y;
			p[0] = x;
			p[1] = location * 0.5;
			polyPoints->SetPoint(i, p);
			break;
		case jet_vis::ORIENTATION::Z:
			if (dataName == "QGOmega") {
				p[2] = location * 2.5 + 4;
				//p[2] = location;
			}
			else {
				p[2] = location;
			}
			p[0] = x;
			p[1] = y;
			break;
		}
		polyPoints->SetPoint(i, p);

	}
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->ShadingOff();
	actor->GetProperty()->LightingOff();
	return actor;
}
