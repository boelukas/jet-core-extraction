#pragma once
#include <mutex>

#include "era_grid.hpp"
#include "line_collection.hpp"

class JetStream
{
public:
	struct JetParameters {
		//Global parameters
		//Changable by user
		int n_predictor_steps = 1;//1
		int n_corrector_steps = 5;//100
		int max_steps_below_speed_thresh = 100;//10
		double wind_speed_threshold = 40;//40
		double integration_stepsize = 0.04;//0.05
		double kdtree_radius = 5.5;//20
		double ps_min_val = 190;//225
		double ps_max_val = 350;//320
		double ps_min_tracing = 90;
		double ps_max_tracing = 450;


		//Not Changable
		double split_merge_threshold = 0.1;
		int stopping_criteria_jet = 10000;//1000
		double min_jet_distance = 5;
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

	/*
		Iterates through the wind Magnitude scalar field to find local maximas.
		If the time step is not zero, the maximas of the core lines from the last time step will be taken as seeds.
	*/
	void GenerateJetSeeds();


	/*
		Getters
	*/
	LineCollection GetJetCoreLines();

  const size_t GetTime() const {return time_;}
	std::vector<float> GetPsAxis() {
		int stepSize = 10;
		double PS_domain_min = 10.;
		int pressureResolution = 104;

		std::vector<float> pressure(pressureResolution);
		pressure[0] = PS_domain_min;
		for (size_t i = 1; i < pressureResolution; i++) {
			pressure[i] = PS_domain_min + i * stepSize;
		}
		return pressure;
	}

	/*
		Setters
	*/
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
	/*
		Computes the previous time steps core lines and uses the local maximas from the core lines as seeds.
	*/
	Line3d GetPreviousTimeStepSeeds();

	/*
		Takes the seeds and finds the jet core lines with help of a Predictor Corrector approach.

		Against coordinate confusion:
		pos_ps=(x, y, pressure[hPs]) or pos_idx = (x, y, idx)
		To display elements in the scene, we use pos_idx. Because or z axis is flipped, this means that high pressure values have a low index.
		To compute anything, especially in combination with vector fields, we use pos_ps=(x, y, pressure[hPa]). x and y are always the lonitude and latitude index.
		This way, one avoids having to flip the gradient because the Sample function of the era field never flips the z axis. Only the resample function flips it.
		Basically the flipping only happens at the very end after all computation is already finished.
	*/
	std::vector<Line3d> FindJet(Line3d& seeds);

	/*
		Traces a line with predictor corrector steps. Deactivates closeby seeds. If the inverse flag is set, the tracing goes in the inverse wind direction.
	*/
	void Trace(Line3d& line, std::set<Vec3d, decltype(wind_magnitude_comparator_)>& seeds_set, KdTree3d* seeds_kd_tree, PointCloud3d& seeds_point_cloud, bool inverse);

	/*
		In case the seed point is not exactly on the core line, the beginning of the traced line is removed until it is alligned with the core line.
	*/
	void RemoveWrongStartUps(Line3d& jet_lines) const;

	/*
		Removes the endings of the core line if they are below threshold.
	*/
	void CutWeakEndings(Line3d& jet);

	/*
		Perfoms a Runke-Kutta 4 step in the wind direction.
	*/
	Vec3d PredictorStepRK4(const Vec3d& pos, double dt) const;

	/*
	Performs a step in the opposite wind direction.
	*/
	Vec3d PredictorStepRK4Inverse(const Vec3d& pos, double dt) const;
	/*
		Performs a Runge Kutta 4 Step in direction u.
		u is needed for the case when the wind direction and the gradient point in oposite directions.
		It could happen in this case, that the algorithm walks backwards to a strong local maximum.
		u is the projection of the gradient on the plane perpendicular to v.

		pos = (long_idx, lat_idx, pressure(hPa))
		dt = the integration stepsize.
	*/
	Vec3d CorrectorStepRK4(const Vec3d& pos, const double& dt) const;

	/*
		Performs predictor corrector steps.
	*/
	Vec3d PredictorCorrectorStep(const Vec3d& pos) const;

	/*
		Performs inverse predictor corrector steps.
	*/
	Vec3d InversePredictorCorrectorStep(const Vec3d& pos) const;

	/*
		Condition to make sure the line stays in the domain.
	*/
	bool ConditionDomain(const Vec3d& point) const;
	/*
		Condition that the Jet core is only allowed to stay for max_steps_below_speed_thresh steps below threshold.
	*/
	bool ConditionWindMagnitude(const Vec3d& point, int& count) const;

	/*
		Filter to remove tropical storms which have wind magnitude over the threshold but evolve to the surface.
	*/
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
