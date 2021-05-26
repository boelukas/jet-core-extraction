#pragma once
#include <vtkNamedColors.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include "VisualizationDataLoader.h"

#include "SceneWidget.h"

SceneWidget::SceneWidget(VisualizationDataLoader* _op) :op(_op) {
	mRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	setRenderWindow(mRenderWindow);
	vtkNew<vtkNamedColors> colors;
	vtkNew<vtkRenderer> renderer;
	renderer->SetBackground(colors->GetColor3d("White").GetData());
	renderWindow()->AddRenderer(renderer);
	renderWindow()->SetWindowName("RenderWindowNoUIFile");
}
void SceneWidget::RenderBackground(bool axes_active) {
	/*
		Renders the background of the scene. Adds the coordinate axes.
		This function should be called as last, after all actors are placed in the scene because this function moves the camera.
	*/
	//vtkSmartPointer<vtkRenderer> renderer = renderWindow()->GetRenderers()->GetFirstRenderer();

	////Begin Axes
	//axes = vtkSmartPointer<vtkCubeAxesActor>::New();
	//std::vector<float> lonAxisLabels = op->loadFloatArray("lon");
	//std::vector<float> latAxisLabels = op->loadFloatArray("lat");
	//std::vector<float> psAxisLabels = op->loadFloatArray("ps");


	//axes->SetCamera(renderer->GetActiveCamera());
	//axes->SetBounds(0, lonAxisLabels.size() / 2.0, 0, latAxisLabels.size() / 2.0, 0, psAxisLabels.size());

	//axes->SetXTitle("Longitude");
	//axes->SetYTitle("Latitude");
	//axes->SetZTitle("Pressure[hPa]");

	//axes->SetXAxisRange(lonAxisLabels[0], lonAxisLabels[lonAxisLabels.size() - 1]);
	//axes->SetYAxisRange(latAxisLabels[0], latAxisLabels[latAxisLabels.size() - 1]);
	////axes->SetZAxisRange(psAxisLabels[0], psAxisLabels[psAxisLabels.size() - 1]);

	//vtkSmartPointer<vtkStringArray> ZAxisLabels = vtkSmartPointer<vtkStringArray>::New();
	//for (int i = psAxisLabels.size() - 1; i >= 0; i--) {
	//	ZAxisLabels->InsertNextValue(std::to_string((int)std::round(psAxisLabels[i])));
	//}
	//for (int i = 0; i < 20; i++) {
	//	ZAxisLabels->InsertNextValue(std::to_string(42));
	//}

	//axes->SetAxisLabels(2, ZAxisLabels);
	//axes->SetFlyModeToStaticTriad();

	//axes->GetTitleTextProperty(0)->SetColor(1.0, 0.0, 0.0);
	//axes->GetLabelTextProperty(0)->SetColor(1.0, 0.0, 0.0);

	//axes->GetTitleTextProperty(1)->SetColor(0.0, 1.0, 0.0);
	//axes->GetLabelTextProperty(1)->SetColor(0.0, 1.0, 0.0);

	//axes->GetTitleTextProperty(2)->SetColor(0.0, 0.0, 1.0);
	//axes->GetLabelTextProperty(2)->SetColor(0.0, 0.0, 1.0);

	//std::cout << renderer->GetNumberOfPropsRendered() << std::endl;
	////renderer->ResetCamera();
	//std::cout << "da" << std::endl;

	//if (axes_active) {
	//	renderer->AddActor(axes);
	//}

	//End Axes
}
void SceneWidget::removeAxes() {
	vtkSmartPointer<vtkRenderer> renderer = renderWindow()->GetRenderers()->GetFirstRenderer();
	renderer->RemoveActor(axes);
}
void SceneWidget::addAxes() {
	RenderBackground(true);
	renderWindow()->Render();
}
