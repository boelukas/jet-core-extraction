#pragma once
#include "VisualizationHelper.h"

class vtkActor;
class vtkPolyData;
class VisualizationDataLoader;
class Wcb;
class ActorFactory;

class WcbVisualization
{
public:
	WcbVisualization(VisualizationDataLoader* dl,
		const size_t& time,
		const jet_vis::VIEW& view,
		const double& horizontal_filter,
		const double& vertical_filter,
		const size_t& points_before,
		const size_t& points_after);
	~WcbVisualization();
	vtkSmartPointer<vtkActor> getPointsActor();
	vtkSmartPointer<vtkActor> getExtensionsActor();

private:
	const size_t time_;
	vtkSmartPointer<vtkImageData> data_;
	Wcb* wcb;
	jet_vis::VIEW view_;
};
