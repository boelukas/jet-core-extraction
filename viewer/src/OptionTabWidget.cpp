#pragma once
#include "core/JetStream.hpp"

#include "MainWindow.h"
#include "DoubleRangeSliderBox.hpp"

#include "OptionTabWidget.h"

OptionTabWidget::OptionTabWidget(MenuWidget* _parent):parent(_parent), mainWindow(parent->parent) {
	MainWindow::GuiState* state = mainWindow->getState();

	//Option tab menu
	{//Wcb Tab
		wcbTab = new QWidget();
		wcbTabLayout = new QGridLayout();

		wcbTabLayout->addWidget(new QLabel("Horizontal Filter[Degree]:"), 0, 0);
		horizontalFilterLine = new QSpinBox();
		horizontalFilterLine->setMaximum(1000);
		horizontalFilterLine->setMinimum(0);
		horizontalFilterLine->setValue(state->wcb.horizontal_filter);
		connect(horizontalFilterLine, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changeHorizontalFilter);
		wcbTabLayout->addWidget(horizontalFilterLine, 0, 1);

		wcbTabLayout->addWidget(new QLabel("Vertical Filter[hPa]:"), 1, 0);
		verticalFilterLine = new QSpinBox();
		verticalFilterLine->setMaximum(1500);
		verticalFilterLine->setMinimum(0);
		verticalFilterLine->setValue(state->wcb.vertical_filter);
		connect(verticalFilterLine, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changeVerticalFilter);
		wcbTabLayout->addWidget(verticalFilterLine, 1, 1);

		wcbTabLayout->addWidget(new QLabel("Backward point extension:[Points]"), 2, 0);
		pointsBeforeLine = new QSpinBox();
		pointsBeforeLine->setMaximum(48);
		pointsBeforeLine->setMinimum(0);
		pointsBeforeLine->setValue(state->wcb.points_before);
		connect(pointsBeforeLine, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changePointsBefore);
		wcbTabLayout->addWidget(pointsBeforeLine, 2, 1);

		wcbTabLayout->addWidget(new QLabel("Forward point extension:[Points]"), 3, 0);
		pointsAfterLine = new QSpinBox();
		pointsAfterLine->setMaximum(48);
		pointsAfterLine->setMinimum(0);
		pointsAfterLine->setValue(state->wcb.points_after);
		connect(pointsAfterLine, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changePointsAfter);
		wcbTabLayout->addWidget(pointsAfterLine, 3, 1);



		wcbTab->setLayout(wcbTabLayout);
		wcbTab->setVisible(state->wcb.active);
		addTab(wcbTab, tr("&WCB"));
	}
	{//Volume Tab
		volumeTab = new QWidget();
		volumeTabLayout = new QGridLayout();

		volumeTabLayout->addWidget(new QLabel("Data:"), 0, 0);
		volumeDataSelectionBox = new QComboBox();
		volumeDataSelectionBox->addItems(jet_vis::dataItems);
		volumeDataSelectionBox->setCurrentText(QString::fromStdString(state->volume.dataName));
		connect(volumeDataSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeVolumeData);
		volumeTabLayout->addWidget(volumeDataSelectionBox, 0, 1);

		volumeTabLayout->addWidget(new QLabel("Color transferfunction:"), 1, 0);
		volumeTfSelectionBox = new QComboBox();
		volumeTfSelectionBox->addItems(jet_vis::transferItems);
		volumeTfSelectionBox->setCurrentText(QString::fromStdString((jet_vis::tf_tfName_map).at(state->volume.transferfunction)));
		connect(volumeTfSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeVolumeTransferfuntion);
		volumeTabLayout->addWidget(volumeTfSelectionBox, 1, 1);

		std::string volumeLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->volume.dataName);
		volumeOpacitySlider = new DoubleRangeSliderBox(volumeLabelText.c_str(), state->volume.minDataValue, state->volume.maxDataValue);
		volumeOpacitySlider->setStepSize((state->volume.maxDataValue - state->volume.minDataValue) / 100.0);
		volumeOpacitySlider->setValues(state->volume.minOpacityValue, state->volume.maxOpacityValue);
		connect(volumeOpacitySlider, &DoubleRangeSliderBox::minValueChanged, mainWindow, &MainWindow::changeMinOpacityValueDoubleSliderVolume);
		connect(volumeOpacitySlider, &DoubleRangeSliderBox::maxValueChanged, mainWindow, &MainWindow::changeMaxOpacityValueDoubleSliderVolume);
		volumeTabLayout->addWidget(volumeOpacitySlider, 2, 0, 1, 2);
		volumeTab->setLayout(volumeTabLayout);
		addTab(volumeTab, tr("&Volume"));

	}
	{//Slice Tab
		sliceTab = new QWidget();
		sliceTabLayout = new QGridLayout();

		sliceTabLayout->addWidget(new QLabel("Data:"), 0, 0);
		sliceDataSelectionBox = new QComboBox();
		sliceDataSelectionBox->addItems(jet_vis::dataItems);
		sliceDataSelectionBox->setCurrentText(QString::fromStdString(state->slice.dataName.c_str()));
		connect(sliceDataSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeSliceData);
		sliceTabLayout->addWidget(sliceDataSelectionBox, 0, 1);

		std::string sliceLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->slice.dataName);
		sliceOpacitySlider = new DoubleRangeSliderBox(sliceLabelText.c_str(), state->slice.minDataValue, state->slice.maxDataValue);
		sliceOpacitySlider->setStepSize((state->slice.maxDataValue - state->slice.minDataValue) / 100.0);
		sliceOpacitySlider->setValues(state->slice.minOpacityValue, state->slice.maxOpacityValue);
		connect(sliceOpacitySlider, &DoubleRangeSliderBox::minValueChanged, mainWindow, &MainWindow::changeMinOpacityValueDoubleSliderSlice);
		connect(sliceOpacitySlider, &DoubleRangeSliderBox::maxValueChanged, mainWindow, &MainWindow::changeMaxOpacityValueDoubleSliderSlice);
		sliceTabLayout->addWidget(sliceOpacitySlider, 1, 0, 1, 2);

		sliceTabLayout->addWidget(new QLabel("Color transferfunction:"), 2, 0);
		sliceTfSelectionBox = new QComboBox();
		sliceTfSelectionBox->addItems(jet_vis::transferItems);
		sliceTfSelectionBox->setCurrentText(QString::fromStdString((jet_vis::tf_tfName_map).at(state->slice.transferfunction)));
		connect(sliceTfSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeSliceTransferfuntion);
		sliceTabLayout->addWidget(sliceTfSelectionBox, 2, 1);

		sliceTabLayout->addWidget(new QLabel("Axis:"), 3, 0);
		sliceAxisSelectionBox = new QComboBox();
		sliceAxisSelectionBox->addItems(jet_vis::sliceSelectionItems);
		connect(sliceAxisSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeOrientation);
		sliceTabLayout->addWidget(sliceAxisSelectionBox, 3, 1);

		sliceTabLayout->addWidget(new QLabel("Position:"), 4, 0);
		sliceAxisPositionSpinBox = new QDoubleSpinBox();
		switch (state->slice.orientation) {
		case(jet_vis::ORIENTATION::X): {
			sliceAxisPositionSpinBox->setMaximum(179.5);
			sliceAxisPositionSpinBox->setMinimum(-180);
			sliceAxisPositionSpinBox->setSingleStep(0.5);
			sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getLonAxis(), state->slice.location, false)); break;
		}
		case(jet_vis::ORIENTATION::Y): {
			sliceAxisPositionSpinBox->setMaximum(90);
			sliceAxisPositionSpinBox->setMinimum(-90);
			sliceAxisPositionSpinBox->setSingleStep(0.5);
			sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getLatAxis(), state->slice.location, false)); break;
		}
		case(jet_vis::ORIENTATION::Z): {
			if (state->slice.dataName == "QGOmega") {
				sliceAxisPositionSpinBox->setMaximum(1000);
				sliceAxisPositionSpinBox->setMinimum(200);
				sliceAxisPositionSpinBox->setSingleStep(25);
			}
			else {
				sliceAxisPositionSpinBox->setMaximum(1040);
				sliceAxisPositionSpinBox->setMinimum(10);
				sliceAxisPositionSpinBox->setSingleStep(10);
			}
			sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getPsAxis(), state->slice.location, true)); break;
		}
		}
		connect(sliceAxisPositionSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeLocation);
		sliceTabLayout->addWidget(sliceAxisPositionSpinBox, 4, 1);

		sliceAxisSlider = new QSlider(parent);
		sliceAxisSlider->setRange(0, state->slice.dataDimensions[2] - 1);
		int r = state->slice.dataDimensions[2] - 1;
		sliceAxisSlider->setOrientation(Qt::Horizontal);
		connect(sliceAxisSlider, &QSlider::valueChanged, mainWindow, &MainWindow::changeLocationSlider);
		sliceTabLayout->addWidget(sliceAxisSlider, 5, 0, 1, 2);

		sliceTab->setLayout(sliceTabLayout);
		addTab(sliceTab, tr("&Slice"));
	}
	{//Iso Tab
		isoTab = new QWidget();
		isoTabLayout = new QGridLayout();

		isoTabLayout->addWidget(new QLabel("Data:"), 0, 0);
		isoDataSelectionBox = new QComboBox();
		isoDataSelectionBox->addItems(jet_vis::dataItems);
		isoDataSelectionBox->setCurrentText(QString::fromStdString(state->iso.dataName.c_str()));
		connect(isoDataSelectionBox, &QComboBox::currentTextChanged, mainWindow, &MainWindow::changeIsoData);
		isoTabLayout->addWidget(isoDataSelectionBox, 0, 1);

		std::string isoInfoLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->iso.dataName);
		std::string isoDataLabelText = "[" + std::to_string(state->iso.minDataValue) + ", " + std::to_string(state->iso.maxDataValue) + "]";
		isoScalarInfoLabel = new QLabel(isoInfoLabelText.c_str());
		isoScalarDataLabel = new QLabel(isoDataLabelText.c_str());
		isoTabLayout->addWidget(isoScalarInfoLabel, 1, 0);
		isoTabLayout->addWidget(isoScalarDataLabel, 1, 1);

		isoTabLayout->addWidget(new QLabel("Iso value:"), 2, 0);
		isoValueSpinBox = new QDoubleSpinBox();
		isoValueSpinBox->setMinimum(state->iso.minDataValue);
		isoValueSpinBox->setMaximum(state->iso.maxDataValue);
		isoValueSpinBox->setDecimals(5);
		isoValueSpinBox->setSingleStep(static_cast<double>((state->iso.maxDataValue - state->iso.minDataValue) / 100.));
		isoValueSpinBox->setValue(state->iso.level);
		connect(isoValueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeIsoValue);
		isoTabLayout->addWidget(isoValueSpinBox, 2, 1);
		isoTabLayout->addWidget(new QLabel(""), 3, 0, 4, 2);


		isoTab->setLayout(isoTabLayout);
		addTab(isoTab, tr("&Iso surface"));
	}
	{//Jet Tab
		jetTab = new QWidget();
		jetTabLayout = new QGridLayout();

		jetTabLayout->addWidget(new QLabel("n_predictor_steps:"), 0, 0);
		jetTabLayout->addWidget(new QLabel("n_corrector_steps:"), 1, 0);
		jetTabLayout->addWidget(new QLabel("max_steps_below_speed_thresh:"), 2, 0);
		jetTabLayout->addWidget(new QLabel("wind_speed_threshold:"), 3, 0);
		jetTabLayout->addWidget(new QLabel("integration_stepsize:"), 4, 0);
		jetTabLayout->addWidget(new QLabel("kdtree_radius:"), 5, 0);
		jetTabLayout->addWidget(new QLabel("ps_min_val:"), 6, 0);
		jetTabLayout->addWidget(new QLabel("ps_max_val:"), 7, 0);
		jetTabLayout->addWidget(new QLabel("split_merge_threshold:"), 8, 0);
		jetTabLayout->addWidget(new QLabel("min_jet_distance:"), 9, 0);
		jetTabLayout->addWidget(new QLabel("Constant tube radius:"), 10, 0);

		n_predictor_stepsBox = new QSpinBox();
		n_predictor_stepsBox->setObjectName("n_predictor_stepsBox");
		n_predictor_stepsBox->setValue(state->jetParams.n_predictor_steps);
		connect(n_predictor_stepsBox, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsInt);
		jetTabLayout->addWidget(n_predictor_stepsBox, 0, 1);

		n_corrector_stepsBox = new QSpinBox();
		n_corrector_stepsBox->setObjectName("n_corrector_stepsBox");
		n_corrector_stepsBox->setValue(state->jetParams.n_corrector_steps);
		connect(n_corrector_stepsBox, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsInt);
		jetTabLayout->addWidget(n_corrector_stepsBox, 1, 1);

		max_steps_below_speed_threshBox = new QSpinBox();
		max_steps_below_speed_threshBox->setObjectName("max_steps_below_speed_threshBox");
		max_steps_below_speed_threshBox->setMaximum(1000);
		max_steps_below_speed_threshBox->setMinimum(0);
		max_steps_below_speed_threshBox->setValue(state->jetParams.max_steps_below_speed_thresh);
		connect(max_steps_below_speed_threshBox, QOverload<int>::of(&QSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsInt);
		jetTabLayout->addWidget(max_steps_below_speed_threshBox, 2, 1);

		wind_speed_thresholdBox = new QDoubleSpinBox();
		wind_speed_thresholdBox->setObjectName("wind_speed_thresholdBox");
		wind_speed_thresholdBox->setValue(state->jetParams.wind_speed_threshold);
		connect(wind_speed_thresholdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(wind_speed_thresholdBox, 3, 1);

		integration_stepsizeBox = new QDoubleSpinBox();
		integration_stepsizeBox->setObjectName("integration_stepsizeBox");
		integration_stepsizeBox->setValue(state->jetParams.integration_stepsize);
		connect(integration_stepsizeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(integration_stepsizeBox, 4, 1);

		kdtree_radiusBox = new QDoubleSpinBox();
		kdtree_radiusBox->setObjectName("kdtree_radiusBox");
		kdtree_radiusBox->setValue(state->jetParams.kdtree_radius);
		connect(kdtree_radiusBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(kdtree_radiusBox, 5, 1);

		ps_min_valBox = new QDoubleSpinBox();
		ps_min_valBox->setObjectName("ps_min_valBox");
		ps_min_valBox->setMaximum(1000);
		ps_min_valBox->setMinimum(0);
		ps_min_valBox->setValue(state->jetParams.ps_min_val);
		connect(ps_min_valBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(ps_min_valBox, 6, 1);

		ps_max_valBox = new QDoubleSpinBox();
		ps_max_valBox->setObjectName("ps_max_valBox");
		ps_max_valBox->setMaximum(1000);
		ps_max_valBox->setMinimum(0);
		ps_max_valBox->setValue(state->jetParams.ps_max_val);
		connect(ps_max_valBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(ps_max_valBox, 7, 1);

		split_merge_thresholdBox = new QDoubleSpinBox();
		split_merge_thresholdBox->setObjectName("split_merge_thresholdBox");
		split_merge_thresholdBox->setValue(state->jetParams.split_merge_threshold);
		connect(split_merge_thresholdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(split_merge_thresholdBox, 8, 1);

		min_jet_distance = new QDoubleSpinBox();
		min_jet_distance->setObjectName("min_jet_distance");
		min_jet_distance->setValue(state->jetParams.min_jet_distance);
		connect(min_jet_distance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), mainWindow, &MainWindow::changeJetParamsDouble);
		jetTabLayout->addWidget(min_jet_distance, 9, 1);

		jet_constant_tube_radius_check_box = new QCheckBox();
		int tube_state = state->jet.constant_tube_radius ? Qt::Checked : Qt::Unchecked;
		jet_constant_tube_radius_check_box->setCheckState(Qt::CheckState(tube_state));
		connect(jet_constant_tube_radius_check_box, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeJetConstantTubeRadius);
		jetTabLayout->addWidget(jet_constant_tube_radius_check_box, 10, 1);

		jetTab->setLayout(jetTabLayout);
		addTab(jetTab, tr("&Jet Stream"));
	}
}
void OptionTabWidget::update() {
	MainWindow::GuiState* state = mainWindow->getState();
	{//iso Tab
		isoValueSpinBox->blockSignals(true);
		isoValueSpinBox->setMaximum(state->iso.maxDataValue);
		isoValueSpinBox->setMinimum(state->iso.minDataValue);
		isoValueSpinBox->setSingleStep(static_cast<double>((state->iso.maxDataValue - state->iso.minDataValue) / 100.));
		isoValueSpinBox->setValue(state->iso.level);
		isoValueSpinBox->blockSignals(false);

		std::string isoInfoLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->iso.dataName);
		std::string isoDataLabelText = "[" + std::to_string(state->iso.minDataValue) + ", " + std::to_string(state->iso.maxDataValue) + "]";
		isoScalarInfoLabel->setText(isoInfoLabelText.c_str());
		isoScalarDataLabel->setText(isoDataLabelText.c_str());
	}
	{//Slice Tab
		sliceAxisPositionSpinBox->blockSignals(true);
		sliceAxisSlider->blockSignals(true);
		switch (state->slice.orientation) {
		case(jet_vis::ORIENTATION::X):
			sliceAxisPositionSpinBox->setRange(-180, 179.5);
			sliceAxisPositionSpinBox->setSingleStep(0.5);
			sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getLonAxis(), state->slice.location, false));
			sliceAxisSlider->setRange(0, state->slice.dataDimensions[0] - 1);
			break;
		case(jet_vis::ORIENTATION::Y):
			sliceAxisPositionSpinBox->setRange(-90, 90);
			sliceAxisPositionSpinBox->setSingleStep(0.5);
			sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getLatAxis(), state->slice.location, false));
			sliceAxisSlider->setRange(0, state->slice.dataDimensions[1] - 1);
			break;
		case(jet_vis::ORIENTATION::Z):
			if (state->slice.dataName == "QGOmega") {
				sliceAxisPositionSpinBox->setMaximum(1000);
				sliceAxisPositionSpinBox->setMinimum(200);
				sliceAxisPositionSpinBox->setSingleStep(25);
				sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(jet_vis::qgOmegaAxisValues, state->slice.location, true));
			}
			else {
				sliceAxisPositionSpinBox->setMaximum(1040);
				sliceAxisPositionSpinBox->setMinimum(10);
				sliceAxisPositionSpinBox->setSingleStep(10);
				sliceAxisPositionSpinBox->setValue(CoordinateConverter::valueOfIndexInArray(*mainWindow->getPsAxis(), state->slice.location, true));
			}
			sliceAxisSlider->setRange(0, state->slice.dataDimensions[2] - 1);
			break;
		}
		sliceAxisSlider->setSliderPosition(state->slice.location);
		sliceAxisPositionSpinBox->blockSignals(false);
		sliceAxisSlider->blockSignals(false);

		double scaling_factor = 1;
		if (state->slice.dataName == "IWC" || state->slice.dataName == "LWC" || state->slice.dataName == "RWC" || state->slice.dataName == "SWC") {
			scaling_factor = 1000000;
		}
		sliceOpacitySlider->blockSignals(true);
		sliceOpacitySlider->setStepSize((state->slice.maxDataValue * scaling_factor - state->slice.minDataValue * scaling_factor) / 1000.0);
		sliceOpacitySlider->setLowerBound(state->slice.minDataValue * scaling_factor);
		sliceOpacitySlider->setUpperBound(state->slice.maxDataValue * scaling_factor);
		sliceOpacitySlider->setValues(state->slice.minOpacityValue * scaling_factor, state->slice.maxOpacityValue * scaling_factor);
		sliceOpacitySlider->blockSignals(false);
		std::string sliceLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->slice.dataName);
		sliceOpacitySlider->setLabelText(sliceLabelText);
	}
	{//Volume Tab
		volumeOpacitySlider->blockSignals(true);
		double scaling_factor = 1;
		if (state->volume.dataName == "IWC" || state->volume.dataName == "LWC" || state->volume.dataName == "RWC" || state->volume.dataName == "SWC") {
			scaling_factor = 1000000;
		}
		volumeOpacitySlider->setStepSize((state->volume.maxDataValue * scaling_factor - state->volume.minDataValue * scaling_factor) / 1000.0);
		volumeOpacitySlider->setLowerBound(state->volume.minDataValue * scaling_factor);
		volumeOpacitySlider->setUpperBound(state->volume.maxDataValue * scaling_factor);
		volumeOpacitySlider->setValues(state->volume.minOpacityValue * scaling_factor, state->volume.maxOpacityValue * scaling_factor);
		volumeOpacitySlider->blockSignals(false);
		std::string volumeLabelText = "Scalar range" + (jet_vis::scalarFieldUnits).at(state->volume.dataName);
		volumeOpacitySlider->setLabelText(volumeLabelText);
	}
	{
		setTabEnabled(0, state->wcb.active);
		setTabEnabled(1, state->volume.active);
		setTabEnabled(2, state->slice.active);
		setTabEnabled(3, state->iso.active);
	}
}
