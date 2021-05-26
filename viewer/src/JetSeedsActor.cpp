#pragma once
#include <vtkDoubleArray.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkTubeFilter.h>
#include <vtkPolyDataMapper.h>

#include "core/JetStream.hpp"

#include "VisualizationDataLoader.h"
#include "geometry_visualization.h"

#include "JetSeedsActor.h"


JetSeedsActor::JetSeedsActor(VisualizationDataLoader* dl, const size_t& time, const JetStream::JetParameters& jetParams, JetStream* jetStream)
	:_time(time),
	_dl(dl),
	_jetParams(jetParams),
	_seedPoints(vtkSmartPointer<vtkPoints>::New())
{
	std::vector<Vec3d> seeds_vec;
	if (jetStream) {
		seeds_vec = jetStream->getSeeds();
	}
	else {
		JetStream js = JetStream(_time, _jetParams, true);
		if (_time != 0) {
			js.setUsePreviousTimeStep();
		}
		js.generateJetSeeds();
		seeds_vec = js.getSeeds();
	}
	
	for (int i = 0; i < seeds_vec.size(); i++) {
		double p[] = { seeds_vec[i][0] * 0.5, seeds_vec[i][1] * 0.5, seeds_vec[i][2] };
		_seedPoints->InsertNextPoint(p);
	}
	_seedAct = GeometryVisualization::create_points_actor(_seedPoints, 15 / 255., 145 / 255., 21 / 255., 1.2);

}
JetSeedsActor::~JetSeedsActor() {

}