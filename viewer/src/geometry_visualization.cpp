#pragma once
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkQImageToImageSource.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkInformationVector.h>
#include <vtkVectorText.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyLine.h>
#include <vtkFollower.h>

#include "core/TimeHelper.hpp"
#include "core/LineCollection.hpp"
#include "core/DataHelper.hpp"

#include "VisualizationDataLoader.h"

#include "geometry_visualization.h"



vtkSmartPointer<vtkActor> GeometryVisualization::create_map_actor(const jet_vis::VIEW& view) {
	/*
		Visualized the map for the respective hemisphere. The vtkImageData is transformed to a polydata with the vtkImageDataGeometryFilter-
	*/

	//Adding the map to the volume
	QImage map = QImage();
	/*
		Map is from:
		https://neo.sci.gsfc.nasa.gov/view.php?datasetId=SRTM_RAMP2_TOPO
	*/
	if (view == jet_vis::VIEW::POLAR_SOUTH) {
		std::string p = "mapSouth.jpg";
		map.load(p.c_str());
	}
	else if (view == jet_vis::VIEW::POLAR_NORTH) {
		std::string p = "mapNorth.jpg";
		map.load(p.c_str());
	}
	else if (view == jet_vis::VIEW::CLASSIC || view == jet_vis::VIEW::GLOBE) {
		std::string p = "map.jpg";
		map.load(p.c_str());
	}
	//QImage mapScaled = map.scaled(QSize(3600, 1800));
	//QImage mapMirrored = mapScaled.mirrored(false, false);
	vtkSmartPointer<vtkQImageToImageSource> transform = vtkSmartPointer<vtkQImageToImageSource>::New();
	transform->SetQImage(&map);
	transform->Update();
	vtkSmartPointer<vtkImageData> mapData = vtkSmartPointer<vtkImageData>::New();
	mapData = transform->GetOutput();
	mapData->SetSpacing(0.1, 0.1, 1);


	vtkSmartPointer<vtkImageDataGeometryFilter> imageDataGeometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	vtkSmartPointer<vtkInformationVector> v = vtkSmartPointer<vtkInformationVector>::New();
	imageDataGeometryFilter->SetInputData(mapData);
	imageDataGeometryFilter->Update();

	vtkSmartPointer<vtkPolyData> polyData = imageDataGeometryFilter->GetOutput();
	size_t test = polyData->GetPoints()->GetNumberOfPoints();

	if (view == jet_vis::VIEW::POLAR_NORTH) {
		vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
		for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
			double* p = points->GetPoint(i);
			points->SetPoint(i, p[0], p[1] + 90, p[2]);
		}
	}
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetOpacity(.5);
	actor->SetPosition(0., 0., -1.);
	actor->GetProperty()->ShadingOff();
	actor->GetProperty()->LightingOff();

	return actor;
}
vtkSmartPointer<vtkTextActor> GeometryVisualization::create_date_label_follower(const size_t& time, VisualizationDataLoader* op) {
	/*
		Creates a label actor displaying the date of the current data.
	*/
	std::string date = TimeHelper::convertHoursToDate(time, DataHelper::getDataStartDate());
	std::string date_formatted = "Date: " + date.substr(0, 4) + "." + date.substr(4, 2) + "." + date.substr(6, 2) + "_" + date.substr(9, 2);
	vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
	textSource->SetText(date_formatted.c_str());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(textSource->GetOutputPort());

	vtkSmartPointer<vtkFollower> follower = vtkSmartPointer<vtkFollower>::New();
	follower->SetMapper(mapper);
	follower->GetProperty()->SetColor(1.0, 0, 0);
	follower->SetScale(10);
	follower->SetPosition(130, 200, 100);

	vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
	textActor->SetInput(date_formatted.c_str());
	textActor->SetPosition2(10, 40);
	textActor->GetTextProperty()->SetFontSize(24);
	textActor->GetTextProperty()->SetColor(0.0, 0.0, 0.0);

	return textActor;
}
vtkSmartPointer<vtkActor> GeometryVisualization::create_points_actor(vtkSmartPointer<vtkPoints> points, double r, double g, double b, double radius = 2.5) {
	vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
	grid->SetPoints(points);

	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
	glyph3D->SetInputData(grid);
	glyph3D->SetSourceConnection(sphereSource->GetOutputPort());

	sphereSource->SetRadius(radius);
	sphereSource->SetPhiResolution(20);
	sphereSource->SetThetaResolution(20);
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(glyph3D->GetOutputPort());
	mapper->Update();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);
	return actor;
}

vtkSmartPointer<vtkActor> GeometryVisualization::create_lines_actor(const LineCollection& lines, double r, double g, double b) {
	std::vector<Line3d> lines_vec = lines.getLinesInVectorOfVector();
	std::vector<vtkSmartPointer<vtkActor>> result = std::vector<vtkSmartPointer<vtkActor>>(lines_vec.size());

	double spacing[] = {
	0.5,// (179.5 + 180)/719 = 0.5
	0.5,// (90 + 90)/ 360 = 0.5
	1 };

	vtkSmartPointer<vtkPoints> line_points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> line_cells = vtkSmartPointer<vtkCellArray>::New();

	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
	size_t point_counter = 0;

	for (size_t i = 0; i < lines_vec.size(); i++) {
		polyLine->GetPointIds()->SetNumberOfIds(lines_vec[i].size());
		for (size_t j = 0; j < lines_vec[i].size(); j++) {
			Vec3d p = lines_vec[i][j];
			line_points->InsertNextPoint(p[0] * spacing[0], p[1] * spacing[1], p[2] * spacing[2]);
			polyLine->GetPointIds()->SetId(j, point_counter);
			point_counter++;
		}
		line_cells->InsertNextCell(polyLine);
	}

	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	polyData->SetPoints(line_points);
	polyData->SetLines(line_cells);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(r, g, b);

	return actor;
}
