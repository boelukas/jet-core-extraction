#pragma once
#include <vtkSmartPointer.h>

#include "VisualizationHelper.h"

class vtkActor;
class vtkPolyData;
class vtkImageData;
class VisualizationDataLoader;

class TropopauseVisualization
{
public:
	TropopauseVisualization(VisualizationDataLoader* dl, const size_t& time);

	/*
		Extracts the tropopause of the requested hemisphere.
	*/
	vtkSmartPointer<vtkActor> getActor(const jet_vis::VIEW& view);

private:
	/*
		Extracts the tropopause by computing the iso surface 2 and -2 with a marching cubes algorithm and filtering away the false positives.
		Set the variable thresh to be the threshold after which all smalller values are discarded.
	*/
	vtkSmartPointer<vtkPolyData> extract_PolyData(const jet_vis::VIEW& view);
	const size_t time_;
	const float isoValue_;
	vtkSmartPointer<vtkImageData> data_;
};
