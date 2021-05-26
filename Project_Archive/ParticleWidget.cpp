#include "ParticleWidget.h"
#include "core/ParticleTracer.hpp"

#include <vtkNamedColors.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <stdlib.h>

ParticleWidget::ParticleWidget(std::string _dataName, int _time, DataLoader_Amira* op)
	:ISceneWidget(_dataName, _time, op)
{
	CreateTestScene();
}

void ParticleWidget::CreateTestScene() {
	vtkSmartPointer<vtkImageData> volumeData = op->loadVtkImageData(dataName, time, true);
	minDataValue = volumeData->GetScalarRange()[0];
	maxDataValue = volumeData->GetScalarRange()[1];
	dimensions = volumeData->GetDimensions();

	ParticleTracer pt = ParticleTracer(op, BoundingBox3d(Vec3i({0,0,0}), Vec3i({ dimensions[0]/2, dimensions[1]/2, dimensions[2] })));
	std::vector<Vec3f> streamLine;
	Vec3f seed({ 360, 50, 50});

	std::vector<Vec3f> seeds;
	for (int i = 0; i < 5; i++) {
		Vec3f s({0,0,0});
		s[0] = std::rand() % 361;
		s[1] = std::rand() % 181;
		s[2] = std::rand() % 100;
		seeds.push_back(s);
	}

	float dt = 0.01;
	float tau = 4* PI;

	std::vector< vtkSmartPointer<vtkActor>> actors;
	for (int i = 0; i < 5; i++) {
		std::vector<Vec3f> sl;
		pt.traceParticle(seeds[i], dt, tau, sl);
		vtkSmartPointer<vtkActor> actor = createLineActor(sl, Vec3f({ 0.2f, 0.2f, 0.8f }));
		actors.push_back(actor);
	}

	//vtkSmartPointer<vtkActor> actor = createLineActor(streamLine, Vec3f({ 0.2f, 0.2f, 0.8f }));

	vtkNew<vtkNamedColors> colors;
	vtkNew<vtkRenderer> renderer;
	//renderer->AddActor(actor);
	for (int i = 0; i < 5; i++) {
		renderer->AddActor(actors[i]);
	}
	renderer->SetBackground(colors->GetColor3d("White").GetData());
	RenderBackground(renderer.GetPointer());
}
vtkSmartPointer<vtkActor> ParticleWidget::createLineActor(const std::vector<Vec3f>& line, const Vec3f& color)
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