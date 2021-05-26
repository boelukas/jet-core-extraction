#pragma once
#include <vtkPoints.h>

#include "core/Wcb.hpp"
#include "core/LineCollection.hpp"

#include "geometry_visualization.h"
#include "VisualizationDataLoader.h"

#include "WcbVisualization.h"

WcbVisualization::WcbVisualization(VisualizationDataLoader* dl,
		const size_t& time,
		const jet_vis::VIEW& view,
		const double& horizontal_filter,
		const double& vertical_filter,
		const size_t& points_before,
		const size_t& points_after):time_(time), view_(view){
	wcb = new Wcb(time_, horizontal_filter, vertical_filter, points_before, points_after);
}
WcbVisualization::~WcbVisualization() {
	delete wcb;

}

vtkSmartPointer<vtkActor> WcbVisualization::getPointsActor() {
	Points3d wcb_points = wcb->getWcbPoints();
	jet_vis::view_filter(wcb_points, view_);
	jet_vis::view_transform(wcb_points, view_);
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	for (size_t i = 0; i < wcb_points.size(); i++) {
		double x = wcb_points[i][0] * 0.5;
		double y = wcb_points[i][1] * 0.5;
		double z = wcb_points[i][2] * 1;
		points->InsertNextPoint(x, y, z);
	}
	return GeometryVisualization::create_points_actor(points, 255 / 255., 20 / 255., 147 / 255., 1.5);
}
vtkSmartPointer<vtkActor> WcbVisualization::getExtensionsActor() {
	LineCollection extensions = wcb->getWcbLines();
	jet_vis::view_filter(extensions, view_);
	jet_vis::view_transform(extensions, view_);
	return  GeometryVisualization::create_lines_actor(extensions, 255 / 255., 20 / 255., 147 / 255.);
}