#pragma once
#include <QtWidgets>

class MenuWidget;
class MainWindow;
class DoubleRangeSliderBox;

class OptionTabWidget : public QTabWidget {
	Q_OBJECT
public:
	OptionTabWidget(MenuWidget* parent);
	void update();
	MenuWidget* parent;
	MainWindow* mainWindow;

	QWidget* wcbTab;

	QGridLayout* wcbTabLayout;
	QComboBox* wcbDataSelectionBox;
	QComboBox* wcbTfSelectionBox;
	QSpinBox* horizontalFilterLine;
	QSpinBox* verticalFilterLine;
	QSpinBox* pointsBeforeLine;
	QSpinBox* pointsAfterLine;
	QCheckBox* onlyOnePointCheckBox;
	QLabel* wcbScalarInfoLabel;
	QLabel* wcbScalarDataLabel;

	QWidget* volumeTab;

	QGridLayout* volumeTabLayout;
	QComboBox* volumeDataSelectionBox;
	QComboBox* volumeTfSelectionBox;
	DoubleRangeSliderBox* volumeOpacitySlider;
	QWidget* sliceTab;
	QGridLayout* sliceTabLayout;
	QComboBox* sliceDataSelectionBox;
	QComboBox* sliceTfSelectionBox;
	DoubleRangeSliderBox* sliceOpacitySlider;
	QComboBox* sliceAxisSelectionBox;
	QDoubleSpinBox* sliceAxisPositionSpinBox;
	QSlider* sliceAxisSlider;

	QWidget* isoTab;

	QGridLayout* isoTabLayout;
	QComboBox* isoDataSelectionBox;
	QDoubleSpinBox* isoValueSpinBox;
	QLabel* isoScalarInfoLabel;
	QLabel* isoScalarDataLabel;

	QWidget* jetTab;

	QGridLayout* jetTabLayout;
	QSpinBox* n_predictor_stepsBox;
	QSpinBox* n_corrector_stepsBox;
	QSpinBox* max_steps_below_speed_threshBox;
	QDoubleSpinBox* wind_speed_thresholdBox;
	QDoubleSpinBox* integration_stepsizeBox;
	QDoubleSpinBox* kdtree_radiusBox;
	QDoubleSpinBox* ps_min_valBox;
	QDoubleSpinBox* ps_max_valBox;
	QSpinBox* n_steps_before_pushing_position_to_jetBox;
	QDoubleSpinBox* split_merge_thresholdBox;
	QSpinBox* tube_radius_scaling_factorBox;
	QSpinBox* stopping_criteria_jetBox;
	QSpinBox* min_jet_sizeBox;
	QDoubleSpinBox* min_jet_distance;
	QCheckBox* jet_constant_tube_radius_check_box;


};
