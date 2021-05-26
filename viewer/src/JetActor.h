#pragma once
#include "core/JetStream.hpp"

#include "VisualizationHelper.h"
#include "Colors.h"

class VisualizationDataLoader;
class vtkActor;

class JetActor
{

public:
	JetActor(const size_t& time, const jet_vis::TRANSFERFUNCTION& transferfunction, const jet_vis::VIEW& view, VisualizationDataLoader* dataloader, const JetStream::JetParameters& jetParams, const bool& usePreprocJet, const bool& constant_tube_radius);
	~JetActor();
	vtkSmartPointer<vtkActor> get() { return jet; };
	JetStream* get_jet() { return js; };

private:
	size_t _time;
	double _minAttributeVal;
	double _maxAttributeVal;
	double _minTransferValue;
	double _maxTransferValue;
	bool constant_tube_radius_;
	const jet_vis::VIEW view_;
	const jet_vis::TRANSFERFUNCTION _transferfunction;
	double _windMagnitudeThreshold;
	double _tubeRadiusScalingFactor;
	std::vector<float> PSaxisValues;
	vtkSmartPointer<vtkActor> jet;
	VisualizationDataLoader* dl;
	JetStream* js;


	void filterHemisphere(LineCollection& jet, const bool& north) const;
	void createJetActor(const LineCollection& jet_lines, EraScalarField3f* scalarValues, const bool& tube);
	vtkSmartPointer<vtkPoints> JetActor::getVtkPointsAllInOne(double spacing[], const LineCollection& line_col) const;
	LineCollection JetActor::removeShortAndWeakJets(const LineCollection& jet, EraScalarField3f* windMag) const;

};
