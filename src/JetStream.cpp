#include <mutex>
#include <limits>

#include "jet-core-extraction/WindFields.hpp"
#include "jet-core-extraction/DataHelper.hpp"
#include "jet-core-extraction/TimeHelper.hpp"

#include "jet-core-extraction/JetStream.hpp"

JetStream::JetStream(const size_t& time, const JetParameters& jetParams, const bool& PS3D_preprocessed)
	:_time(time),
	_jetParams(jetParams),
	_PS3D_preprocessed(PS3D_preprocessed),
	minAttributeVal(std::numeric_limits<double>::max()),
	maxAttributeVal(-std::numeric_limits<double>::max()),
	PSaxisValues(getPsAxis()),
	jet_core_lines(LineCollection()),
	fields(std::vector<RegScalarField3f*>()),
	windDirection_normalized(nullptr),
	gradWindMagnitude(nullptr),
	windMagnitude(nullptr),
	windMagnitudeSmooth(nullptr),
	PS3D(nullptr),
	mtx(std::mutex()),
	_usePreviousTimeStep(false),
	_usePreprocessedPreviousJet(false)
{
	WindFields wf = WindFields();

  fields = DataHelper::loadScalarFields(time, std::vector<std::string>({ "U", "V", "OMEGA", "T" }));
  PS3D = DataHelper::compute_PS3D(TimeHelper::convertHoursToDate(time, DataHelper::getDataStartDate()), fields[0]->GetResolution(), fields[0]->GetDomain());
  windDirection_normalized = wf.GetNormalizedWindDirectionEra(time, PS3D, fields[0], fields[1], fields[2]);
  windMagnitude = wf.GetWindMagnitudeEra(time, PSaxisValues, PS3D, fields[0], fields[1], fields[2], fields[3]);
  windMagnitudeSmooth = wf.getSmoothWindMagnitude(time, PSaxisValues, PS3D, fields[0], fields[1], fields[2], fields[3]);
  gradWindMagnitude = wf.GetWindMagnitudeGradientEra(time, PS3D, fields[3], windMagnitudeSmooth->GetField());

	cmp.PSaxisValues = PSaxisValues;
	cmp.windMagnitude = windMagnitude;
}
JetStream::~JetStream() {
	PSaxisValues.clear();
	for (size_t i = 0; i < fields.size(); i++) {
		delete fields[i];
	}
	fields.clear();
	delete windDirection_normalized->GetField();
	delete windDirection_normalized;
	delete windMagnitude->GetField();
	delete windMagnitude;
	delete windMagnitudeSmooth->GetField();
	delete windMagnitudeSmooth;
	delete gradWindMagnitude->GetField();
	delete gradWindMagnitude;
	delete jet_kd_tree;
	if (!_PS3D_preprocessed) {
		delete PS3D;
	}
}
LineCollection JetStream::getJetCoreLines() {
	if (jet_core_lines.getNumberOfLines() == 0) {
		computeJetLines();
	}
	return jet_core_lines;
}
/*
	Interface for Preprocessor to compute the Jet Core Lines.
*/
void JetStream::computeJetLines() {
	generateJetSeeds();
	std::vector<Line3d> jet = findJet(_seeds);
	jet_core_lines = LineCollection();
	jet_core_lines.setData(jet);
	jet_core_lines = filterTropicalStorms(jet_core_lines);


}

/*
	Iterates through the wind Magnitude scalar field to find points where the threshold is reached.
*/
void JetStream::generateJetSeeds() {
	PointCloud3d prev_jet_cloud{ Line3d() };
	KdTree3d* prev_jet_tree = new KdTree3d(3, prev_jet_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));

	if (_usePreviousTimeStep) {
		Line3d prev_jet = getPreviousTimeStepSeeds();
		prev_jet_cloud.pts = prev_jet;
		prev_jet_tree->buildIndex();
		_seeds.insert(_seeds.end(), prev_jet.begin(), prev_jet.end());
	}
	double ps_min_idx = CoordinateConverter::indexOfValueInArray(PSaxisValues, _jetParams.ps_min_val, true);
	double ps_max_idx = CoordinateConverter::indexOfValueInArray(PSaxisValues, _jetParams.ps_max_val, true);
	size_t numEntries = (size_t)windMagnitudeSmooth->GetField()->GetResolution()[0] * (size_t)windMagnitudeSmooth->GetField()->GetResolution()[1] * (size_t)windMagnitudeSmooth->GetField()->GetResolution()[2];


#pragma omp parallel for schedule(dynamic,24)
	for (long long linearIndex = 0; linearIndex < numEntries; linearIndex++) {
		Vec3i coords = windMagnitudeSmooth->GetField()->GetGridCoord(linearIndex);
		if (coords[2] <= ps_min_idx && coords[2] >= ps_max_idx) {
			Vec3d seed_candidate = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) });
			Vec3d up = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) + 10 });
			Vec3d down = Vec3d({ (double)coords[0], (double)coords[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) - 10 });
			Vec3d left = Vec3d({ (double)coords[0] - 1, (double)coords[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) });
			Vec3d right = Vec3d({ (double)coords[0] + 1, (double)coords[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) });
			Vec3d front = Vec3d({ (double)coords[0], (double)coords[1] + 1, CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) });
			Vec3d back = Vec3d({ (double)coords[0], (double)coords[1] - 1, CoordinateConverter::valueOfIndexInArray(PSaxisValues, coords[2], true) });


			float wind_mag = windMagnitudeSmooth->Sample(seed_candidate);

			float w_up = windMagnitudeSmooth->Sample(up);
			float w_down = windMagnitudeSmooth->Sample(down);
			float w_left = windMagnitudeSmooth->Sample(left);
			float w_right = windMagnitudeSmooth->Sample(right);
			float w_front = windMagnitudeSmooth->Sample(front);
			float w_back = windMagnitudeSmooth->Sample(back);

			if (wind_mag >= _jetParams.wind_speed_threshold && wind_mag > std::max({ w_up, w_down,w_left, w_right, w_front, w_back })) {
				if (_usePreviousTimeStep) {
					Line3d closeby_prev_jet_seeds = findPointsWithinRadius(prev_jet_tree, prev_jet_cloud, _jetParams.kdtree_radius, coords);
					if (closeby_prev_jet_seeds.size() == 0) {
						mtx.lock();
						_seeds.push_back(coords);
						mtx.unlock();
					}
				}
				else {
					mtx.lock();
					_seeds.push_back(coords);
					mtx.unlock();
				}
			}
		}

	}
	delete prev_jet_tree;
}
Line3d JetStream::getPreviousTimeStepSeeds() {
	if (_time != 0) {
		LineCollection jet;
		if (_usePreprocessedPreviousJet) {
			std::vector<std::string> attributes;
			DataHelper::loadLineCollection("Jet", (_time - 1), attributes, jet);
		}
		else {
			JetStream js2 = JetStream((_time - 1), _jetParams, false);
			jet = js2.getJetCoreLines();
		}

		std::vector<Line3d> prev_jet = jet.getLinesInVectorOfVector();

		Line3d res = Line3d();
		for (auto line : prev_jet) {
			if (line.size() < 3) { continue; }
			for (int i = 1; i < line.size() - 1; i++) {
				double left = windMagnitude->Sample(toDomainCoordinates(line[i - 1]));
				double centre = windMagnitude->Sample(toDomainCoordinates(line[i]));
				double right = windMagnitude->Sample(toDomainCoordinates(line[i + 1]));
				if (centre > left && centre > right) {
					res.push_back(line[i]);
				}
			}
		}
		return res;
	}
	else {
		return Line3d();
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
std::vector<Line3d> JetStream::findJet(Line3d& seeds) {
	double dt = _jetParams.integration_stepsize;
	std::vector<Line3d> result;

	std::set<Vec3d, decltype(cmp)> seeds_set(cmp);
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

		trace(jet, seeds_set, seeds_kd_tree, seeds_point_cloud, false);
		removeWrongStartUps(jet);
		trace(jet, seeds_set, seeds_kd_tree, seeds_point_cloud, true);
		cutWeakEndings(jet);
		if (getLineDistance(jet) >= _jetParams.min_jet_distance) {
			result.push_back(jet);
			updateKdTree(jet);
		}

	}
	delete seeds_kd_tree;
	return result;
}

void JetStream::trace(Line3d& line, std::set<Vec3d, decltype(cmp)>& seeds_set, KdTree3d* seeds_kd_tree, PointCloud3d& seeds_point_cloud, bool inverse) {
	Line3d traced_line;
	if (line.size() == 0) { return; }
	Vec3d pos;
	if (inverse) {
		pos = toDomainCoordinates(line[0]);
	}
	else {
		pos = toDomainCoordinates(line[line.size() - 1]);
	}
	int speed_criteria_not_met = 0;
	do {
		if (!condition_domain(pos) || !condition_wind_magnitude(pos, speed_criteria_not_met)) {
			break;
		}
		Vec3d corrPos;
		if (inverse) {
			corrPos = inversePredictorCorrectorStep(pos);
		}
		else {
			corrPos = predictorCorrectorStep(pos);
		}
		if (condition_domain(corrPos)) {
			traced_line.push_back(toIndexCoordinates(corrPos));
			if (!jet_point_cloud.is_empty()) {
				Vec3d closest = findClosestJetPoint(_jetParams.split_merge_threshold, toIndexCoordinates(corrPos));
				if (closest != Vec3d({ -1, -1, -1 })) {
					traced_line.push_back(closest);
					break;
				}
			}
		}
		pos = corrPos;

		Line3d nearby_seeds = findPointsWithinRadius(seeds_kd_tree, seeds_point_cloud, _jetParams.kdtree_radius, toIndexCoordinates(corrPos));
		for (int j = 0; j < nearby_seeds.size(); j++) {
			seeds_set.erase(nearby_seeds[j]);
		}
	} while (traced_line.size() + line.size() < _jetParams.stopping_criteria_jet);

	if (inverse) {
		std::reverse(traced_line.begin(), traced_line.end());
		line.insert(line.begin(), traced_line.begin(), traced_line.end());
	}
	else {
		line.insert(line.end(), traced_line.begin(), traced_line.end());
	}
}

void JetStream::removeWrongStartUps(Line3d& jet_line) const {
	double threshold = 0.5;
	while (jet_line.size() >= 2) {
		Vec3d start_point = Vec3d({ jet_line[0][0], jet_line[0][1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, jet_line[0][2], true) });
		Vec3d next_point = Vec3d({ jet_line[1][0], jet_line[1][1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, jet_line[1][2], true) });
		Vec3d jet_direction = next_point - start_point;
		jet_direction.normalize();

		Vec3d corr_pos = inversePredictorCorrectorStep(start_point);
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
void JetStream::cutWeakEndings(Line3d& jet) {
	if (jet.size() >= 2) {
		int left = 0;
		int right = jet.size() - 1;
		float wm_l = windMagnitude->Sample(toDomainCoordinates(jet[left]));
		float wm_r = windMagnitude->Sample(toDomainCoordinates(jet[right]));
		while (wm_l < _jetParams.wind_speed_threshold || wm_r < _jetParams.wind_speed_threshold) {
			if ((wm_l < _jetParams.wind_speed_threshold && wm_r < _jetParams.wind_speed_threshold)) {
				left++;
				right--;
			}
			else if (wm_l < _jetParams.wind_speed_threshold) {
				left++;
			}
			else if (wm_r < _jetParams.wind_speed_threshold) {
				right--;
			}
			else {
				break;
			}
			if (left < right) {
				wm_l = windMagnitude->Sample(toDomainCoordinates(jet[left]));
				wm_r = windMagnitude->Sample(toDomainCoordinates(jet[right]));
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



Vec3d JetStream::predictorCorrectorStep(const Vec3d& pos) const {
	Vec3d predPos = pos;
	for (int i = 0; i < _jetParams.n_predictor_steps; i++) {
		predPos = predictorStepRK4(predPos, _jetParams.integration_stepsize);
	}
	Vec3d corrPos = predPos;
	for (int i = 0; i < _jetParams.n_corrector_steps; i++) {
		corrPos = correctorStepRK4(corrPos, _jetParams.integration_stepsize);
	}
	return corrPos;
}
Vec3d JetStream::inversePredictorCorrectorStep(const Vec3d& pos) const {
	Vec3d predPos = pos;
	for (int i = 0; i < _jetParams.n_predictor_steps; i++) {
		predPos = predictorStepRK4_inverse(predPos, _jetParams.integration_stepsize);
	}
	Vec3d corrPos = predPos;
	for (int i = 0; i < _jetParams.n_corrector_steps; i++) {
		corrPos = correctorStepRK4(corrPos, _jetParams.integration_stepsize);
	}
	return corrPos;
}

/*
	Perfoms a Runke-Kutta 4 step in the wind direction.
	dt is overwritten for the final step. Otherwise the step would be too small.
*/
Vec3d JetStream::predictorStepRK4(const Vec3d& pos, double dt) const {

	Vec3d k1 = windDirection_normalized->Sample(pos);
	Vec3d k2 = windDirection_normalized->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = windDirection_normalized->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = windDirection_normalized->Sample(pos + k3 * dt);
	Vec3d v = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	v.normalize();
	return pos + v * dt;
}
/*
Performs a step in the opposite wind direction at pos.
*/
Vec3d  JetStream::predictorStepRK4_inverse(const Vec3d& pos, double dt) const {
	Vec3d k1 = windDirection_normalized->Sample(pos);
	Vec3d k2 = windDirection_normalized->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = windDirection_normalized->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = windDirection_normalized->Sample(pos + k3 * dt);
	Vec3d v = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	v.normalize();
	return pos + -v * dt;
}
/*
	Performs a Runge Kutta 4 Step in direction u.
	u is needed for the case when the wind direction and the gradient point in oposite directions.
	It could happen in this case, that the algorithm walks backwards to a strong local maximum.
	u is the projection of the gradient on the plane perpendicular to v.

	pos = (long_idx, lat_idx, pressure(hPa))
	dt = the integration stepsize.
*/
Vec3d JetStream::correctorStepRK4(const Vec3d& pos, const double& dt) const {

	Vec3d k1 = gradWindMagnitude->Sample(pos);
	Vec3d k2 = gradWindMagnitude->Sample(pos + k1 * (dt / 2));
	Vec3d k3 = gradWindMagnitude->Sample(pos + k2 * (dt / 2));
	Vec3d k4 = gradWindMagnitude->Sample(pos + k3 * dt);
	Vec3d g = (k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6);
	Vec3d v1_normalized = windDirection_normalized->Sample(pos);
	Vec3d v2_normalized = windDirection_normalized->Sample(pos + v1_normalized * (dt / 2));
	Vec3d v3_normalized = windDirection_normalized->Sample(pos + v2_normalized * (dt / 2));
	Vec3d v4_normalized = windDirection_normalized->Sample(pos + v3_normalized * dt);
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
bool JetStream::condition_domain(const Vec3d& point) const {
	Vec3i res = windDirection_normalized->GetField()->GetResolution();
	double lon = point[0];
	double lat = point[1];
	double ps = point[2];

	bool condition = (lon >= 0 && lon < res[0]) && (lat >= 0 && lat < res[1]) && (ps >= _jetParams.ps_min_tracing && ps <= _jetParams.ps_max_tracing);
	return condition;
}
/*
Condition that the Jet core is only allowed to stay for max_steps_below_speed_thresh steps below threshold.
*/
bool JetStream::condition_wind_magnitude(const Vec3d& point, int& count) const {
	float wind_f = windMagnitude->Sample(point);
	bool condition = wind_f >= _jetParams.wind_speed_threshold;
	if (!condition) {
		count++;
	}
	else {
		count = 0;
	}
	return condition || (count <= _jetParams.max_steps_below_speed_thresh);
}

LineCollection JetStream::filterTropicalStorms(const LineCollection& jet) const {
	std::vector<Line3d> jet_vec = jet.getLinesInVectorOfVector();
	std::vector<Line3d> result;
	for (int i = 0; i < jet_vec.size(); i++) {
		if (jet_vec[i].size() == 0) { continue; }
		Vec3d startP = jet_vec[i][0];
		double start_ps = CoordinateConverter::valueOfIndexInArray(PSaxisValues, startP[2], true);
		Vec2d start_p = Vec2d{ (jet_vec[i][0][0] * 0.5, jet_vec[i][0][1] * 0.5) };
		double largest_ver_dist = 0;
		double largest_horiz_dist = 0;
		for (int j = 0; j < jet_vec[i].size(); j++) {
			double p = CoordinateConverter::valueOfIndexInArray(PSaxisValues, jet_vec[i][j][2], true);
			Vec2d p_v = Vec2d{ (jet_vec[i][j][0] * 0.5, jet_vec[i][j][1] * 0.5) };
			double d_tmp = (start_p - p_v).length();
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
	LineCollection r = LineCollection();
	r.setData(result);
	return r;
}

void JetStream::updateKdTree(const Line3d& new_line) {

	jet_point_cloud.pts.insert(jet_point_cloud.pts.end(), new_line.begin(), new_line.end());
	jet_kd_tree->buildIndex();
}
Line3d JetStream::findPointsWithinRadius(const KdTree3d* kd_tree, const PointCloud3d& point_cloud, const double& radius, const Vec3d& point) const {
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t nMatches = kd_tree->radiusSearch(&point[0], radius, ret_matches, params);
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
Vec3d JetStream::findClosestJetPoint(const double& radius, const Vec3d& point) const {
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t nMatches = jet_kd_tree->radiusSearch(&point[0], radius, ret_matches, params);
	if (ret_matches.size() > 0) {
		return jet_point_cloud.pts[ret_matches[0].first];
	}
	else {
		return Vec3d({ -1, -1, -1 });
	}
}
Vec3d JetStream::findClosestJetPointExcludingLine(const double& radius, const Vec3d& point, const Line3d& line) const {
	Vec3dSet line_set(line.begin(), line.end());
	std::vector<std::pair<size_t, double> >  ret_matches;
	nanoflann::SearchParams params;
	params.sorted = true;
	const size_t nMatches = jet_kd_tree->radiusSearch(&point[0], radius, ret_matches, params);
	for (int i = 0; i < ret_matches.size(); i++) {
		Vec3d candiate = jet_point_cloud.pts[ret_matches[i].first];
		if (line_set.find(candiate) == line_set.end()) {
			return candiate;
		}
	}
	return Vec3d({ -1, -1, -1 });
}
