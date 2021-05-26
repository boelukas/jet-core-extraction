#pragma once
#include "vtkSmartPointer.h"
#include "vtkTextActor.h"
#include "vtkActor.h"

#include "VisualizationHelper.h"

class VisualizationDataLoader;

class GeometryVisualization
{
public:
	static vtkSmartPointer<vtkActor> create_map_actor(const jet_vis::VIEW& view);
	static vtkSmartPointer<vtkTextActor> create_date_label_follower(const size_t& time, VisualizationDataLoader* op);
	static vtkSmartPointer<vtkActor> create_points_actor(vtkSmartPointer<vtkPoints> points, double r, double g, double b, double radius);
	static vtkSmartPointer<vtkActor> create_lines_actor(const LineCollection& lines, double r, double g, double b);
};
