#pragma once
#include "core/DataHelper.hpp"

#include "VisualizationDataLoader.h"
#include "geometry_visualization.h"

#include "JetSplitMergeActor.h"

JetSplitMergeActor::JetSplitMergeActor(const size_t& time, VisualizationDataLoader* dataloader, const jet_vis::VIEW& view, JetStream* precomuted_jet) :
	_time(time),
	view_(view),
	_mergePoints(vtkSmartPointer<vtkPoints>::New()),
	_splitPoints(vtkSmartPointer<vtkPoints>::New())
{
	if (precomuted_jet) {
		jet = precomuted_jet->getJetCoreLines();
	}
	else {
		std::vector<std::string> attributes;
		DataHelper::loadLineCollection("Jet", time, attributes, jet);
	}

	computeSplitMergePoints();
	_mergeActor = GeometryVisualization::create_points_actor(_mergePoints, 197 / 255., 27 / 255., 138 / 255., 2.5);
	_splitActor = GeometryVisualization::create_points_actor(_splitPoints, 0.0, 1.0, 0.0, 2.5);
	jet.clear();
}

void JetSplitMergeActor::computeSplitMergePoints() {
	double close_by_point_threshold = 2.5;
	std::vector<std::vector<Vec3d>> jet_vec = jet.getLinesInVectorOfVector();
	Line3d split_vec;
	Line3d merge_vec;
	Vec3dSet split_set;
	Vec3dSet merge_set;
	double spacing[] = { 0.5, 0.5, 1 };
	std::vector<Vec3dSet> jet_set;
	for (int i = 0; i < jet_vec.size(); i++) {
		jet_set.push_back(Vec3dSet());
		for (int j = 0; j < jet_vec[i].size(); j++) {
			jet_set[jet_set.size() - 1].insert(jet_vec[i][j]);
		}
	}
	for (int i = 0; i < jet_vec.size(); i++) {
		if (jet_vec[i].size() == 0) { continue; }
		Vec3d begin = jet_vec[i][0];
		double begin_d[] = { begin[0] * 0.5, begin[1] * 0.5, begin[2] };
		Vec3d end = jet_vec[i][jet_vec[i].size() - 1];
		double end_d[] = { end[0] * 0.5, end[1] * 0.5, end[2] };

		for (int j = 0; j < jet_set.size(); j++) {
			if (i != j) {
				auto begin_it = jet_set[j].find(begin);
				auto end_it = jet_set[j].find(end);
				if (begin_it != jet_set[j].end()) {
					split_set.insert(begin);
					split_vec.push_back(begin);
				}
				if (end_it != jet_set[j].end()) {
					merge_set.insert(end);
					merge_vec.push_back(end);
				}
			}
		}
	}
	PointCloud3d merge_cloud;
	merge_cloud.pts = merge_vec;
	KdTree3d* merge_tree = new KdTree3d(3, merge_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	merge_tree->buildIndex();
	for (Vec3d v : split_vec) {
		std::vector<std::pair<size_t, double> >  ret_matches;
		nanoflann::SearchParams params;
		params.sorted = true;
		const size_t nMatches = merge_tree->radiusSearch(&v[0], close_by_point_threshold, ret_matches, params);
		for (auto e : ret_matches) {
			merge_set.erase(merge_cloud.pts[e.first]);
			split_set.erase(v);
		}
	}

	for (auto merge_p : merge_set) {
		double v[] = { merge_p[0] * 0.5, merge_p[1] * 0.5, merge_p[2] };
		_mergePoints->InsertNextPoint(v);
	}
	for (auto split_p : split_set) {
		double v[] = { split_p[0] * 0.5, split_p[1] * 0.5, split_p[2] };
		_splitPoints->InsertNextPoint(v);
	}

}
