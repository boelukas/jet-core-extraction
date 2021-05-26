#pragma once
#include "LineCollection.hpp";

class Wcb
{
public:
	Wcb(const size_t& time,
		const double& horizontal_filter,
		const double& vertical_filter,
		const size_t& points_before,
		const size_t& points_after);
	~Wcb();
	LineCollection getWcbLines() const;
	Line3d getWcbPoints() const;
	void setParameters(double horicontal_filter, double vertical_filter, size_t n_backwards_extension, size_t n_forward_extension_points);
	static void readInputFileAndExportAsLineCollection(const std::string& srcpath, const std::string& destpath, LineCollection& lineCol);

private:
	void pointsAndExtensions();
	bool conditionDistance(Vec3d pos) const;
	Line3d trimLineAroundIndex(const Line3d& line, int index) const;


	// Helpers
	double horicontalDistance(Vec3d a, Vec3d b) const;
	double verticalDistance(Vec3d a, Vec3d b) const;

	// Members
	size_t time_;
	LineCollection time_trajetories_month_;
	LineCollection jet_;
	PointCloud3d jet_cloud_;
	KdTree3d* jet_tree_;
	std::vector<Line3d> trajectories_at_time_step_;
	std::vector<int> points_at_time_step_indexes_;
	Line3d points_at_time_step_;

	// Parameters
	double horicontal_filter_ = 500;
	double vertical_filter_ = 50;
	size_t n_backwards_extension_ = 50;
	size_t n_forward_extension_points_ = 2;
};
