#include <mutex>
#include <limits>

#include "wind_fields.hpp"
#include "data_helper.hpp"
#include "time_helper.hpp"

#include "jet_stream.hpp"

JetStream::JetStream(const size_t& time, const JetParameters& jet_params, const bool& ps3d_preprocessed)
	:time_(time),
	jet_params_(jet_params),
	ps3d_preprocessed_(ps3d_preprocessed),
	ps_axis_values_(DataHelper::GetPsAxis()),
	jet_core_lines_(LineCollection()),
	fields_(std::vector<RegScalarField3f*>()),
	wind_direction_normalized_(nullptr),
	grad_wind_magnitude_(nullptr),
	wind_magnitude_(nullptr),
	wind_magnitude_smooth_(nullptr),
	ps3d_(nullptr),
	mtx_(std::mutex()),
	previous_jet_(nullptr)
{
	WindFields wind_fields;

	fields_ = DataHelper::LoadScalarFields(time_, std::vector<std::string>({ "U", "V", "OMEGA", "T" }));
	ps3d_ = DataHelper::ComputePS3D(TimeHelper::ConvertHoursToDate(time_, DataHelper::GetDataStartDate()), fields_[0]->GetResolution(), fields_[0]->GetDomain());
	wind_direction_normalized_ = wind_fields.GetNormalizedWindDirectionEra(time_, ps3d_, fields_[0], fields_[1], fields_[2]);
	wind_magnitude_ = wind_fields.GetWindMagnitudeEra(time_, ps_axis_values_, ps3d_, fields_[0], fields_[1], fields_[2], fields_[3]);
	wind_magnitude_smooth_ = wind_fields.GetSmoothWindMagnitude(time_, ps_axis_values_, ps3d_, fields_[0], fields_[1], fields_[2], fields_[3]);
	grad_wind_magnitude_ = wind_fields.GetWindMagnitudeGradientEra(time_, ps3d_, fields_[3], wind_magnitude_smooth_->GetField());

	wind_magnitude_comparator_.ps_axis_values = ps_axis_values_;
	wind_magnitude_comparator_.wind_magnitude = wind_magnitude_;
}
JetStream::~JetStream() {
	ps_axis_values_.clear();
	for (size_t i = 0; i < fields_.size(); i++) {
		delete fields_[i];
	}
	fields_.clear();
	delete wind_direction_normalized_->GetField();
	delete wind_direction_normalized_;
	delete wind_magnitude_->GetField();
	delete wind_magnitude_;
	delete wind_magnitude_smooth_->GetField();
	delete wind_magnitude_smooth_;
	delete grad_wind_magnitude_->GetField();
	delete grad_wind_magnitude_;
	delete jet_kd_tree;
	if (!ps3d_preprocessed_) {
		delete ps3d_;
	}
}
void JetStream::DeletePreviousJet()
{
	if (previous_jet_ != nullptr)
	{
		delete previous_jet_;
		previous_jet_ = nullptr;
	}
}

const LineCollection& JetStream::GetJetCoreLines() {
	if (jet_core_lines_.GetNumberOfLines() == 0) {
		ComputeJetCoreLines();
	}
	return jet_core_lines_;
}

void JetStream::ComputeJetCoreLines() {
	GenerateJetSeeds();
	std::vector<Line3d> jet = FindJet(_seeds);
	jet_core_lines_ = LineCollection();
	jet_core_lines_.SetData(jet);
	jet_core_lines_ = FilterFalsePositives(jet_core_lines_);
}

/*
		Iterates through the wind Magnitude scalar field to find local maximas.
		If the previous time step exists, the maximas of the core lines from the last time step will be taken as additional seeds.
*/
void JetStream::GenerateJetSeeds() {
	PointCloud3d prev_jet_cloud{ Line3d() };
	KdTree3d* prev_jet_tree = new KdTree3d(3, prev_jet_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));

	if (previous_jet_ != nullptr) {
		Line3d prev_jet = GetPreviousTimeStepSeeds();
		prev_jet_cloud.pts = prev_jet;
		prev_jet_tree->buildIndex();
		_seeds.insert(_seeds.end(), prev_jet.begin(), prev_jet.end());
	}
	double ps_min_idx = CoordinateConverter::IndexOfValueInArray(ps_axis_values_, (float)jet_params_.ps_min_val, true);
	double ps_max_idx = CoordinateConverter::IndexOfValueInArray(ps_axis_values_, (float)jet_params_.ps_max_val, true);
	size_t num_entries = (size_t)wind_magnitude_smooth_->GetField()->GetResolution()[0] * (size_t)wind_magnitude_smooth_->GetField()->GetResolution()[1] * (size_t)wind_magnitude_smooth_->GetField()->GetResolution()[2];


#pragma omp parallel for schedule(dynamic,24)
	for (int64_t linear_index = 0; linear_index < (int64_t)num_entries; linear_index++) {
		Vec3i coords = wind_magnitude_smooth_->GetField()->GetGridCoord(linear_index);
		if (coords[2] <= ps_min_idx && coords[2] >= ps_max_idx) {
			Vec3d seed_candidate = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) });
			Vec3d up = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) + 10.0 });
			Vec3d down = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) - 10.0 });
			Vec3d left = Vec3d({ (double)coords[0] - 1, (double)coords[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) });
			Vec3d right = Vec3d({ (double)coords[0] + 1, (double)coords[1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) });
			Vec3d front = Vec3d({ (double)coords[0], (double)coords[1] + 1, CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) });
			Vec3d back = Vec3d({ (double)coords[0], (double)coords[1] - 1, CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)coords[2], true) });

			float wind_mag = wind_magnitude_smooth_->Sample(seed_candidate);

			float w_up = wind_magnitude_smooth_->Sample(up);
			float w_down = wind_magnitude_smooth_->Sample(down);
			float w_left = wind_magnitude_smooth_->Sample(left);
			float w_right = wind_magnitude_smooth_->Sample(right);
			float w_front = wind_magnitude_smooth_->Sample(front);
			float w_back = wind_magnitude_smooth_->Sample(back);

			if (wind_mag >= jet_params_.wind_speed_threshold && wind_mag > std::max({ w_up, w_down,w_left, w_right, w_front, w_back })) {
				if (previous_jet_ != nullptr) {
					Line3d closeby_prev_jet_seeds = FindPointsWithinRadius(prev_jet_tree, prev_jet_cloud, jet_params_.kdtree_radius, coords);
					if (closeby_prev_jet_seeds.size() == 0) {
						mtx_.lock();
						_seeds.push_back(coords);
						mtx_.unlock();
					}
				}
				else {
					mtx_.lock();
					_seeds.push_back(coords);
					mtx_.unlock();
				}
			}
		}

	}
	delete prev_jet_tree;
}

Points3d JetStream::GetPreviousTimeStepSeeds()
{
	if (time_ != 0) {
		const LineCollection& jet = previous_jet_->GetJetCoreLines();

		std::vector<Line3d> prev_jet = jet.GetLinesInVectorOfVector();

		Points3d res = Points3d();
		for (auto line : prev_jet) {
			if (line.size() < 3) { continue; }
			for (int i = 1; i < line.size() - 1; i++) {
				double left = wind_magnitude_->Sample(ToDomainCoordinates(line[i - 1ll]));
				double centre = wind_magnitude_->Sample(ToDomainCoordinates(line[i]));
				double right = wind_magnitude_->Sample(ToDomainCoordinates(line[i + 1ll]));
				if (centre > left && centre > right) {
					res.push_back(line[i]);
				}
			}
		}
		return res;
	}
	else {
		return Points3d();
	}
}

/*
	Takes the seeds and finds the jet core lines with help of a Predictor Corrector approach.

	Against coordinate confusion:
	pos_ps=(x, y, pressure[hPs]) or pos_idx = (x, y, idx)
	To display elements in the scene, we use pos_idx. Because or z axis is flipped, this means that high pressure values have a low index.
	To compute anything, especially in combination with vector fields, we use pos_ps=(x, y, pressure[hPa]). x and y are always the lonitude and latitude index.
	This way, one avoids having to flip the gradient because the Sample function of the era field never flips the z axis. Only the resample function flips it.
	Basically the flipping only happens at the very end after all computation is already finished.
*/
std::vector<Line3d> JetStream::FindJet(Line3d& seeds) {
	double dt = jet_params_.integration_stepsize;
	std::vector<Line3d> result;

	std::set<Vec3d, decltype(wind_magnitude_comparator_)> seeds_set(wind_magnitude_comparator_);
	jet_kd_tree = new KdTree3d(3, jet_point_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));

	for (int i = 0; i < seeds.size(); i++) {
		seeds_set.insert(seeds[i]);
	}

	PointCloud3d seeds_point_cloud;
	seeds_point_cloud.pts = seeds;
	KdTree3d* seeds_kd_tree = new KdTree3d(3, seeds_point_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	seeds_kd_tree->buildIndex();

	while (seeds_set.size() != 0) {
		Vec3d seed = *(seeds_set.begin());
		seeds_set.erase(seeds_set.begin());
		Line3d jet({ seed });
		// Forward tracing
		Trace(jet, seeds_set, seeds_kd_tree, seeds_point_cloud, false);
		RemoveWrongStartUps(jet);
		// Backward tracing
		Trace(jet, seeds_set, seeds_kd_tree, seeds_point_cloud, true);
		CutWeakEndings(jet);
		if (GetLineDistance(jet) >= jet_params_.min_jet_distance) {
			result.push_back(jet);
			UpdateKdTree(jet);
		}

	}
	delete seeds_kd_tree;
	return result;
}

void JetStream::Trace(Line3d& line, std::set<Vec3d, decltype(wind_magnitude_comparator_)>& seeds_set, KdTree3d* seeds_kd_tree, PointCloud3d& seeds_point_cloud, bool inverse) {
	Line3d traced_line;
	if (line.size() == 0) { return; }
	Vec3d pos;
	if (inverse) {
		pos = ToDomainCoordinates(line[0]);
	}
	else {
		pos = ToDomainCoordinates(line[line.size() - 1]);
	}
	int speed_criteria_not_met = 0;
	do {
		if (!ConditionDomain(pos) || !ConditionWindMagnitude(pos, speed_criteria_not_met)) {
			break;
		}
		Vec3d corr_pos;
		if (inverse) {
			corr_pos = InversePredictorCorrectorStep(pos);
		}
		else {
			corr_pos = PredictorCorrectorStep(pos);
		}
		if (ConditionDomain(corr_pos)) {
			traced_line.push_back(ToIndexCoordinates(corr_pos));
			if (!jet_point_cloud.is_empty()) {
				Vec3d closest = FindClosestJetPoint(jet_params_.split_merge_threshold, ToIndexCoordinates(corr_pos));
				if (closest != Vec3d({ -1, -1, -1 })) {
					traced_line.push_back(closest);
					break;
				}
			}
		}
		pos = corr_pos;

		Line3d nearby_seeds = FindPointsWithinRadius(seeds_kd_tree, seeds_point_cloud, jet_params_.kdtree_radius, ToIndexCoordinates(corr_pos));
		for (int j = 0; j < nearby_seeds.size(); j++) {
			seeds_set.erase(nearby_seeds[j]);
		}
	} while (traced_line.size() + line.size() < jet_params_.stopping_criteria_jet);

	if (inverse) {
		std::reverse(traced_line.begin(), traced_line.end());
		line.insert(line.begin(), traced_line.begin(), traced_line.end());
	}
	else {
		line.insert(line.end(), traced_line.begin(), traced_line.end());
	}
}

void JetStream::RemoveWrongStartUps(Line3d& jet_line) const {
	double threshold = 0.5;
	while (jet_line.size() >= 2) {
		Vec3d start_point = Vec3d({ jet_line[0][0], jet_line[0][1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)jet_line[0][2], true) });
		Vec3d next_point = Vec3d({ jet_line[1][0], jet_line[1][1], CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)jet_line[1][2], true) });
		Vec3d jet_direction = next_point - start_point;
		jet_direction.normalize();

		Vec3d corr_pos = InversePredictorCorrectorStep(start_point);
		Vec3d inverse_jet_direction = corr_pos - start_point;
		inverse_jet_direction.normalize();
		double projection = jet_direction.dot(inverse_jet_direction);
		if (projection + 1 <= threshold) {
			break;
		}
		else {
			jet_line.erase(jet_line.begin());
		}
	}
}

/*
	Cuts the endings of the jet if they are below threshold.
*/
void JetStream::CutWeakEndings(Line3d& jet) {
	if (jet.size() >= 2) {
		int left = 0;
		int right = (int)jet.size() - 1;
		float wm_l = wind_magnitude_->Sample(ToDomainCoordinates(jet[left]));
		float wm_r = wind_magnitude_->Sample(ToDomainCoordinates(jet[right]));
		while (wm_l < jet_params_.wind_speed_threshold || wm_r < jet_params_.wind_speed_threshold) {
			if ((wm_l < jet_params_.wind_speed_threshold && wm_r < jet_params_.wind_speed_threshold)) {
				left++;
				right--;
			}
			else if (wm_l < jet_params_.wind_speed_threshold) {
				left++;
			}
			else if (wm_r < jet_params_.wind_speed_threshold) {
				right--;
			}
			else {
				break;
			}
			if (left < right) {
				wm_l = wind_magnitude_->Sample(ToDomainCoordinates(jet[left]));
				wm_r = wind_magnitude_->Sample(ToDomainCoordinates(jet[right]));
			}
			else {
				jet.clear();
				return;
			}
		}
		//v.size = x
		/*
			[0, 1, 2, 3, 4]
			 ^		       ^
			 begin		   end
		*/
		if (left != 0) {
			jet.erase(jet.begin(), jet.begin() + left);
		}
		//v.size = x - left
		right = right - left;
		// left needs to be subtraced because the jet size changed.
		if (right != jet.size() - 1) {
			jet.erase(jet.begin() + right, jet.end());
		}
	}
}

Vec3d JetStream::PredictorCorrectorStep(const Vec3d& pos) const {
	Vec3d pred_pos = pos;
	for (int i = 0; i < jet_params_.n_predictor_steps; i++) {
		pred_pos = PredictorStepRK4(pred_pos, jet_params_.integration_stepsize);
	}
	Vec3d corr_pos = pred_pos;
	for (int i = 0; i < jet_params_.n_corrector_steps; i++) {
		corr_pos = CorrectorStepRK4(corr_pos, jet_params_.integration_stepsize);
	}
	return corr_pos;
}

Vec3d JetStream::InversePredictorCorrectorStep(const Vec3d& pos) const {
	Vec3d pre_pos = pos;
	for (int i = 0; i < jet_params_.n_predictor_steps; i++) {
		pre_pos = PredictorStepRK4Inverse(pre_pos, jet_params_.integration_stepsize);
	}
	Vec3d corr_pos = pre_pos;
	for (int i = 0; i < jet_params_.n_corrector_steps; i++) {
		corr_pos = CorrectorStepRK4(corr_pos, jet_params_.integration_stepsize);
	}
	return corr_pos;
}

/*
	Perfoms a Runke-Kutta 4 step in the wind direction.
*/
Vec3d JetStream::PredictorStepRK4(const Vec3d& pos, double dt) const {

	Vec3d k1 = wind_direction_normalized_->Sample(pos);
	Vec3d k2 = wind_direction_normalized_->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = wind_direction_normalized_->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = wind_direction_normalized_->Sample(pos + k3 * dt);
	Vec3d v = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	v.normalize();
	return pos + v * dt;
}
/*
  Performs a step in the opposite wind direction at pos.
*/
Vec3d  JetStream::PredictorStepRK4Inverse(const Vec3d& pos, double dt) const {
	Vec3d k1 = wind_direction_normalized_->Sample(pos);
	Vec3d k2 = wind_direction_normalized_->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = wind_direction_normalized_->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = wind_direction_normalized_->Sample(pos + k3 * dt);
	Vec3d v = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	v.normalize();
	return pos + -v * dt;
}
/*
	Performs a Runge Kutta 4 Step in direction u.
	u is needed for the case when the wind direction and the gradient point in opposite directions.
	It could happen in this case, that the algorithm walks backwards to a strong local maximum.
	u is the projection of the gradient on the plane perpendicular to v.
*/
Vec3d JetStream::CorrectorStepRK4(const Vec3d& pos, const double& dt) const {

	Vec3d k1 = grad_wind_magnitude_->Sample(pos);
	Vec3d k2 = grad_wind_magnitude_->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = grad_wind_magnitude_->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = grad_wind_magnitude_->Sample(pos + k3 * dt);
	Vec3d g = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	Vec3d v1_normalized = wind_direction_normalized_->Sample(pos);
	Vec3d v2_normalized = wind_direction_normalized_->Sample(pos + v1_normalized * (dt / 2));
	Vec3d v3_normalized = wind_direction_normalized_->Sample(pos + v2_normalized * (dt / 2));
	Vec3d v4_normalized = wind_direction_normalized_->Sample(pos + v3_normalized * dt);
	Vec3d v = (v1_normalized / 6 + v2_normalized / 3 + v3_normalized / 3 + v4_normalized / 6);

	Vec3d u = g - v * g.dot(v);
	u[2] *= 10;
	double d;
	u.normalize(&d);
	d = std::min(1.0, d);
	return pos + u * d * dt;
}
/*
	Condition to make sure the line stays in the domain.
*/
bool JetStream::ConditionDomain(const Vec3d& point) const {
	Vec3i res = wind_direction_normalized_->GetField()->GetResolution();
	double lon = point[0];
	double lat = point[1];
	double ps = point[2];

	bool condition = (lon >= 0 && lon < res[0]) && (lat >= 0 && lat < res[1]) && (ps >= jet_params_.ps_min_tracing && ps <= jet_params_.ps_max_tracing);
	return condition;
}
/*
  Condition that the Jet core is only allowed to stay for max_steps_below_speed_thresh steps below threshold.
*/
bool JetStream::ConditionWindMagnitude(const Vec3d& point, int& count) const {
	float wind_mag = wind_magnitude_->Sample(point);
	bool condition = wind_mag >= jet_params_.wind_speed_threshold;
	if (!condition) {
		count++;
	}
	else {
		count = 0;
	}
	return condition || (count <= jet_params_.max_steps_below_speed_thresh);
}
/*
  Makes sure only horizontally evolving core lines are considered.
  Lines which evolves more vertically than horizontally don't belong to the jet.
  This could happen for example at tropical storms.
*/
LineCollection JetStream::FilterFalsePositives(const LineCollection& jet) const {
	std::vector<Line3d> jet_vec = jet.GetLinesInVectorOfVector();
	std::vector<Line3d> result;
	for (int i = 0; i < jet_vec.size(); i++) {
		if (jet_vec[i].size() == 0) { continue; }
		Vec3d start_point_3d = jet_vec[i][0];
		double start_ps = CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)start_point_3d[2], true);
		Vec2d start_point_2d = Vec2d{ (jet_vec[i][0][0] * 0.5, jet_vec[i][0][1] * 0.5) };
		double largest_ver_dist = 0;
		double largest_horiz_dist = 0;
		for (int j = 0; j < jet_vec[i].size(); j++) {
			double p = CoordinateConverter::ValueOfIndexInArray(ps_axis_values_, (float)jet_vec[i][j][2], true);
			Vec2d p_v = Vec2d{ (jet_vec[i][j][0] * 0.5, jet_vec[i][j][1] * 0.5) };
			double d_tmp = (start_point_2d - p_v).length();
			if (d_tmp > largest_horiz_dist) {
				largest_horiz_dist = d_tmp;
			}
			double ps_dif = std::abs(start_ps - p);
			if (ps_dif > largest_ver_dist) {
				largest_ver_dist = ps_dif;
			}
		}
		if (largest_ver_dist > 100 && largest_horiz_dist < 10) { continue; }
		result.push_back(jet_vec[i]);
	}
	LineCollection res = LineCollection();
	res.SetData(result);
	return res;
}

void JetStream::UpdateKdTree(const Line3d& new_line) {

	jet_point_cloud.pts.insert(jet_point_cloud.pts.end(), new_line.begin(), new_line.end());
	jet_kd_tree->buildIndex();
}

Line3d JetStream::FindPointsWithinRadius(const KdTree3d* kd_tree, const PointCloud3d& point_cloud, const double& radius, const Vec3d& point) const {
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t n_matches = kd_tree->radiusSearch(&point[0], radius, ret_matches, params);
	if (ret_matches.size() > 0) {
		Line3d result(ret_matches.size());
		for (int i = 0; i < ret_matches.size(); i++) {
			result[i] = point_cloud.pts[ret_matches[i].first];
		}
		return result;
	}
	else {
		return Line3d();
	}
}

Vec3d JetStream::FindClosestJetPoint(const double& radius, const Vec3d& point) const {
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t n_matches = jet_kd_tree->radiusSearch(&point[0], radius, ret_matches, params);
	if (ret_matches.size() > 0) {
		return jet_point_cloud.pts[ret_matches[0].first];
	}
	else {
		return Vec3d({ -1, -1, -1 });
	}
}
