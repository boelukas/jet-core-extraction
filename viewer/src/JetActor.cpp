#pragma once
#include <vtkDoubleArray.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkTubeFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

#include "core/DataHelper.hpp"

#include "VisualizationDataLoader.h"

#include "JetActor.h"

JetActor::JetActor(const size_t& time, const jet_vis::TRANSFERFUNCTION& transferfunction, const jet_vis::VIEW& view, VisualizationDataLoader* dataloader, const JetStream::JetParameters& jetParams, const bool& usePreprocJet, const bool& constant_tube_radius)
	:_time(time),
	dl(dataloader),
	js(nullptr),
	_transferfunction(transferfunction),
	view_(view),
	_minAttributeVal(1),
	_maxAttributeVal(-1),
	_minTransferValue(10),
	_maxTransferValue(110),
	_tubeRadiusScalingFactor(8),
	_windMagnitudeThreshold(40),
	constant_tube_radius_(constant_tube_radius)
{
	_time = time;

	LineCollection jet_lines;
	if (usePreprocJet) {
		std::vector<std::string> attributes;
		DataHelper::loadLineCollection("Jet", time, attributes, jet_lines);
	}
	else {
		std::cout << "Jet Actor" << std::endl;
		js = new JetStream(time, jetParams, true);
		std::cout << "Js" << std::endl;
		if (_time != 0) {
			js->setUsePreviousTimeStep();
		}
		jet_lines = js->getJetCoreLines();
	}
	EraScalarField3f* line_attributes = DataHelper::loadEraScalarField3f("windMagnitude_lev", time);
	PSaxisValues = DataHelper::loadFloatArray("ps");
	//jet_lines = filterTropicalStorms(jet_lines);
	//DataHelper::loadKernPaperJet(0, jet_lines); // For kern computed jets
	//jet_lines = removeShortAndWeakJets(jet_lines, line_attributes);
	//-------------------------------------------------------------------------------------------------------------------------------------------------------Change Input Jets to Kern

	jet_vis::view_filter(jet_lines, view_);
	//jet_lines = jetPostProcessing(jet_lines, time);
	createJetActor(jet_lines, line_attributes, true);

	jet_lines.clear();
	delete line_attributes->GetField();
	delete line_attributes->Get3DPS();
	delete line_attributes;
}
LineCollection JetActor::removeShortAndWeakJets(const LineCollection& jet, EraScalarField3f* windMag) const {
	std::vector<std::vector<Vec3d>> jet_vec = jet.getLinesInVectorOfVector();
	std::vector<std::vector<Vec3d>> res;
	for (int i = 0; i < jet_vec.size(); i++) {
		//if (jet_vec[i].size() >= min_jet_size) {
		double max_wind_mag = -1;
		for (int j = 0; j < jet_vec[i].size(); j++) {
			double mag = windMag->Sample(Vec3d({ jet_vec[i][j][0], jet_vec[i][j][1],CoordinateConverter::valueOfIndexInArray(PSaxisValues, jet_vec[i][j][2], true) }));
			if (mag > max_wind_mag) {
				max_wind_mag = mag;
			}
		}
		if (max_wind_mag >= 40) {
			res.push_back(jet_vec[i]);
		}
	}

	LineCollection r = LineCollection();
	r.setData(res);
	return r;
}
JetActor::~JetActor() {

}

void JetActor::filterHemisphere(LineCollection& jet, const bool& north) const {
	double thresh = 180;
	size_t nr_lines = jet.getNumberOfLines();
	std::vector<bool> mask(nr_lines);
	//#pragma omp parallel for schedule(dynamic,24)
	for (long long i = 0; i < nr_lines; i++) {
		Vec3d p = jet.getPointOfLine(i, 0);
		if (north) {
			if (p[1] >= thresh) {
				mask[i] = true;
			}
			else {
				mask[i] = false;
			}
		}
		else {
			if (p[1] < thresh) {
				mask[i] = true;
			}
			else {
				mask[i] = false;
			}
		}
	}
	std::vector<std::vector<Vec3d>> jet_filtered = std::vector<std::vector<Vec3d>>();
	for (long long i = 0; i < nr_lines; i++) {
		if (mask[i]) {
			jet_filtered.push_back(std::vector<Vec3d>());
			for (long long j = 0; j < jet.getNumberOfPointsOfLine(i); j++) {
				Vec3d p = jet.getPointOfLine(i, j);
				jet_filtered[jet_filtered.size() - 1].push_back(p);
			}
		}
	}
	jet.setData(jet_filtered);
}

void JetActor::createJetActor(const LineCollection& jet_lines, EraScalarField3f* scalarValues, const bool& tube) {

	std::vector<vtkSmartPointer<vtkActor>> result = std::vector<vtkSmartPointer<vtkActor>>(jet_lines.getNumberOfLines());

	double spacing[] = {
	(scalarValues->GetField()->GetDomain().GetMax()[0] - scalarValues->GetField()->GetDomain().GetMin()[0]) / (scalarValues->GetField()->GetResolution()[0] - 1.),// (179.5 + 180)/719 = 0.5
	(scalarValues->GetField()->GetDomain().GetMax()[1] - scalarValues->GetField()->GetDomain().GetMin()[1]) / (scalarValues->GetField()->GetResolution()[1] - 1.),// (90 + 90)/ 360 = 0.5
	1 };

	vtkSmartPointer<vtkPoints> jet_lines_points = getVtkPointsAllInOne(spacing, jet_lines);
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkDoubleArray> point_data = vtkSmartPointer<vtkDoubleArray>::New();
	point_data->SetName("ScalarValues");
	vtkSmartPointer<vtkDoubleArray> color_data = vtkSmartPointer<vtkDoubleArray>::New();
	color_data->SetName("ColorValues");


	size_t point_pointer = 0;
	_minAttributeVal = std::numeric_limits<double>::max();
	_maxAttributeVal = -std::numeric_limits<double>::max();
	for (int i = 0; i < jet_lines.getNumberOfLines(); i++) {
		vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
		polyLine->GetPointIds()->SetNumberOfIds(jet_lines.getNumberOfPointsOfLine(i));
		for (unsigned int j = 0; j < jet_lines.getNumberOfPointsOfLine(i); j++) {
			polyLine->GetPointIds()->SetId(j, point_pointer);

			double* p = jet_lines_points->GetPoint(point_pointer);
			Vec3d coord = Vec3d({ p[0] / spacing[0], p[1] / spacing[1], p[2] / spacing[2] });
			double val = (double)scalarValues->Sample(Vec3d({ coord[0], coord[1], CoordinateConverter::valueOfIndexInArray(PSaxisValues, coord[2], true) }));
			color_data->InsertNextValue(val);
			double val2 = (val - _windMagnitudeThreshold);
			val2 /= _tubeRadiusScalingFactor;
			if (constant_tube_radius_) {
				point_data->InsertNextValue(val2 < 0 ? 1 : 1);
			}
			else {
				point_data->InsertNextValue(val2 < 0 ? 0.1 : val2);
			}
			if (val < _minAttributeVal) {
				_minAttributeVal = val;
			}
			else if (val > _maxAttributeVal) {
				_maxAttributeVal = val;
			}

			point_pointer++;
		}

		// Create a cell array to store the jet_seeds in and add the jet_seeds to it
		cells->InsertNextCell(polyLine);

	}
	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	polyData->SetPoints(jet_lines_points);	// Add the points to the dataset
	polyData->SetLines(cells);			// Add the jet_seeds to the dataset
	polyData->GetPointData()->AddArray(color_data);
	polyData->GetPointData()->AddArray(point_data);
	//polyData->GetPointData()->AddArray(color_data);
	polyData->GetPointData()->SetActiveScalars("ScalarValues");

	vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
	tubeFilter->SetInputData(polyData);
	tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
	tubeFilter->SetNumberOfSides(100);
	tubeFilter->Update();
	//polyData->GetPointData()->SetActiveScalars("ColorValues");

	// Setup actor and mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	if (tube) {
		mapper->SetInputConnection(tubeFilter->GetOutputPort());
	}
	else {
		mapper->SetInputData(polyData);
	}
	//http://vtk.1045678.n5.nabble.com/How-to-use-LookupTable-correctly-td1227112.html as source of how to set up the mapper.
	vtkSmartPointer<vtkLookupTable> colorLut;
	if (_minTransferValue < _maxTransferValue) {
		colorLut = Colors::CreateLookupTable(_transferfunction, _minTransferValue, _maxTransferValue);
	}
	else {
		colorLut = Colors::CreateLookupTable(_transferfunction, _minAttributeVal, _maxAttributeVal);
	}
	mapper->SetLookupTable(colorLut);
	mapper->SetScalarModeToUsePointFieldData();
	mapper->UseLookupTableScalarRangeOn();
	mapper->SetColorModeToMapScalars();
	mapper->SelectColorArray("ColorValues");
	//mapper->SetScalarModeToUseFieldData();
	//mapper->SelectColorArray("ColorValues");
	jet = vtkSmartPointer<vtkActor>::New();
	jet->SetMapper(mapper);
}
vtkSmartPointer<vtkPoints> JetActor::getVtkPointsAllInOne(double spacing[], const LineCollection& line_col) const {
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	size_t line_ptr = 1;
	size_t data_ptr = 1 + line_col.getNumberOfLines();
	for (size_t i = 0; i < line_col.getNumberOfLines(); i++) {
		for (size_t j = 0; j < (size_t)std::round(line_col.getLinesAsVector()[line_ptr]); j++) {
			double x = line_col.getLinesAsVector()[data_ptr++] * spacing[0];
			double y = line_col.getLinesAsVector()[data_ptr++] * spacing[1];
			double z = line_col.getLinesAsVector()[data_ptr++] * spacing[2];
			points->InsertNextPoint(x, y, z);
		}
		line_ptr++;
	}
	return points;
}
