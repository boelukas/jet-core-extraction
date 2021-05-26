#pragma once
#include <vtkActor.h>

#include "core/JetStream.hpp"

#include "Colors.h"

class VisualizationDataLoader;

class JetSeedsActor
{

public:
	JetSeedsActor(VisualizationDataLoader* _dl, const size_t& time, const JetStream::JetParameters& jetParams, JetStream* jetstream);
	~JetSeedsActor();
	vtkSmartPointer<vtkActor> get() { return _seedAct; };

private:
	const size_t& _time;
	VisualizationDataLoader* _dl;
	const JetStream::JetParameters& _jetParams;
	vtkSmartPointer<vtkPoints> _seedPoints;
	vtkSmartPointer<vtkActor> _seedAct;

};
