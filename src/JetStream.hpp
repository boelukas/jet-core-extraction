#pragma once
#include <mutex>

#include "EraGrid.hpp"
#include "LineCollection.hpp"

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
		EraScalarField3f* windMagnitude;
		std::vector<float> PSaxisValues;
		Vec3d toDomainCoordinates(Vec3d& p) const {
			return Vec3d({ p[0], p[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], true) });
		}
		bool operator() (Vec3d a, Vec3d b) const {
			return windMagnitude->Sample(toDomainCoordinates(a)) > windMagnitude->Sample(toDomainCoordinates(b));
		};
	};

	enum class HEMISPHERE { BOTH, NORTH, SOUTH };

	JetStream(const size_t& time, const JetParameters& jetParams, const bool& PS3D_preprocessed);
	~JetStream();

	/*
		Iterates through the wind Magnitude scalar field to find local maximas.
		If the time step is not zero, the maximas of the core lines from the last time step will be taken as seeds.
	*/
	void generateJetSeeds();

	LineCollection getJetCoreLines();

	/*
		Getters
	*/
	const double& getMaxAttributeValue() const { return maxAttributeVal; }
	const double& getMinAttributeValue() const { return minAttributeVal; }
	Line3d getSeeds() { return _seeds; };
	std::vector<float> getPsAxis() {
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
	void setUsePreviousTimeStep() { _usePreviousTimeStep = true; };
	void setUsePreprocessedPreviousJet() { _usePreprocessedPreviousJet = true; };

private:
	const bool _PS3D_preprocessed;
	LineCollection jet_core_lines;

	std::vector<RegScalarField3f*> fields;
	EraVectorField3f* windDirection_normalized;
	EraVectorField3f* gradWindMagnitude;
	EraScalarField3f* windMagnitude;
	EraScalarField3f* windMagnitudeSmooth;
	RegScalarField3f* PS3D;
	std::vector<float> PSaxisValues;
	KdTree3d* jet_kd_tree;
	PointCloud3d jet_point_cloud;
	Line3d _seeds;

	double minAttributeVal;
	double maxAttributeVal;

	bool _usePreviousTimeStep;
	bool _usePreprocessedPreviousJet;
	std::mutex mtx;

	struct LineDistance {
		double distance;
		Vec3d previous;
	};
	size_t _time;
	const JetParameters _jetParams;
	WindMagComparator cmp;


	void computeJetLines();
	/*
		Computes the previous time steps core lines and uses the local maximas from the core lines as seeds.
	*/
	Line3d getPreviousTimeStepSeeds();

	/*
		Takes the seeds and finds the jet core lines with help of a Predictor Corrector approach.

		Against coordinate confusion:
		pos_ps=(x, y, pressure[hPs]) or pos_idx = (x, y, idx)
		To display elements in the scene, we use pos_idx. Because or z axis is flipped, this means that high pressure values have a low index.
		To compute anything, especially in combination with vector fields, we use pos_ps=(x, y, pressure[hPa]). x and y are always the lonitude and latitude index.
		This way, one avoids having to flip the gradient because the Sample function of the era field never flips the z axis. Only the resample function flips it.
		Basically the flipping only happens at the very end after all computation is already finished.
	*/
	std::vector<Line3d> findJet(Line3d& seeds);

	/*
		Traces a line with predictor corrector steps. Deactivates closeby seeds. If the inverse flag is set, the tracing goes in the inverse wind direction.
	*/
	void trace(Line3d& line, std::set<Vec3d, decltype(cmp)>& seeds_set, KdTree3d* seeds_kd_tree, PointCloud3d& seeds_point_cloud, bool inverse);

	/*
		In case the seed point is not exactly on the core line, the beginning of the traced line is removed until it is alligned with the core line.
	*/
	void removeWrongStartUps(Line3d& jet_lines) const;

	/*
		Removes the endings of the core line if they are below threshold.
	*/
	void cutWeakEndings(Line3d& jet);

	/*
		Perfoms a Runke-Kutta 4 step in the wind direction.
	*/
	Vec3d predictorStepRK4(const Vec3d& pos, double dt) const;

	/*
	Performs a step in the opposite wind direction.
	*/
	Vec3d predictorStepRK4_inverse(const Vec3d& pos, double dt) const;
	/*
		Performs a Runge Kutta 4 Step in direction u.
		u is needed for the case when the wind direction and the gradient point in oposite directions.
		It could happen in this case, that the algorithm walks backwards to a strong local maximum.
		u is the projection of the gradient on the plane perpendicular to v.

		pos = (long_idx, lat_idx, pressure(hPa))
		dt = the integration stepsize.
	*/
	Vec3d correctorStepRK4(const Vec3d& pos, const double& dt) const;

	/*
		Performs predictor corrector steps.
	*/
	Vec3d predictorCorrectorStep(const Vec3d& pos) const;

	/*
		Performs inverse predictor corrector steps.
	*/
	Vec3d inversePredictorCorrectorStep(const Vec3d& pos) const;

	/*
		Condition to make sure the line stays in the domain.
	*/
	bool condition_domain(const Vec3d& point) const;
	/*
		Condition that the Jet core is only allowed to stay for max_steps_below_speed_thresh steps below threshold.
	*/
	bool condition_wind_magnitude(const Vec3d& point, int& count) const;

	/*
		Filter to remove tropical storms which have wind magnitude over the threshold but evolve to the surface.
	*/
	LineCollection filterTropicalStorms(const LineCollection& jet) const;

	/*
		Helper functions.
	*/
	void updateKdTree(const Line3d& new_line);
	Vec3d findClosestJetPoint(const double& radius, const Vec3d& point) const;
	Vec3d findClosestJetPointExcludingLine(const double& radius, const Vec3d& point, const Line3d& line) const;
	Line3d findPointsWithinRadius(const KdTree3d* kd_tree, const PointCloud3d& point_cloud, const double& radius, const Vec3d& point) const;

	double getLineDistance(const Line3d& line) const {
		double res = 0;
		for (int i = 1; i < line.size(); i++) {
			Vec3d v = (line[i] - line[i - 1]);
			res += std::sqrt(std::pow(v[0], 2) + std::pow(v[1], 2));
		}
		return res;
	}
	Vec3d toIndexCoordinates(Vec3d& p) const {
		return Vec3d({ p[0], p[1], CoordinateConverter::indexOfValueInArray(PSaxisValues, p[2], true) });
	}
	Vec3d toDomainCoordinates(Vec3d& p) const {
		return Vec3d({ p[0], p[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], true) });
	}
};
