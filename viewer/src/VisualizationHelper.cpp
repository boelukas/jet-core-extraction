#pragma once
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include "core/LineCollection.hpp"
#include "SceneWidget.h"

#include "VisualizationHelper.h"

void jet_vis::view_filter(LineCollection& lines, const VIEW& view) {
	if (view == VIEW::CLASSIC || view == VIEW::GLOBE) { return; }
	double threshold = 180;
	std::vector<Line3d> filtered_lines;
	std::vector<Line3d> unfiltered_lines = lines.getLinesInVectorOfVector();
	for (size_t i = 0; i < unfiltered_lines.size(); i++) {
		bool first_point = true;
		for (size_t j = 0; j < unfiltered_lines[i].size(); j++) {
			if (view == VIEW::POLAR_NORTH && unfiltered_lines[i][j][1] >= threshold) {
				if (first_point) {
					filtered_lines.push_back(Line3d({ unfiltered_lines[i][j] }));
					first_point = false;
				}
				else {
					filtered_lines[filtered_lines.size() - 1].push_back(unfiltered_lines[i][j]);
				}
			}
			else if (view == VIEW::POLAR_SOUTH && unfiltered_lines[i][j][1] < threshold) {
				if (first_point) {
					filtered_lines.push_back(Line3d({ unfiltered_lines[i][j] }));
					first_point = false;
				}
				else {
					filtered_lines[filtered_lines.size() - 1].push_back(unfiltered_lines[i][j]);
				}
			}

		}
	}
	lines.setData(filtered_lines);
}
void jet_vis::view_filter(Points3d& points, const VIEW& view) {
	if (view == VIEW::CLASSIC || view == VIEW::GLOBE) { return; }
	double threshold = 180;
	auto it = points.begin();
	while (it != points.end()) {
		if ((view == VIEW::POLAR_NORTH && (*it)[1] >= threshold) ||
			(view == VIEW::POLAR_SOUTH && (*it)[1] < threshold)) {
			it++;
		}
		else {
			it = points.erase(it);
		}
	}
}
void jet_vis::view_transform(Points3d& points, const VIEW& view) {
	if (view == jet_vis::VIEW::CLASSIC) { return; }
	if (view == VIEW::POLAR_SOUTH) {
		for (size_t i = 0; i < points.size(); i++) {
			Vec3d p = points[i];
			double r = p[1] / 180.0 * MAP_RADIUS;
			double phi = ((719.8 - p[0]) / 719.8) * 2.0 * M_PI;
			double x = r * cos(phi - M_PI / 2.0);
			double y = r * sin(phi - M_PI / 2.0);
			points[i] = Vec3d({ x * 2.0, y * 2.0, p[2] });
		}
	}
	else if (view == VIEW::POLAR_NORTH) {
		for (size_t i = 0; i < points.size(); i++) {
			Vec3d p = points[i];
			double r = ((359.8 - p[1]) / (359.8 - 180.0)) * MAP_RADIUS;
			double phi = (p[0] / 719.8) * 2 * M_PI;
			double x = r * cos(phi + M_PI / 2.0);
			double y = r * sin(phi + M_PI / 2.0);
			points[i] = Vec3d({ x * 2.0, y * 2.0 , p[2] });
		}
	}
	else {
		for (size_t i = 0; i < points.size(); i++) {
			Vec3d p = points[i];
			double r = p[2] + MAP_RADIUS;
			double phi = M_PI - (p[1] / 359.8) * M_PI;
			double psi = -M_PI + (p[0] / 719.8) * 2 * M_PI;
			double x = r * sin(phi) * cos(psi);
			double y = r * sin(phi) * sin(psi);
			double z = r * cos(phi);
			points[i] = Vec3d({ x * 2.0, y * 2.0, z });
		}
	}
}
void jet_vis::view_transform(LineCollection& lines, const VIEW& view) {
	if (view == VIEW::CLASSIC) { return; }
	std::vector<Line3d> lines_vec = lines.getLinesInVectorOfVector();
#pragma omp parallel for schedule(dynamic,16)
	for (long long i = 0; i < lines_vec.size(); i++) {
		view_transform(lines_vec[i], view);
	}
	lines.setData(lines_vec);
}

void jet_vis::view_transform(vtkSmartPointer<vtkActor> act, const VIEW& view, SceneWidget* sw) {
	if (view == VIEW::CLASSIC) { return; }
	vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::SafeDownCast(act->GetMapper()->GetInput());
	vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
	if (points != nullptr) {
		if (view == VIEW::GLOBE) {
			for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
				double* p = points->GetPoint(i);
				p[0] *= 2.0;
				p[1] *= 2.0;
				double r = p[2] + MAP_RADIUS;
				double phi = M_PI - (p[1] / 359.8) * M_PI;
				double psi = -M_PI + (p[0] / 719.8) * 2 * M_PI;
				double x = r * sin(phi) * cos(psi);
				double y = r * sin(phi) * sin(psi);
				double z = r * cos(phi);
				points->SetPoint(i, x, y, z);
			}
		}
		else if (view == VIEW::POLAR_NORTH) {
			for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
				double* p = points->GetPoint(i);
				p[0] *= 2.0;
				p[1] *= 2.0;
				double test = p[1];
				double r = ((359.8 - p[1]) / (359.8 - 180.0)) * MAP_RADIUS;
				double phi = (p[0] / 719.8) * 2 * M_PI;
				double x = r * cos(phi + M_PI / 2.0);
				double y = r * sin(phi + M_PI / 2.0);
				points->SetPoint(i, x, y, p[2]);
			}
		}
		else {
			for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
				double* p = points->GetPoint(i);
				p[0] *= 2.0;
				p[1] *= 2.0;
				double r = p[1] / 180.0 * MAP_RADIUS;
				double phi = ((719.8 - p[0]) / 719.8) * 2.0 * M_PI;
				double x = r * cos(phi - M_PI / 2.0);
				double y = r * sin(phi - M_PI / 2.0);
				points->SetPoint(i, x, y, p[2]);
			}
		}
	}
	std::cout << "End loop Jet" << std::endl;

	//Correcting the light: Recompute the normals
	if (view != VIEW::GLOBE) {
		sw->renderWindow()->GetRenderers()->GetFirstRenderer()->TwoSidedLightingOn();
	}
	vtkSmartPointer<vtkPolyDataNormals> pdNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	pdNormals->SetInputData(polyData);
	pdNormals->ComputeCellNormalsOn();

	act->GetMapper()->SetInputConnection(pdNormals->GetOutputPort());
	std::cout << "End loop Jet" << std::endl;

}