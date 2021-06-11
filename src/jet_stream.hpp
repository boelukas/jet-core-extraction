#pragma once
#include <mutex>

#include "era_grid.hpp"
#include "line_collection.hpp"

class JetStream
{
public:
	struct JetParameters {
		//Changable by user
		int n_predictor_steps = 1;//1
		int n_corrector_steps = 5;//100
		int max_steps_below_speed_thresh = 100;//10
		double wind_speed_threshold = 40;//40
		double integration_stepsize = 0.04;//0.05
		double ps_min_val = 190;//225
		double ps_max_val = 350;//320

		//Not Changable
		double split_merge_threshold = 0.1;
		int stopping_criteria_jet = 10000;//1000
		double min_jet_distance = 5;
    double ps_min_tracing = std::max(ps_min_val - 100, 10.);
    double ps_max_tracing = std::min(ps_max_val + 100, 1040.);
    double kdtree_radius = 5.5; //20
  };

	struct WindMagComparator {
		EraScalarField3f* wind_magnitude;
		std::vector<float> ps_axis_values;
		Vec3d ToDomainCoordinates(Vec3d& p) const {
			return Vec3d({ p[0], p[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values, p[2], true) });
		}
		bool operator() (Vec3d a, Vec3d b) const {
			return wind_magnitude->Sample(ToDomainCoordinates(a)) > wind_magnitude->Sample(ToDomainCoordinates(b));
		};
	};

	enum class HEMISPHERE { BOTH, NORTH, SOUTH };

	JetStream(const size_t& time, const JetParameters& jet_params, const bool& ps3d_preprocessed);
	~JetStream();
  void DeletePreviousJet();

	void GenerateJetSeeds();


	LineCollection GetJetCoreLines();

  const size_t GetTime() const {return time_;}


  void SetPreviousJet(JetStream *previous_jet){previous_jet_ = previous_jet; }

private:
	const bool ps3d_preprocessed_;
	LineCollection jet_core_lines_;
  JetStream* previous_jet_;

	std::vector<RegScalarField3f*> fields_;
	EraVectorField3f* wind_direction_normalized_;
	EraVectorField3f* grad_wind_magnitude_;
	EraScalarField3f* wind_magnitude_;
	EraScalarField3f* wind_magnitude_smooth_;
	RegScalarField3f* ps3d_;
	std::vector<float> ps_axis_values_;
	KdTree3d* jet_kd_tree;
	PointCloud3d jet_point_cloud;
	Line3d _seeds;

	bool _usePreviousTimeStep;
	bool _usePreprocessedPreviousJet;
	std::mutex mtx_;

	struct LineDistance {
		double distance;
		Vec3d previous;
	};
	size_t time_;
	const JetParameters jet_params_;
	WindMagComparator wind_magnitude_comparator_;


	void ComputeJetCoreLines();

	Line3d GetPreviousTimeStepSeeds();

	std::vector<Line3d> FindJet(Line3d& seeds);

	void Trace(Line3d& line, std::set<Vec3d, decltype(wind_magnitude_comparator_)>& seeds_set, KdTree3d* seeds_kd_tree, PointCloud3d& seeds_point_cloud, bool inverse);

	void RemoveWrongStartUps(Line3d& jet_lines) const;

	void CutWeakEndings(Line3d& jet);

	Vec3d PredictorStepRK4(const Vec3d& pos, double dt) const;

	Vec3d PredictorStepRK4Inverse(const Vec3d& pos, double dt) const;

	Vec3d CorrectorStepRK4(const Vec3d& pos, const double& dt) const;

	Vec3d PredictorCorrectorStep(const Vec3d& pos) const;

	Vec3d InversePredictorCorrectorStep(const Vec3d& pos) const;

	bool ConditionDomain(const Vec3d& point) const;

	bool ConditionWindMagnitude(const Vec3d& point, int& count) const;

	LineCollection FilterFalsePositives(const LineCollection& jet) const;

	/*
		Helper functions.
	*/
	void UpdateKdTree(const Line3d& new_line);
	Vec3d FindClosestJetPoint(const double& radius, const Vec3d& point) const;
	Line3d FindPointsWithinRadius(const KdTree3d* kd_tree, const PointCloud3d& point_cloud, const double& radius, const Vec3d& point) const;

	double GetLineDistance(const Line3d& line) const {
		double res = 0;
		for (int i = 1; i < line.size(); i++) {
			Vec3d v = (line[i] - line[i - 1]);
			res += std::sqrt(std::pow(v[0], 2) + std::pow(v[1], 2));
		}
		return res;
	}
	Vec3d ToIndexCoordinates(Vec3d& p) const {
		return Vec3d({ p[0], p[1], CoordinateConverter::IndexOfValueInArray(ps_axis_values_, p[2], true) });
	}
	Vec3d ToDomainCoordinates(Vec3d& p) const {
		return Vec3d({ p[0], p[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, p[2], true) });
	}
};
