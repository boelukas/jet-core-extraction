#include "JetWidget.h"
#include "core/PredictorCorrector.hpp"

#include <vtkNamedColors.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <stdlib.h>

JetWidget::JetWidget(std::string _dataName, int _time, DataLoader_Amira* op)
	:ISceneWidget(_dataName, _time, op)
{
	CreateTestScene();
}

void JetWidget::CreateTestScene() {
	vtkSmartPointer<vtkImageData> volumeData = op->loadVtkImageData(dataName, time, true);
	minDataValue = volumeData->GetScalarRange()[0];
	maxDataValue = volumeData->GetScalarRange()[1];
	dimensions = volumeData->GetDimensions();

	PredictorCorrector pc = PredictorCorrector(op);
	Vec3d seed1 = Vec3d({ 0, 60, 70 });
	Vec3d seed2 = Vec3d({ 20, 70, 70 });
	Vec3d seed3 = Vec3d({ 0, 10, 70 });


	std::vector<Vec3d> line1 = pc.findJet(seed1, 1000);
	std::vector<Vec3d> line2 = pc.findJet(seed2, 1000);
	std::vector<Vec3d> line3 = pc.findJet(seed3, 1000);


	vtkSmartPointer<vtkActor> actor = createLineActor(line1, Vec3f({ 0.2f, 0.2f, 0.8f }));
	vtkSmartPointer<vtkActor> actor2 = createLineActor(line2, Vec3f({ 0.2f, 0.2f, 0.8f }));
	vtkSmartPointer<vtkActor> actor3 = createLineActor(line3, Vec3f({ 0.2f, 0.2f, 0.8f }));



	vtkNew<vtkNamedColors> colors;
	vtkNew<vtkRenderer> renderer;
	renderer->AddActor(actor);
	renderer->AddActor(actor2);
	renderer->AddActor(actor3);



	renderer->SetBackground(colors->GetColor3d("White").GetData());
	RenderBackground(renderer.GetPointer());
}
vtkSmartPointer<vtkActor> JetWidget::createLineActor(const std::vector<Vec3d>& line, const Vec3f& color)
{
	// create polyline vertices
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	for (const Vec3f& vertex : line)
		points->InsertNextPoint(vertex[0], vertex[1], vertex[2]);

	// create polyline indices
	vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
	polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
	for (unsigned int i = 0; i < points->GetNumberOfPoints(); i++)
		polyLine->GetPointIds()->SetId(i, i);

	// Create a cell array to store the lines in and add the lines to it
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	cells->InsertNextCell(polyLine);

	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	polyData->SetPoints(points);	// Add the points to the dataset
	polyData->SetLines(cells);			// Add the lines to the dataset

	// Setup actor and mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polyData);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color[0], color[1], color[2]);
	actor->GetProperty()->SetLineWidth(2);
	return actor;
}