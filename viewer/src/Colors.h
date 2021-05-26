#pragma once
#include <vtkActor2D.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>

#include "VisualizationHelper.h"

class Colors
{
	/*
		Class for creating lookup tables and sampling pre-defined color transfer functions.
	*/
public:

	static Vec3d Sample(const jet_vis::TRANSFERFUNCTION& colorFunction, const double& minScalarValue, const double& maxScalarValue, const double& sampleValue) {
		/*
			Samples a color transferfunctions at the position sampleValue. Returns a RGB color.
		*/
		switch (colorFunction) {
		case(jet_vis::TRANSFERFUNCTION::HEAT): {
			return SampleTransferFunction_Heat(minScalarValue, maxScalarValue, sampleValue);
		}
		case(jet_vis::TRANSFERFUNCTION::RED_TO_BLUE): {
			return SampleTransferFunction_BlueToRed(minScalarValue, maxScalarValue, sampleValue);
		}
		case(jet_vis::TRANSFERFUNCTION::WHITE_TO_BLUE): {
			return SampleTransferFunction_WhiteToBlue(minScalarValue, maxScalarValue, sampleValue);
		}
		case(jet_vis::TRANSFERFUNCTION::WHITE_TO_RED): {
			return SampleTransferFunction_WhiteToRed(minScalarValue, maxScalarValue, sampleValue);
		}
		case(jet_vis::TRANSFERFUNCTION::YELLOW_TO_RED): {
			return SampleTransferFunction_YellowToRed(minScalarValue, maxScalarValue, sampleValue);
		}
		case(jet_vis::TRANSFERFUNCTION::GREEN_TO_VIOLET): {
			return SampleTransferFunction_GreenToViolet(minScalarValue, maxScalarValue, sampleValue);
		}
		}
	}
	static vtkSmartPointer<vtkLookupTable> CreateLookupTable(const jet_vis::TRANSFERFUNCTION& colorFunction, const double& minScalarValue, const double& maxScalarValue) {
		/*
			Creates a lookup table with value range from minScalarValue to maxScalarValue.
			Inspired by code from Nathalie Baebler and Rafael Boduryan.
		*/
		int numColors = 1000;
		double range = maxScalarValue - minScalarValue;
		vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
		lut->SetNumberOfTableValues(numColors);
		lut->SetTableRange(minScalarValue, maxScalarValue);
		lut->SetValueRange(minScalarValue, maxScalarValue);

		for (int i = 0; i < numColors; i++) {
			double value = minScalarValue + ((double)i / numColors) * range;
			Vec3d color = Colors::Sample(colorFunction, minScalarValue, maxScalarValue, value);
			lut->SetTableValue(i, color[0], color[1], color[2]);
		}

		lut->Build();
		return lut;
	}
	static vtkSmartPointer<vtkActor2D> CreateColorBarActor(const std::string& title, const jet_vis::TRANSFERFUNCTION& colorFunction, const double& minScalarValue, const double& maxScalarValue, const double& x_Axis_pos_factor) {
		/*
			Creates a vtkActor2D to display the color bar in the scene.
			x_Axis_pos_factor moves the actor on the X axis.
		*/
		vtkSmartPointer<vtkLookupTable> lut = CreateLookupTable(colorFunction, minScalarValue, maxScalarValue);
		vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
		scalarBar->SetLookupTable(lut);
		scalarBar->SetTitle(title.c_str());
		scalarBar->SetNumberOfLabels(4);
		scalarBar->SetWidth(0.05);
		scalarBar->SetHeight(0.2);
		//scalarBar->SetWidth(0.1);
		//scalarBar->SetHeight(0.8);
		double font_color[3] = { 0.0, 0.0, 0.0 };
		scalarBar->GetTitleTextProperty()->SetColor(font_color);
		scalarBar->GetTitleTextProperty()->SetFontSize(10);
		scalarBar->GetLabelTextProperty()->SetColor(font_color);
		double* pos = scalarBar->GetPosition();
		pos[0] *= x_Axis_pos_factor;
		pos[1] *= 0.1;
		scalarBar->SetPosition(pos);
		return scalarBar;
	}
	static vtkSmartPointer<vtkColorTransferFunction> CreateColorTransferFunction(const jet_vis::TRANSFERFUNCTION& colorFunction, const int& numberOfPoints, const double& minScalarValue, const double& maxScalarValue) {
		/*
			Creates a color transferfunction.
		*/
		vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
		for (double i = 0; i < (double)numberOfPoints + 1; i++) {
			double offset = i / numberOfPoints;
			double point = minScalarValue + (maxScalarValue - minScalarValue) * offset;
			Vec3d color = Sample(colorFunction, minScalarValue, maxScalarValue, point);
			colorTransferFunction->AddRGBPoint(point, color[0], color[1], color[2]);
		}
		return colorTransferFunction;
	}

private:
	/*
		Pre-defined color transfer functions.
	*/
	static Vec3d SampleTransferFunction_Heat(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d color =
			((Vec3d({ -1.0411, 1.7442, 0.9397 }) * t +
				Vec3d({ 0.4149, -2.7388, -0.2565 })) * t +
				Vec3d({ 0.0162, 0.1454, -1.5570 })) * t +
			Vec3d({ 0.9949, 0.9971, 0.9161 });
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
	static Vec3d SampleTransferFunction_BlueToRed(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d color =
			(((Vec3d({ 5.0048, 7.4158, 6.1246 }) * t +
				Vec3d({ -8.0915, -15.9415, -16.2287 })) * t +
				Vec3d({ 1.1657, 7.4696, 11.9910 })) * t +
				Vec3d({ 1.4380, 1.2767, -1.4886 })) * t +
			Vec3d({ 0.6639, -0.0013, 0.1685 });
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
	static Vec3d SampleTransferFunction_WhiteToBlue(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d color =
			(((Vec3d({ 0.6599, 0.7582, -1.6662 }) * t +
				Vec3d({ 0.5815, -1.3170, 2.5921 })) * t +
				Vec3d({ -1.7724, 0.2448, -1.4702 })) * t +
				Vec3d({ -0.4004, -0.4792, -0.0314 })) * t +
			Vec3d({ 0.9636, 0.9830, 0.9977 });
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
	static Vec3d SampleTransferFunction_WhiteToRed(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d color =
			(((Vec3d({ 0.6786, -0.7348, -3.0188 }) * t +
				Vec3d({ -2.6266, 3.0499, 6.8674 })) * t +
				Vec3d({ 1.6476, -2.9471, -4.1791 })) * t +
				Vec3d({ -0.2985, -0.3204, -0.5610 })) * t +
			Vec3d({ 1.0031, 0.9594, 0.9422 });
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
	static Vec3d SampleTransferFunction_YellowToRed(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d color =
			(((Vec3d({ -0.2206, -0.3529, -0.4765 }) * t +
				Vec3d({ 0, 0, 0 })) * t +
				Vec3d({ 0.1716, 0.0627, 1.3235 })) * t +
				Vec3d({ -0.0608, -0.6078, -1.4353 })) * t +
			Vec3d({ 1.0000, 1.0000, 0.6980 });
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
	static Vec3d SampleTransferFunction_GreenToViolet(const double& minTransferValue, const double& maxTransferValue, const double& value)
	{
		double t = std::min(std::max(0.0, ((value - minTransferValue) /
			(maxTransferValue - minTransferValue))), 1.0);
		Vec3d violet = Vec3d({ 175. / 255., 141. / 255., 195. / 255. });
		Vec3d white = Vec3d({ 247 / 255.,247 / 255.,247 / 255. });
		Vec3d green = Vec3d({ 127 / 255.,191 / 255.,123 / 255. });

		Vec3d color = green * (1 - t) + violet * t;
		if (t <= 0.5) {
			color = green + ((white - green) / (0.5 - 0)) * (t - 0);
		}
		else {
			color = white + ((violet - white) / (1 - 0.5)) * (t - 0.5);
		}
		return Vec3d({
			std::min(std::max(0.0, color[0]), 1.0),
			std::min(std::max(0.0, color[1]), 1.0),
			std::min(std::max(0.0, color[2]), 1.0) });
	}
};
