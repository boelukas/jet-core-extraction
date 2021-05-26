#pragma once
#include "DataLoader_Amira.hpp"
#include <vtkPolyData.h>
#include <vtkMarchingCubes.h>
#include <vtkConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkActor.h>
#include <vtkFlyingEdges3D.h>
#include <vtkPolyLine.h>
#include <vtkDoubleArray.h>
#include <vtkProperty.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkPointData.h>
#include <vtkKdTreePointLocator.h>
#include <sstream>

#include "RegularGrid.hpp"
#include "Colors.hpp"

class WcbTrajectories
{

private:
	float isoValue = 2.0;
	std::vector<float>PSaxisValues;
	std::vector<float>lon_array;
	std::vector<float>lat_array;
	double minAttributeVal;
	double maxAttributeVal;

public:	
	enum class HEMISPHERE { BOTH, NORTH, SOUTH };

	WcbTrajectories():minAttributeVal(std::numeric_limits<double>::max()), maxAttributeVal(-std::numeric_limits<double>::max()) {
	}
	/*
		Returns an actor, with all wcb points that exist at time "time". Shows only points on corresponding hemisphere and draws a sphere around them.
		Samples the field scalarValues for every point and colors them with the transferfunctin tf.
		min and max colorValue set the range of the transfer funciton. Points below the range are colored black.
	*/
	vtkSmartPointer<vtkActor> createWcbPointsActor(const LineCollection& wcb_points, const size_t& time, const HEMISPHERE& hemisphere, RegScalarField3f* scalarValues, const Colors::TRANSFERFUNCTION& tf, const double& minColorValue = 1, const double& maxColorValue = -1) const{
		
		vtkSmartPointer<vtkDoubleArray> scalar_data = vtkSmartPointer<vtkDoubleArray>::New();
		scalar_data->SetName("ColorValues");

		std::vector<float> all_points = wcb_points.getLinesAsVector();
		size_t nr_points = all_points[time + 1];
		size_t start_idx = 1;
		start_idx += all_points[0];
		for (long long i = 0; i < time; i++) {
			start_idx += all_points[i + 1] * 3;
		}
		double max_ps = 0;
		double min_ps = 10000000;
		vtkSmartPointer<vtkPoints> vtkP = vtkSmartPointer<vtkPoints>::New();
		for (long long i = 0; i < all_points[time + 1]; i++) {
			Vec3d p = wcb_points.getPointOfLine(time, i);
			if (hemisphere == HEMISPHERE::BOTH) {
				vtkP->InsertNextPoint(p[0] * 0.5, p[1] * 0.5, p[2]);
				double ps = CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], false);
				if (ps < min_ps) {
					min_ps = ps;
				}
				if (ps > max_ps) {
					max_ps = ps;
				}
				//double val = (double)scalarValues->GetVertexDataAt(p);
				double val = (double)scalarValues->Sample(Vec3d({ CoordinateConverter::valueOfIndexInArray(lon_array, p[0], false),CoordinateConverter::valueOfIndexInArray(lat_array, p[1], false), CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], false) }));
				scalar_data->InsertNextValue(val);
			}
			else if (hemisphere == HEMISPHERE::NORTH && p[1] >= 180) {
				vtkP->InsertNextPoint(p[0] * 0.5, p[1] * 0.5, p[2]);
				//double val = (double)scalarValues->GetVertexDataAt(p);
				double val = (double)scalarValues->Sample(Vec3d({ CoordinateConverter::valueOfIndexInArray(lon_array, p[0], false),CoordinateConverter::valueOfIndexInArray(lat_array, p[1], false), CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], false) }));
				scalar_data->InsertNextValue(val);
			}
			else if (hemisphere == HEMISPHERE::SOUTH && p[1] < 180) {
				vtkP->InsertNextPoint(p[0] * 0.5, p[1] * 0.5, p[2]);
				double val = (double)scalarValues->Sample(Vec3d({ CoordinateConverter::valueOfIndexInArray(lon_array, p[0], false),CoordinateConverter::valueOfIndexInArray(lat_array, p[1], false), CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], false) }));
				//double val = (double)scalarValues->GetVertexDataAt(p);
				scalar_data->InsertNextValue(val);
			}
		}

		//transformToSphere(vtkP);
		//transformToPolarStereographic(vtkP, true);
		vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
		grid->SetPoints(vtkP);
		grid->GetPointData()->AddArray(scalar_data);

		vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetRadius(1.5);
		sphereSource->SetPhiResolution(50);
		sphereSource->SetThetaResolution(50);
		vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
		glyph3D->SetInputData(grid);
		glyph3D->SetSourceConnection(sphereSource->GetOutputPort());

		vtkSmartPointer<vtkLookupTable> colorLut;
		if (minColorValue > maxColorValue) {
			colorLut = Colors::CreateLookupTable(tf, minAttributeVal, maxAttributeVal);
		}
		else {
			colorLut = Colors::CreateLookupTable(tf, minColorValue, maxColorValue);
		}
		colorLut->SetBelowRangeColor(0.0, 0.0, 0.0, 1.0);
		colorLut->SetUseBelowRangeColor(true);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(glyph3D->GetOutputPort());
		/*
		mapper->SetLookupTable(colorLut);
		mapper->SetScalarModeToUsePointFieldData();
		mapper->UseLookupTableScalarRangeOn();
		mapper->SetColorModeToMapScalars();
		mapper->SelectColorArray("ColorValues");
		*/
		mapper->Update();

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		//actor->GetProperty()->SetColor(165/255., 42/255., 42/255.);
		actor->GetProperty()->SetColor(255 / 255., 20 / 255., 147 / 255.);



		return actor;
	}
	/*
		Returns a line collection where each line shows the forward and backward extension of the points in wcb_points at time "time". 
		The extende lines exist are not anymore at time "time", because the wcb trajectories are traced over time.
		In wcb_points the points per time step are stored. In wcb_dist, the points per trajectory are stored.
		Wcb_points has attributes Line and Idx_in_line. With those, for every point the corresponding point in a trajectory from wcb_dist can be found. 
		Then a line is created from Line[idx_in_line - points_before] through all points until Line[idx_in_line + points_after].
	*/
	vtkSmartPointer<vtkActor> createWcbExtendedPointsActor(const LineCollection& wcb_points, const LineCollection& wcb_dist, const size_t& time, const long long& points_before, const long long& points_after,const HEMISPHERE& hemisphere, RegScalarField3f* scalarValues, const Colors::TRANSFERFUNCTION& tf, const DataLoader_Amira* dl, const double& minColorValue = 1, const double& maxColorValue = -1) {

		std::vector<float> line = wcb_points.getAttributeByName("Line");
		std::vector<float> idx_in_line = wcb_points.getAttributeByName("Idx_in_line");

		vtkSmartPointer<vtkPoints> line_points = vtkSmartPointer<vtkPoints>::New();
		vtkSmartPointer<vtkCellArray> line_cells = vtkSmartPointer<vtkCellArray>::New();

		vtkSmartPointer<vtkDoubleArray> scalar_data = vtkSmartPointer<vtkDoubleArray>::New();
		scalar_data->SetName("ColorValues");

		size_t nr_points = wcb_points.getNumberOfPointsOfLine(time);
		std::vector<std::vector<Vec3d>> vec_extended_lines(nr_points);

#pragma omp parallel for schedule(dynamic, 24)
		for (long long i = 0; i < nr_points; i++) {
			Vec3d p = wcb_points.getPointOfLine(time, i);
			size_t p_idx = wcb_points.getIndexInAttributeArray(time, i);
			size_t line_nr = (size_t)std::round(line[p_idx]);
			size_t line_idx = (size_t)std::round(idx_in_line[p_idx]);

			size_t start_idx = std::max((long long)line_idx - points_before, (long long)0);
			size_t line_length = wcb_dist.getNumberOfPointsOfLine(line_nr);
			size_t end_idx = std::min((size_t)(line_idx + points_after), wcb_dist.getNumberOfPointsOfLine(line_nr) - 1);
			size_t nr_points_line = (end_idx - start_idx) + 1;
			vec_extended_lines[i] = std::vector<Vec3d>(nr_points_line);
			size_t vec_ptr = 0;
			for (long long j = 0; j < nr_points_line; j++) {
				Vec3d p = wcb_dist.getPointOfLine(line_nr, start_idx + j);
				vec_extended_lines[i][j] = p;
			}
		}
		size_t point_counter = 0;
		vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
		minAttributeVal = std::numeric_limits<double>::max();
		maxAttributeVal = -std::numeric_limits<double>::max();
		PSaxisValues = dl->loadFloatArray("ps");
		lon_array = dl->loadFloatArray("lon");
		lat_array = dl->loadFloatArray("lat");

		for (size_t i = 0; i < vec_extended_lines.size(); i++) {
			Vec3d hemisphereTest = vec_extended_lines[i][0];
			if(hemisphere != HEMISPHERE::BOTH){
				if (hemisphere == HEMISPHERE::NORTH && hemisphereTest[1] < 180) { continue; };
				if (hemisphere == HEMISPHERE::SOUTH && hemisphereTest[1] >= 180) { continue; };
			}
			polyLine->GetPointIds()->SetNumberOfIds(vec_extended_lines[i].size());
			for (size_t j = 0; j < vec_extended_lines[i].size(); j++) {
				Vec3d p = vec_extended_lines[i][j];
				line_points->InsertNextPoint(p[0] * 0.5, p[1] * 0.5, p[2]);
				double val = (double)scalarValues->Sample(Vec3d({ CoordinateConverter::valueOfIndexInArray(lon_array, p[0], false),CoordinateConverter::valueOfIndexInArray(lat_array, p[1], false), CoordinateConverter::valueOfIndexInArray(PSaxisValues, p[2], false) }));
				if (val < minAttributeVal) {
					minAttributeVal = val;
				}
				else if (val > maxAttributeVal) {
					maxAttributeVal = val;
				}
				scalar_data->InsertNextValue(val);
				polyLine->GetPointIds()->SetId(j, point_counter);
				point_counter++;
			}
			line_cells->InsertNextCell(polyLine);
		}
		vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
		polyData->SetPoints(line_points);	
		polyData->GetPointData()->AddArray(scalar_data);
		polyData->SetLines(line_cells);		

		vtkSmartPointer<vtkLookupTable> colorLut;
		if (minColorValue > maxColorValue) {
			colorLut = Colors::CreateLookupTable(tf, minAttributeVal, maxAttributeVal);
		}
		else {
			colorLut = Colors::CreateLookupTable(tf, minColorValue, maxColorValue);
		}
		colorLut->SetBelowRangeColor(0.0, 0.0, 0.0, 1.0);
		colorLut->SetUseBelowRangeColor(true);

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(polyData);
		/*
		mapper->SetLookupTable(colorLut);
		mapper->SetScalarModeToUsePointFieldData();
		mapper->UseLookupTableScalarRangeOn();
		mapper->SetColorModeToMapScalars();
		mapper->SelectColorArray("ColorValues");
		*/
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		//actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
		//actor->GetProperty()->SetColor(165 / 255., 42 / 255., 42 / 255.);
		actor->GetProperty()->SetColor(255 / 255., 20 / 255., 147 / 255.);

		return actor;
	}
	const double& getMinAttributeValue() const{
		return minAttributeVal;
	}
	const double& getMaxAttributeValue() const{
		return maxAttributeVal;
	}

	/*
		Functions for preprocessing. 
	*/
	/*
		Read the input data files and export the trajectories as line collection.
	*/
	void readInputFileAndExportAsLineCollection(const std::string& srcpath, const std::string& destpath, LineCollection& lineCol, const DataLoader_Amira* dl) {
		NetCDF::Info info;
		//std::string path = "C:\\Users\\Lukas\\Documents\\BachelorThesis\\GitProject\\lcy_201609";
		std::vector<std::vector<Vec3d>> lines = std::vector<std::vector<Vec3d>>();
		PSaxisValues = dl->loadFloatArray("ps");
		lon_array = dl->loadFloatArray("lon");
		lat_array = dl->loadFloatArray("lat");


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
	/*
		Computes the distance to the jet and stores it as attribute to the LineCollection wcb.
	*/
	void collectDataForWcb(LineCollection& wcb, const std::vector<float>& PSaxisValues, const DataLoader_Amira* dl) {
		size_t nr_points = wcb.getTotalNumberOfPoints();
		std::vector<float> times = wcb.getAttributeByName("time");
		std::vector<float> vertical_dist_to_Jet(nr_points);
		std::vector<float> horizontal_dist_to_Jet(nr_points);
		double progress = 0;
		
		std::vector<LineCollection::Point> v = wcb.getAllPointsWithAttributesInVector();


		std::sort(v.begin(), v.end(),
			[](const LineCollection::Point& a, const LineCollection::Point& b) -> bool
			{
				return a.attributes[0] < b.attributes[0];
			});
		std::vector<float> time_step = std::vector<float>();
		std::vector<size_t> start_idx = std::vector<size_t>();
		std::vector<size_t> step_size = std::vector<size_t>();
		float old_time_step = -1.0;
		size_t time_step_ptr = -1;
		for (long long i = 0; i < v.size(); i++) {
			float ts = v[i].attributes[0];
			if (ts == old_time_step) {
				step_size[time_step_ptr]++;
			}
			else {
				time_step_ptr++;
				time_step.push_back(ts);
				start_idx.push_back(i);
				step_size.push_back(1);
				old_time_step = ts;
			}
		}
		size_t trajectoryDataTimeDifference = dl->hoursBeteenWCBTrajectoryStartAndDataStart();
#pragma omp parallel for schedule(dynamic, 24)//time_step.size()
		for (long long i = 0; i < time_step.size(); i++) {
			Vec3d spacing = Vec3d({ 0.5, 0.5, 1.0 });
			LineCollection jet = LineCollection();
			std::vector<std::string> attr_jet;

			dl->loadLineCollection("Jet", (size_t)std::round(time_step[i]) - trajectoryDataTimeDifference, attr_jet, jet);
			// New and experimental
			std::vector<std::vector<Vec3d>> j = jet.getLinesInVectorOfVector();
			double min_ps_ind = 10000000;
			double max_ps_ind = -1;


			for (int k = 0; k < j.size(); k++) {
				for (int kk = 0; kk < j[k].size(); kk++) {
					if (j[k][kk][2] < min_ps_ind) {
						min_ps_ind = j[k][kk][2];
					}
					if (j[k][kk][2] > max_ps_ind) {
						max_ps_ind = j[k][kk][2];
					}
				}
			}
			double min_ps_value = CoordinateConverter::valueOfIndexInArray(PSaxisValues, max_ps_ind, true);
			double max_ps_value = CoordinateConverter::valueOfIndexInArray(PSaxisValues, min_ps_ind, true);
			max_ps_value = max_ps_value > 400 ? 400 : max_ps_value;

			if (jet.getNumberOfLines() == 0) {
				for (long long j = 0; j < step_size[i]; j++) {
					vertical_dist_to_Jet[(size_t)std::round(v[start_idx[i] + j].attributes[1])] = 0;
					horizontal_dist_to_Jet[(size_t)std::round(v[start_idx[i] + j].attributes[1])] = 0;
				}
				continue;
			}
			std::vector<Vec3d> points = jet.getAllPointsInVector();

			vtkSmartPointer<vtkPoints> vPoints = vtkSmartPointer<vtkPoints>::New();
			std::unordered_map<vtkIdType, Vec3d> id_map;
			for (size_t j = 0; j < points.size(); j++) {
				double x = points[j][0] * spacing[0];
				double y = points[j][1] * spacing[1];
				double z = CoordinateConverter::valueOfIndexInArray(PSaxisValues, points[i][2], true);
				vtkIdType id = vPoints->InsertNextPoint(x, y, z);
				id_map[id] = Vec3d({ x, y, z });
			}
			vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
			polydata->SetPoints(vPoints);

			vtkSmartPointer<vtkKdTreePointLocator> kDTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
			kDTree->SetDataSet(polydata);
			kDTree->BuildLocator();

			compute_horizontal_and_vertical_distance2(kDTree, i, start_idx, step_size, v, vertical_dist_to_Jet, horizontal_dist_to_Jet, id_map, PSaxisValues, min_ps_value, max_ps_value);

		}
		
		wcb.addAttribute(vertical_dist_to_Jet, "Jet_dist_vertical");
		wcb.addAttribute(horizontal_dist_to_Jet, "Jet_dist_horizontal");
		//wcb.Export("..\\..\\..\\WCB_Trajectories\\Preprocessing\\21609_wcb_dist");
	}
	void compute_horizontal_and_vertical_distance2(vtkSmartPointer<vtkKdTreePointLocator> kDTree, const size_t& index, const std::vector<size_t>& start_idx, const std::vector<size_t>& step_size, const std::vector<LineCollection::Point>& points, std::vector<float>& vertical_dist_to_Jet, std::vector<float>& horizontal_dist_to_Jet, std::unordered_map<vtkIdType, Vec3d>& id_map, const std::vector<float>& PSaxisValues, double min_ps_value, double max_ps_value) const {
		size_t index_ptr = start_idx[index];
		for (long long i = 0; i < step_size[index]; i++) {
			Vec3d pos = points[index_ptr + i].coord;
			double x[] = { pos[0] * 0.5, pos[1] * 0.5, CoordinateConverter::valueOfIndexInArray(PSaxisValues, pos[2], true) };
			vtkSmartPointer<vtkIdList> nn = vtkSmartPointer<vtkIdList>::New();
			kDTree->FindClosestNPoints(1, x, nn);
			double closestPoint[3];
			//kDTree->GetDataSet()->GetPoint(nn->GetId(0), closestPoint);
			Vec3d nearest = id_map[nn->GetId(0)];
			double d = CoordinateConverter::valueOfIndexInArray(PSaxisValues, pos[2], true) - max_ps_value;
			//std::cout << "d, max_ps_value, min_ps_value: " << d << " , " << max_ps_value << " , " << min_ps_value << std::endl;
			d = d < 0 ? 0 : d;
			float vertical_dist = d;
			float horizontal_dist = (Vec3d({ pos[0] * 0.5, pos[1] * 0.5, 0.0 }) - Vec3d({ nearest[0], nearest[1], 0.0 })).length();
			vertical_dist_to_Jet[(size_t)std::round(points[index_ptr + i].attributes[1])] = vertical_dist;
			horizontal_dist_to_Jet[(size_t)std::round(points[index_ptr + i].attributes[1])] = horizontal_dist;
		}
	}

	void compute_horizontal_and_vertical_distance(vtkSmartPointer<vtkKdTreePointLocator> kDTree,const size_t& index, const std::vector<size_t>& start_idx, const std::vector<size_t>& step_size, const std::vector<LineCollection::Point>& points, std::vector<float>& vertical_dist_to_Jet, std::vector<float>& horizontal_dist_to_Jet, std::unordered_map<vtkIdType, Vec3d>& id_map, const std::vector<float>& PSaxisValues) const{
		size_t index_ptr = start_idx[index];
		for (long long i = 0; i < step_size[index]; i++) {
			Vec3d pos = points[index_ptr + i].coord;
			double x[] = {  pos[0]*0.5, pos[1]*0.5, CoordinateConverter::valueOfIndexInArray(PSaxisValues, pos[2], true) };
			vtkSmartPointer<vtkIdList> nn = vtkSmartPointer<vtkIdList>::New();
			kDTree->FindClosestNPoints(1, x, nn);
			double closestPoint[3];
			//kDTree->GetDataSet()->GetPoint(nn->GetId(0), closestPoint);
			Vec3d nearest = id_map[nn->GetId(0)];
			float vertical_dist = (Vec3d({ 0.0, 0.0,  CoordinateConverter::valueOfIndexInArray(PSaxisValues, pos[2], true) }) - Vec3d({0.0, 0.0, nearest[2]})).length();
			float horizontal_dist = (Vec3d({ pos[0]*0.5, pos[1]*0.5, 0.0 }) - Vec3d({ nearest[0], nearest[1], 0.0 })).length();
			vertical_dist_to_Jet[(size_t)std::round(points[index_ptr + i].attributes[1])] = vertical_dist;
			horizontal_dist_to_Jet[(size_t)std::round(points[index_ptr + i].attributes[1])] = horizontal_dist;
		}
	}

	/*
		Uses the line collection to save a vector of the size of timesteps, where on each position a vector with wcb points is stored, which fulfill the criteria.
		Line i: points at time step i
		Attribute 0: line nr of point i
		Attribute 1: idx in line attr[i] of point i
	*/
	void create_wcb_points(LineCollection& wcb_points, const LineCollection& wcb_filtered,const size_t& _time, const float& horizontal_threshold, const float& vertical_threshold, const bool& only_first_point_per_line, size_t totalNrOfWCBHours) const{
		std::vector<float> wcb = wcb_filtered.getLinesAsVector();
		std::vector<float> time = wcb_filtered.getAttributeByName("time");
		std::vector<float> dist_vertical = wcb_filtered.getAttributeByName("Jet_dist_vertical");
		std::vector<float> dist_horizontal = wcb_filtered.getAttributeByName("Jet_dist_horizontal");

		std::vector<std::vector<LineCollection::Point>> points_per_timestep(totalNrOfWCBHours);


		
		size_t point_ptr = 0;
		size_t line_ptr = 1;
		size_t nr_lines = (size_t)std::round(wcb[0]);
		bool point_vector_created = false;
		bool found_point = false;
		for (long long i = 0; i < nr_lines; i++) {
			size_t points_per_line = (size_t)std::round(wcb[line_ptr]);
			for (long long j = 0; j < points_per_line; j++) {

				size_t t = (size_t)std::round(time[point_ptr]);
				relative_to_jet_filter(point_ptr, i, j, t, dist_horizontal, dist_vertical, vertical_threshold, horizontal_threshold, found_point, only_first_point_per_line, wcb_filtered, points_per_timestep);
				//absolute_height_filter(point_ptr, i, j, t, 54, found_point, wcb_filtered, points_per_timestep);
				//pressure_range_filter(point_ptr, i, j, t, wcb_filtered, points_per_timestep);
				point_ptr++;
			}
			line_ptr++;
			if (point_ptr % 48 == 0) {
				found_point = false;
			}
		}
		
		std::vector<std::vector<Vec3d>> points(points_per_timestep.size());
		std::vector<float> line = std::vector<float>();
		std::vector<float> idx_in_line = std::vector<float>();
		point_ptr = 0;
		for (long long i = 0; i < totalNrOfWCBHours; i++) {
			points[i] = std::vector<Vec3d>();
			for (long long j = 0; j < points_per_timestep[i].size(); j++) {
				points[i].push_back(points_per_timestep[i][j].coord);
				line.push_back(points_per_timestep[i][j].attributes[0]);
				idx_in_line.push_back(points_per_timestep[i][j].attributes[1]);
				point_ptr++;
			}
		}
		wcb_points.setData(points);
		wcb_points.addAttribute(line, "Line");
		wcb_points.addAttribute(idx_in_line, "Idx_in_line");
		//wcb_filtered.clear();
		//wcb_points.Export("..\\..\\..\\WCB_Trajectories\\Preprocessing\\21609_wcb_points");
		
	}
	/*
		Filters points relative to the jet: Only adds points closer than horizontal and vertical threshold.
	*/
	void relative_to_jet_filter(const size_t& point_ptr, const long long& i, const long long& j, const size_t& t,  const std::vector<float>& dist_horizontal, 
		const std::vector<float>& dist_vertical, const float& vertical_threshold, const float& horizontal_threshold, bool& found_point, const bool& only_first_point_per_line, 
		const LineCollection& wcb_filtered, std::vector<std::vector<LineCollection::Point>>& points_per_timestep) const {
		float d_h = dist_horizontal[point_ptr];
		float d_v = dist_vertical[point_ptr];
		if (d_v <= vertical_threshold && d_h <= horizontal_threshold && !found_point) {
			if (only_first_point_per_line) {
				found_point = true;
			}
			LineCollection::Point p;
			p.coord = wcb_filtered.getPoint(point_ptr);
			p.attributes = std::vector<float>(2);// line nr, index in line
			p.attributes[0] = i;
			p.attributes[1] = j;
			points_per_timestep[t].push_back(p);
		}
	}
	/*
		Filters points according to their height. Only adds points which are at level absolute_thershold at time t. Interpolates between points.
	*/
	void absolute_height_filter(const size_t& point_ptr, const long long& i, const long long& j, const size_t& t, const float& absolute_threshold,bool& found_point, const LineCollection& wcb_filtered, std::vector<std::vector<LineCollection::Point>>& points_per_timestep) const {
		Vec3d p = wcb_filtered.getPoint(point_ptr);
		if (p[2] >= absolute_threshold  && j != 0 && !found_point) {
			found_point = true;
			Vec3d p_before = wcb_filtered.getPoint(point_ptr - 1);
			float diff_z = p[2] - p_before[2];
			float frac = (absolute_threshold - p_before[2]) / diff_z;
			LineCollection::Point interpol_p;
			interpol_p.coord[0] = p_before[0] + (p[0] - p_before[0]) * frac;
			interpol_p.coord[1] = p_before[1] + (p[1] - p_before[1]) * frac;
			interpol_p.coord[2] = p_before[2] + (p[2] - p_before[2]) * frac;
			interpol_p.attributes = std::vector<float>(2);// line nr, index in line
			interpol_p.attributes[0] = i;
			interpol_p.attributes[1] = j;
			points_per_timestep[t].push_back(interpol_p);
		}
		else if (p[2] >= absolute_threshold && j == 0) {
			found_point = true;
		}
		else {
			//do nothing
		}
		
	}

	void pressure_range_filter(const size_t& point_ptr, const long long& i, const long long& j, const size_t& t, const LineCollection& wcb_filtered, std::vector<std::vector<LineCollection::Point>>& points_per_timestep) const {
		Vec3d p = wcb_filtered.getPoint(point_ptr);
		double ps_min_idx = 72;
		double ps_max_idx = 81.5;
		if (p[2] >= ps_min_idx && p[2] <= ps_max_idx) {
			LineCollection::Point lp;
			lp.coord[0] = p[0];
			lp.coord[1] = p[1];
			lp.coord[2] = p[2];
			lp.attributes = std::vector<float>(2);
			lp.attributes[0] = i;
			lp.attributes[1] = j;
			points_per_timestep[t].push_back(lp);
		}

	}
	void transformToSphere(vtkSmartPointer<vtkPoints> points) const {
		double radius = 200.0;
		double PI = 3.14159265358979323846;
		if (points != nullptr) {
			for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
				double* p = points->GetPoint(i);
				p[0] *= 2.0;
				p[1] *= 2.0;
				double test = p[1];
				double r = p[2] + radius;
				double phi = PI - (p[1] / 359.8) * PI;
				double psi = -PI + (p[0] / 719.8) * 2 * PI;
				double x = r * sin(phi) * cos(psi);
				double y = r * sin(phi) * sin(psi);
				double z = r * cos(phi);
				points->SetPoint(i, x, y, z);
			}
		}
	}
	void transformToPolarStereographic(vtkSmartPointer<vtkPoints> points, const bool& north) const {
		double DiscRadius = 200.0;
		double PI = 3.14159265358979323846;


		if (points != nullptr) {
			for (long long i = 0; i < points->GetNumberOfPoints(); i++) {
				double* p = points->GetPoint(i);
				p[0] *= 2.0;
				p[1] *= 2.0;
				if (north) {
					double test = p[1];
					double r = ((359.8 - p[1]) / (359.8 - 180.0)) * DiscRadius;
					double phi = (p[0] / 719.8) * 2 * PI;
					double x = r * cos(phi + PI / 2.0);
					double y = r * sin(phi + PI / 2.0);
					points->SetPoint(i, x, y, p[2]);
				}
				else {
					double r = p[1] / 180.0 * DiscRadius;
					double phi = ((719.8 - p[0]) / 719.8) * 2.0 * PI;
					double x = r * cos(phi - PI / 2.0);
					double y = r * sin(phi - PI / 2.0);
					points->SetPoint(i, x, y, p[2]);
				}

			}
		}
	}
};

