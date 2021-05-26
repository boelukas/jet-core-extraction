#pragma once
#include <vtkPoints.h>
#include <vtkActor.h>

#include "core/JetStream.hpp"

#include "VisualizationHelper.h"

class VisualizationDataLoader;

class JetSplitMergeActor
{
public:
	JetSplitMergeActor(const size_t& time, VisualizationDataLoader* dataloader, const jet_vis::VIEW& view, JetStream* jet);
	vtkSmartPointer<vtkActor> getMergeActor() { return _mergeActor; };
	vtkSmartPointer<vtkActor> getSplitActor() { return _splitActor; };
private:
	size_t _time;
	LineCollection jet;
	const jet_vis::VIEW& view_;
	vtkSmartPointer<vtkPoints> _mergePoints;
	vtkSmartPointer<vtkPoints> _splitPoints;
	vtkSmartPointer<vtkActor> _mergeActor;
	vtkSmartPointer<vtkActor> _splitActor;

	void computeSplitMergePoints();
};
