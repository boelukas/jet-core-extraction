#pragma once
#include <sstream>
#include <fstream>

#include "DataHelper.hpp"
#include "TimeHelper.hpp"
#include "NetCDF.hpp"

#include "Wcb.hpp"

Wcb::Wcb(const size_t& time,
	const double& horizontal_filter,
	const double& vertical_filter,
	const size_t& points_before,
	const size_t& points_after)
	:time_(time),
	horicontal_filter_(horizontal_filter),
	vertical_filter_(vertical_filter),
	n_backwards_extension_(points_before),
	n_forward_extension_points_(points_after) {
	std::vector<std::string> attributes = { "time" };
	DataHelper::loadLineCollection("Wcb", time, attributes, time_trajetories_month_);
	std::vector<std::string> jet_attributes = {};
	DataHelper::loadLineCollection("Jet", time, jet_attributes, jet_);
	jet_cloud_.pts = jet_.getAllPointsInVector();
	jet_tree_ = new KdTree3d(3, jet_cloud_, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	jet_tree_->buildIndex();

	pointsAndExtensions();
}
Wcb::~Wcb() {
	time_trajetories_month_.clear();
	jet_.clear();
}
void Wcb::pointsAndExtensions() {
	std::vector<float> time_attribute = time_trajetories_month_.getAttributeByName("time");

	std::vector<std::vector<Vec3d>> time_trajectories_vec = time_trajetories_month_.getLinesInVectorOfVector();
	size_t hour_of_month = TimeHelper::convertHoursToHoursOfMonth(time_, DataHelper::getDataStartDate());
	for (int i = 0; i < time_attribute.size(); i++) {
		if (time_attribute[i] == hour_of_month) {
			Vec2i v = time_trajetories_month_.getLineVertexPosition(i);
			Vec3d pos = time_trajetories_month_.getPoint(i);
			if (conditionDistance(pos)) {
				trajectories_at_time_step_.push_back(trimLineAroundIndex(time_trajectories_vec[v[0]], v[1]));
				points_at_time_step_.push_back(pos);
				points_at_time_step_indexes_.push_back(v[1]);
			}
		}
	}
}
bool Wcb::conditionDistance(Vec3d pos) const {
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t nVerticalMatches = jet_tree_->radiusSearch(&pos[0], vertical_filter_ / 10.0, ret_matches, params);
	if (ret_matches.size() > 0) {
		for (int i = 0; i < ret_matches.size(); i++) {
			Vec3d p = jet_cloud_.pts[ret_matches[i].first];
			if (horicontalDistance(p, pos) < horicontal_filter_) {
				return true;
			}
		}
	}
	const size_t nHoricontalMatches = jet_tree_->radiusSearch(&pos[0], horicontal_filter_ * 2.0, ret_matches, params);
	if (ret_matches.size() > 0) {
		for (int i = 0; i < ret_matches.size(); i++) {
			Vec3d p = jet_cloud_.pts[ret_matches[i].first];
			if (verticalDistance(p, pos) < vertical_filter_) {
				return true;
			}
		}
	}

	return false;
}
Line3d Wcb::trimLineAroundIndex(const Line3d& line, int index) const {
	Line3d res;

	int start = 0;
	int end = line.size() - 1;
	if (index >= n_backwards_extension_) {
		start = index - n_backwards_extension_;
	}
	if (end - index >= n_forward_extension_points_) {
		end = index + n_forward_extension_points_;
	}
	for (int i = start; i <= end; i++) {
		res.push_back(line[i]);
	}
	return res;
}

double Wcb::horicontalDistance(Vec3d a, Vec3d b) const {
	return std::sqrt(std::pow(b[0] - a[0], 2) + std::pow(b[1] - a[1], 2)) * 0.5;
}
double Wcb::verticalDistance(Vec3d a, Vec3d b) const {
	return std::abs(a[2] - b[2]) * 10;
}
LineCollection Wcb::getWcbLines() const {
	LineCollection lc;
	lc.setData(trajectories_at_time_step_);
	return lc;
}
Line3d  Wcb::getWcbPoints() const {
	return points_at_time_step_;
}
void Wcb::setParameters(double horicontal_filter, double vertical_filter, size_t n_backwards_extension, size_t n_forward_extension_points) {
	horicontal_filter_ = horicontal_filter;
	vertical_filter_ = vertical_filter;
	n_backwards_extension_ = n_backwards_extension;
	n_forward_extension_points_ = n_forward_extension_points;
}

void Wcb::readInputFileAndExportAsLineCollection(const std::string& srcpath, const std::string& destpath, LineCollection& lineCol) {
	NetCDF::Info info;
	//std::string path = "C:\\Users\\Lukas\\Documents\\BachelorThesis\\GitProject\\lcy_201609";
	std::vector<std::vector<Vec3d>> lines = std::vector<std::vector<Vec3d>>();
	std::vector<float> PSaxisValues = DataHelper::loadFloatArray("ps");
	std::vector<float> lon_array = DataHelper::loadFloatArray("lon");
	std::vector<float> lat_array = DataHelper::loadFloatArray("lat");


	lines.push_back(std::vector<Vec3d>());


	std::ifstream infile(srcpath.c_str());
	std::string line;
	size_t header_size = 5;
	size_t time_steps_to_take = 48;
	size_t line_count = 0;
	size_t label_count = 0;
	float lon_old = -1;
	int count_time_0 = 0;
	std::vector<int> zeros = std::vector<int>();
	std::vector<float> times;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		float time, lon, lat, p, dist, label;
		if (line_count >= header_size) {
			if (!(iss >> time >> lon >> lat >> p >> dist >> label)) {
				//if (label_count == 3) { break; }
				label_count++;
				lines.push_back(std::vector<Vec3d>());
				lon_old = -1;
				time_steps_to_take = 48;

				continue;
			}

			float lon_idx = CoordinateConverter::indexOfValueInArray(lon_array, lon, false);
			float lat_idx = CoordinateConverter::indexOfValueInArray(lat_array, lat, false);
			float p_idx = CoordinateConverter::indexOfValueInArray(PSaxisValues, p, true);

			if (time == 0) {
				count_time_0++;
				zeros.push_back(label_count);
			}
			if (lon_old != -1 && std::abs(lon_old - lon_idx) >= 100 && time_steps_to_take > 0) {
				label_count++;
				lines.push_back(std::vector<Vec3d>());
				lines[label_count].push_back(Vec3d({ lon_idx, lat_idx, p_idx }));
				times.push_back(time);
				lon_old = lon_idx;
				time_steps_to_take--;
			}
			else {
				if (time_steps_to_take > 0) {
					lines[label_count].push_back(Vec3d({ lon_idx, lat_idx, p_idx }));
					times.push_back(time);
					lon_old = lon_idx;
					time_steps_to_take--;
				}
			}
		}
		line_count++;
	}

	std::vector<std::vector<Vec3d>> lines_zero = std::vector<std::vector<Vec3d>>(zeros.size());
	for (int i = 0; i < lines_zero.size(); i++) {
		lines_zero[i] = lines[zeros[i]];
	}
	lineCol.setData(lines);
	lineCol.addAttribute(times, "time");
	lineCol.Export(destpath.c_str());
}
