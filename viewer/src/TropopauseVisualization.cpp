
#pragma once
#include <vtkPolyData.h>
#include <vtkConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkActor.h>
#include <vtkFlyingEdges3D.h>
#include <vtkProperty.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkSmoothPolyDataFilter.h>

#include "VisualizationDataLoader.h"

#include "TropopauseVisualization.h"

TropopauseVisualization::TropopauseVisualization(VisualizationDataLoader* dl, const size_t& time)
	:time_(time),
	isoValue_(2.0) {
	data_ = dl->loadVtkImageData(VisualizationDataLoader::CACHESLOT::TROPO, "PV", time_);
}


vtkSmartPointer<vtkActor> TropopauseVisualization::getActor(const jet_vis::VIEW& view) {
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(extract_PolyData(view));

	mapper->SetScalarVisibility(false);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	//actor->GetProperty()->SetOpacity(0.6);//0.6
	actor->GetProperty()->SetOpacity(1);//0.6
	actor->GetProperty()->SetColor(1.0, 1.0, 1.0);

	return actor;
}

vtkSmartPointer<vtkPolyData> TropopauseVisualization::extract_PolyData(const jet_vis::VIEW& view) {
	//Timer timer;
	//timer.tic();

	int* dimensions = data_->GetDimensions();
	int thresh = 30;
	for (int i = 0; i < dimensions[0]; i++) {
		for (int j = 0; j < dimensions[1]; j++) {
			for (int k = 0; k < dimensions[2]; k++) {
				float* scalar = (float*)data_->GetScalarPointer(i, j, k);
				if (k < thresh) {
					scalar[0] = 0;
				}
			}
		}
	}
	auto gaussFilter = vtkSmartPointer< vtkImageGaussianSmooth>::New();
	gaussFilter->SetInputData(data_);
	gaussFilter->SetStandardDeviation(3);
	gaussFilter->SetDimensionality(3);
	gaussFilter->SetRadiusFactor(5);
	gaussFilter->Update();

	vtkSmartPointer<vtkFlyingEdges3D> mc1 = vtkSmartPointer<vtkFlyingEdges3D>::New();
	//mc1->SetInputData(data_);
	mc1->SetInputConnection(gaussFilter->GetOutputPort());
	mc1->SetValue(0, isoValue_);

	vtkSmartPointer<vtkConnectivityFilter> connectivityFilter1 = vtkSmartPointer<vtkConnectivityFilter>::New();
	connectivityFilter1->SetInputConnection(mc1->GetOutputPort());
	connectivityFilter1->SetExtractionModeToLargestRegion();
	connectivityFilter1->Update();
	vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper1->SetInputConnection(connectivityFilter1->GetOutputPort());
	mapper1->SetScalarVisibility(false);

	vtkSmartPointer<vtkPolyData> mc_data1 = mapper1->GetInput();

	vtkSmartPointer<vtkFlyingEdges3D> mc2 = vtkSmartPointer<vtkFlyingEdges3D>::New();
	//mc2->SetInputData(data_);
	mc2->SetInputConnection(gaussFilter->GetOutputPort());
	mc2->SetValue(0, -isoValue_);

	vtkSmartPointer<vtkConnectivityFilter> connectivityFilter2 = vtkSmartPointer<vtkConnectivityFilter>::New();
	connectivityFilter2->SetInputConnection(mc2->GetOutputPort());
	connectivityFilter2->SetExtractionModeToLargestRegion();
	connectivityFilter2->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper2->SetInputConnection(connectivityFilter2->GetOutputPort());
	mapper2->SetScalarVisibility(false);

	vtkSmartPointer<vtkPolyData> mc_data2 = mapper2->GetInput();

	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	switch (view) {
	case(jet_vis::VIEW::CLASSIC):
		appendFilter->AddInputData(mc_data1);
		appendFilter->AddInputData(mc_data2);
		break;
	case(jet_vis::VIEW::GLOBE):
		appendFilter->AddInputData(mc_data1);
		appendFilter->AddInputData(mc_data2);
		break;
	case(jet_vis::VIEW::POLAR_NORTH):
		appendFilter->AddInputData(mc_data1);
		break;
	case(jet_vis::VIEW::POLAR_SOUTH):
		appendFilter->AddInputData(mc_data2);
		break;
	}


	vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
	cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
	cleanFilter->Update();
	//timer.toc();

	/*auto polySmooth = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	polySmooth->SetInputConnection(cleanFilter->GetOutputPort());
	polySmooth->SetNumberOfIterations(15);
	polySmooth->SetRelaxationFactor(0.1);
	polySmooth->FeatureEdgeSmoothingOn();
	polySmooth->BoundarySmoothingOn();
	polySmooth->Update();*/

	return cleanFilter->GetOutput();
}
