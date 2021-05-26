#pragma once
#include <filesystem>

#include <QtWidgets>

#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkCamera.h>

#include "core/TimeHelper.hpp"
#include "core/DataHelper.hpp"

#include "MenuWidget.h"
#include "TimeGroupBox.h"
#include "OptionTabWidget.h"
#include "JetActor.h"
#include "JetSplitMergeActor.h"
#include "JetSeedsActor.h"
#include "data_visualization.h"
#include "geometry_visualization.h"
#include "WcbVisualization.h"
#include "Colors.h"
#include "TropopauseVisualization.h"
#include "SceneWidget.h"

#include "MainWindow.h"

MainWindow::MainWindow() :
	jetStream(nullptr)
{
	op = new VisualizationDataLoader();
	data_visualization = new DataVisualization(op);
	activeActors = std::vector<vtkSmartPointer<vtkProp>>(jet_vis::actor_id_map.size());
	//readSettingsFile(op);
	lonAxis = DataHelper::loadFloatArray("lon");
	latAxis = DataHelper::loadFloatArray("lat");
	psAxis = DataHelper::loadFloatArray("ps");
	//Initial UI state
	state = {
		{//VolumeState
			false, //active
			"Wind Magnitude", //DataName
			1, //minOpacityValue
			-1, //maxOpacityValue
			1, //minDataValue
			-1, //maxDataValue
			jet_vis::TRANSFERFUNCTION::HEAT //transferfunction
		},
		{//SliceState
			false, //active
			"Wind Magnitude", //DataName
			1, //minDataValue
			-1, //maxDataValue
			50, //location
			jet_vis::ORIENTATION::Z, //orientation
			new int[3] {0, 0, 0}, //dataDimensions
			jet_vis::TRANSFERFUNCTION::HEAT, //transferfunction
			//-25,
			//10
			40, //minOpacityValue 
			50//maxOpacityValue
		},
		{//IsoSurfaceState
			false, //active
			"Q", //DataName
			1, //minDataValue
			-1, //maxDataValue
			0.013688329806939237 // level
		},
		{//WCBState
			false, //active
			500, //horizontal_filter
			50, //vertical_filter
			5, //points_before
			2, //points_after
		},
		{//TropopauseState
			false //active
		},
		{//JetState
			true, //active
			jet_vis::TRANSFERFUNCTION::RED_TO_BLUE, //jet_color_transferfunction
			false, //Split merge points active
			false, //Seeds active
			true, //use_preprocessed_jets
			false //constant_tube_radius
		},
			1,		//n_predictor_steps
			5,		//n_corrector_steps
			100,	//max_steps_below_speed_thresh
			40,		//wind_speed_threshold
			0.04,	//integration_stepsize
			5.5,	//kdtree_radius
			190.0,	//ps_min_val
			350.0,	//ps_max_val
			90.0,	//tracing_min_value
			450,	//tracing_max_value
			0.1,	//split_merge_threshold
			10000,	//stopping_criteria_jet
			5		//min_jet_distance
		,
		{//GeneralState
			0, //time: 624 Wirbelsturm, 371: good overview, 552: Vladyana, 1204,1253: BUG with Polar North and tropo, 255: below_thresh
			0, //screenshot_end_time
			"",//Screenshot label
			false,//ja:1415 nein: 1420
			jet_vis::VIEW::CLASSIC
		}
	};

	//VectorFieldWidget* sw = new VectorFieldWidget(dataName, time_of_fields, &op);
	time_steps = DataHelper::collectTimes();

	splitter = new QSplitter();
	menuWidget = new MenuWidget(this);
	menuWidget->setMinimumWidth(50);

	sw = new SceneWidget(op);

	addActors(sw);
	auto camera = sw->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
	camera->ParallelProjectionOn();
	camera->SetParallelScale(125);
	camera->SetFocalPoint(180, 90, 0.0);
	camera->SetPosition(180, 90, 250);
	camera->SetViewUp(0.0, 1.0, 0.0);

	splitter->addWidget(menuWidget);
	splitter->addWidget(sw);
	splitter->setSizes(QList<int>() << 50 << 400);

	setCentralWidget(splitter);

	createActions();
	createMenus();
	menuWidget->update();
	setWindowTitle(tr("Jet Visualization"));
	setMinimumSize(800, 600);

	showMaximized();
	setAcceptDrops(true);

}

void MainWindow::createActions() {
	/*
		Creates the window menu items
	*/
	classicAct = new QAction(tr("&Classic"), this);
	classicAct->setObjectName(tr("&Classic"));
	classicAct->setCheckable(true);
	connect(classicAct, &QAction::triggered, this, &MainWindow::changeView);

	polarNorthAct = new QAction(tr("&Polar-stereographic North"), this);
	polarNorthAct->setObjectName(tr("&PolarNorth"));
	polarNorthAct->setCheckable(true);
	connect(polarNorthAct, &QAction::triggered, this, &MainWindow::changeView);

	polarSouthAct = new QAction(tr("&Polar-stereographic South"), this);
	polarSouthAct->setObjectName(tr("&PolarSouth"));
	polarSouthAct->setCheckable(true);
	connect(polarSouthAct, &QAction::triggered, this, &MainWindow::changeView);

	globeAct = new QAction(tr("&Globe"), this);
	globeAct->setObjectName(tr("&Globe"));
	globeAct->setCheckable(true);
	connect(globeAct, &QAction::triggered, this, &MainWindow::changeView);
	updateViewChecks();

	startScreenshotAct = new QAction(tr("&Start screenshots"));
	connect(startScreenshotAct, &QAction::triggered, this, &MainWindow::screenshotButtonClicked);

	dateLabelAct = new QAction(tr("&Date label actor"));
	dateLabelAct->setCheckable(true);
	dateLabelAct->setChecked(state.general.time_label_actor_active);
	connect(dateLabelAct, &QAction::triggered, this, &MainWindow::changeLabelActorActive);
}
void MainWindow::createMenus() {
	/*
	Creates the window menus.
	*/
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(classicAct);
	viewMenu->addAction(polarNorthAct);
	viewMenu->addAction(polarSouthAct);
	viewMenu->addAction(globeAct);

	devMenu = menuBar()->addMenu(tr("&Dev"));
	devMenu->addAction(startScreenshotAct);
	devMenu->addAction(dateLabelAct);
}
void MainWindow::updateScene(const bool& reloadAllActors) {
	SceneWidget* sw = (SceneWidget*)splitter->widget(1);
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();

	//Remove not active actors
	if ((!state.volume.active && activeActors[0] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[0]);
		activeActors[0] = nullptr;
	}
	if ((!state.iso.active && activeActors[1] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[1]);
		activeActors[1] = nullptr;
	}
	if ((!state.slice.active && activeActors[2] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[2]);
		activeActors[2] = nullptr;
	}
	if ((!state.tropo.active && activeActors[3] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[3]);
		activeActors[3] = nullptr;
	}
	if ((!state.jet.active && activeActors[5] != nullptr && activeActors[8] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[5]);
		r->RemoveActor(activeActors[8]);
		activeActors[5] = nullptr;
		activeActors[8] = nullptr;

	}
	if ((!state.jet.active && activeActors[12] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[12]);
		activeActors[12] = nullptr;
	}

	if ((!state.jet.split_merge_points_active && activeActors[10] != nullptr && activeActors[11] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[10]);
		r->RemoveActor(activeActors[11]);
		activeActors[11] = nullptr;
		activeActors[10] = nullptr;
	}
	if ((!state.wcb.active && activeActors[6] != nullptr && activeActors[7] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[6]);
		r->RemoveActor(activeActors[7]);
		activeActors[6] = nullptr;
		activeActors[7] = nullptr;
	}
	if ((!state.general.time_label_actor_active && activeActors[9] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[9]);
		activeActors[9] = nullptr;
	}
	if ((!state.jet.seed_points_active && activeActors[13] != nullptr) || reloadAllActors) {
		r->RemoveActor(activeActors[13]);
		activeActors[13] = nullptr;
	}
	if (reloadAllActors) {
		r->RemoveActor(activeActors[4]);
		delete jetStream;
		jetStream = nullptr;
		addMap(sw);
	}
	//add new active actors
	if (state.volume.active && activeActors[0] == nullptr) {
		addVolume(sw);
	}
	if (state.iso.active && activeActors[1] == nullptr) {
		addIso(sw);
	}
	if (state.slice.active && activeActors[2] == nullptr) {
		addSlice(sw);
	}
	if (state.tropo.active && activeActors[3] == nullptr) {
		addTropopause(sw);
	}
	if (state.jet.active && activeActors[5] == nullptr) {
		addJet(sw);
	}
	if (state.jet.split_merge_points_active && activeActors[10] == nullptr && activeActors[11] == nullptr) {
		addJetSplitMergePoints(sw);
	}
	if (state.wcb.active && activeActors[6] == nullptr && activeActors[7] == nullptr) {
		addWcb(sw);
	}
	if (state.general.time_label_actor_active && activeActors[9] == nullptr) {
		addDateLabel(sw);
	}
	if (state.jet.seed_points_active && activeActors[13] == nullptr) {
		addJetSeedPoints(sw);
	}
	//Render Scene
	sw->renderWindow()->Render();

}
void MainWindow::updateActor(const jet_vis::ACTORS& act) {
	/*
		Updates a single actor in the scene.
	*/
	SceneWidget* sw = (SceneWidget*)splitter->widget(1);
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	r->RemoveActor(activeActors[jet_vis::actor_id_map.at(act)]);
	switch (act) {
	case(jet_vis::ACTORS::Volume): if (state.volume.active) { addVolume(sw); } break;
	case(jet_vis::ACTORS::Iso): if (state.iso.active) { addIso(sw); } break;
	case(jet_vis::ACTORS::Slice): if (state.slice.active) { addSlice(sw); } break;
	case(jet_vis::ACTORS::Tropo): if (state.tropo.active) { addTropopause(sw); } break;
	case(jet_vis::ACTORS::Map): addMap(sw); break;
	case(jet_vis::ACTORS::Jet): if (state.jet.active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetColorBar)]); addJet(sw); } break;
	case(jet_vis::ACTORS::WcbPoints): if (state.wcb.active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::WcbExtendedPoints)]); addWcb(sw); } break;
	case(jet_vis::ACTORS::WcbExtendedPoints): if (state.wcb.active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::WcbPoints)]); addWcb(sw); } break;
	case(jet_vis::ACTORS::JetColorBar): if (state.jet.active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Jet)]); addJet(sw); } break;
	case(jet_vis::ACTORS::PVColoBar): if (state.jet.active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Jet)]); addJet(sw); } break;
	case(jet_vis::ACTORS::DateLabel):if (state.general.time_label_actor_active) { addDateLabel(sw); } break;
	case(jet_vis::ACTORS::JetMergePoints): if (state.jet.split_merge_points_active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetMergePoints)]); r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetSplitPoints)]); addJetSplitMergePoints(sw); } break;
	case(jet_vis::ACTORS::JetSplitPoints): if (state.jet.split_merge_points_active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetSplitPoints)]); r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetMergePoints)]); addJetSplitMergePoints(sw); } break;
	case(jet_vis::ACTORS::JetSeeds): if (state.jet.seed_points_active) { r->RemoveActor(activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetSeeds)]); addJetSeedPoints(sw); } break;

	}
	sw->renderWindow()->Render();
}
void MainWindow::updateActors(const std::vector<jet_vis::ACTORS>& acts) {
	for (int i = 0; i < acts.size(); i++) {
		updateActor(acts[i]);
	}
}

void MainWindow::updateViewChecks() {
	/*
		Updates the checks in the View menu.
	*/
	classicAct->setChecked(state.general.view == jet_vis::VIEW::CLASSIC);
	polarNorthAct->setChecked(state.general.view == jet_vis::VIEW::POLAR_NORTH);
	polarSouthAct->setChecked(state.general.view == jet_vis::VIEW::POLAR_SOUTH);
	globeAct->setChecked(state.general.view == jet_vis::VIEW::GLOBE);
}
/*
	Slot section: The following funtions change the state of the GUI.
*/
void MainWindow::changeTime() {
	int val = menuWidget->timeGroupBox->timeSlider->value();
	state.general.time = val;
	updateScene(true);
	menuWidget->update();
}
void MainWindow::changeScreenshotEndTime(const QDateTime& datetime) {
	std::string newTime = datetime.toString("yyyymmdd_hh").toStdString();
	size_t new_t = TimeHelper::convertDateToHours(newTime, DataHelper::getDataStartDate());
	state.general.screenshot_end_time = new_t;
	menuWidget->update();
}
void MainWindow::changeScreenShotLabel(const QString& text) {
	state.general.screenshot_label = text.toStdString();
	menuWidget->update();
}

void MainWindow::changeQTime(const QDateTime& datetime) {
	std::string newTime = datetime.toString("yyyymmdd_hh").toStdString();
	size_t new_t = TimeHelper::convertDateToHours(newTime, DataHelper::getDataStartDate());
	if (new_t <= time_steps.size() && new_t >= 0) {
		state.general.time = new_t;
		updateScene(true);
	}
	menuWidget->update();
}

void MainWindow::changeVolumeData(const QString& newData) {
	QAction* sendingMenuItem = static_cast<QAction*>(sender());
	state.volume.dataName = newData.toStdString();
	state.volume.minOpacityValue = 1;
	state.volume.maxOpacityValue = -1;
	updateActor(jet_vis::ACTORS::Volume);
	state.volume.minOpacityValue = state.volume.minDataValue;
	state.volume.maxOpacityValue = state.volume.maxDataValue;
	menuWidget->update();
}
void MainWindow::changeIsoData(const QString& newData) {
	QAction* sendingMenuItem = static_cast<QAction*>(sender());
	state.iso.dataName = newData.toStdString();
	updateActor(jet_vis::ACTORS::Iso);
	menuWidget->update();
}
void MainWindow::changeSliceData(const QString& newData) {
	QAction* sendingMenuItem = static_cast<QAction*>(sender());
	state.slice.dataName = newData.toStdString();
	state.slice.minOpacityValue = 1;
	state.slice.maxOpacityValue = -1;
	if (state.slice.dataName == "QGOmega" && state.slice.orientation == jet_vis::ORIENTATION::Z) {
		state.slice.location = 20;
	}
	updateActor(jet_vis::ACTORS::Slice);
	state.slice.minOpacityValue = state.slice.minDataValue;
	state.slice.maxOpacityValue = state.slice.maxDataValue;
	menuWidget->update();
}

void MainWindow::changeIsoValue(const double& value) {
	state.iso.level = value;
	updateActor(jet_vis::ACTORS::Iso);
}


void MainWindow::changeMinOpacityValueDoubleSliderVolume(const double& newValue) {
	double scaling_factor = 1;
	if (state.volume.dataName == "IWC" || state.volume.dataName == "LWC" || state.volume.dataName == "RWC" || state.volume.dataName == "SWC") {
		scaling_factor = 1000000;
	}
	state.volume.minOpacityValue = newValue / scaling_factor;
	updateActor(jet_vis::ACTORS::Volume);
}
void MainWindow::changeMaxOpacityValueDoubleSliderVolume(const double& newValue) {
	double scaling_factor = 1;
	if (state.volume.dataName == "IWC" || state.volume.dataName == "LWC" || state.volume.dataName == "RWC" || state.volume.dataName == "SWC") {
		scaling_factor = 1000000;
	}
	state.volume.maxOpacityValue = newValue / scaling_factor;
	updateActor(jet_vis::ACTORS::Volume);
}
void MainWindow::changeMinOpacityValueDoubleSliderSlice(const double& newValue) {
	double scaling_factor = 1;
	if (state.slice.dataName == "IWC" || state.slice.dataName == "LWC" || state.slice.dataName == "RWC" || state.slice.dataName == "SWC") {
		scaling_factor = 1000000;
	}
	state.slice.minOpacityValue = newValue / scaling_factor;
	updateActor(jet_vis::ACTORS::Slice);
}
void MainWindow::changeMaxOpacityValueDoubleSliderSlice(const double& newValue) {
	double scaling_factor = 1;
	if (state.slice.dataName == "IWC" || state.slice.dataName == "LWC" || state.slice.dataName == "RWC" || state.slice.dataName == "SWC") {
		scaling_factor = 1000000;
	}
	state.slice.maxOpacityValue = newValue / scaling_factor;
	updateActor(jet_vis::ACTORS::Slice);
}
void MainWindow::changeLocation(const double& value) {
	switch (state.slice.orientation) {
	case(jet_vis::ORIENTATION::X): {
		state.slice.location = (int)std::round(CoordinateConverter::indexOfValueInArray(lonAxis, value, false)); break;
	}
	case(jet_vis::ORIENTATION::Y): {
		state.slice.location = (int)std::round(CoordinateConverter::indexOfValueInArray(latAxis, value, false)); break;
	}
	case(jet_vis::ORIENTATION::Z): {
		if (state.slice.dataName == "QGOmega") {
			state.slice.location = (int)std::round(CoordinateConverter::indexOfValueInArray(jet_vis::qgOmegaAxisValues, value, true));
		}
		else {
			state.slice.location = (int)std::round(CoordinateConverter::indexOfValueInArray(psAxis, value, true));
		}
		break;
	}
	}
	updateActor(jet_vis::ACTORS::Slice);
	menuWidget->update();
}
void MainWindow::changeLocationSlider() {
	state.slice.location = menuWidget->optionTabWidget->sliceAxisSlider->value();;
	updateActor(jet_vis::ACTORS::Slice);
	menuWidget->update();
}
void MainWindow::changeOrientation(const QString& newOrientation) {
	std::string x = "Longitude";
	std::string y = "Latitude";
	std::string z = "Pressure";
	std::string newOr = newOrientation.toStdString();

	if (newOr == x) {
		state.slice.orientation = jet_vis::ORIENTATION::X;
		state.slice.location = 0;
	}
	else if (newOr == y) {
		state.slice.orientation = jet_vis::ORIENTATION::Y;
		state.slice.location = 0;
	}
	else if (newOr == z) {
		state.slice.orientation = jet_vis::ORIENTATION::Z;
		state.slice.location = 0;
	}
	updateActor(jet_vis::ACTORS::Slice);
	menuWidget->update();
}

void MainWindow::changeSliceTransferfuntion(const QString& newFunction) {
	std::string newTf = newFunction.toStdString();
	state.slice.transferfunction = jet_vis::tfName_tf_map.at(newTf);
	updateActor(jet_vis::ACTORS::Slice);
}

void MainWindow::changeVolumeTransferfuntion(const QString& newFunction) {
	std::string newTf = newFunction.toStdString();
	state.volume.transferfunction = jet_vis::tfName_tf_map.at(newTf);
	updateActor(jet_vis::ACTORS::Volume);
}

void MainWindow::changeJetActive(const int& check_state) {
	if (check_state == Qt::Unchecked) {
		state.jet.active = false;
	}
	else {
		state.jet.active = true;
	}

	updateActor(jet_vis::ACTORS::Jet);
	updateActor(jet_vis::ACTORS::JetColorBar);
	menuWidget->update();
}
void MainWindow::changeJetSplitMergeActive(const int& check_state) {
	if (check_state == Qt::Unchecked) {
		state.jet.split_merge_points_active = false;
	}
	else {
		state.jet.split_merge_points_active = true;
	}
	updateActor(jet_vis::ACTORS::JetMergePoints);
	updateActor(jet_vis::ACTORS::JetSplitPoints);
	menuWidget->update();

}
void MainWindow::changeJetSeedsActive(const int& check_state) {
	if (check_state == Qt::Unchecked) {
		state.jet.seed_points_active = false;
	}
	else {
		state.jet.seed_points_active = true;
	}
	updateActor(jet_vis::ACTORS::JetSeeds);
	menuWidget->update();
}

void MainWindow::changeJetParamsInt(const int& value) {
	state.jet.use_preprocessed_jets = false;
	QAction* sendingMenuItem = static_cast<QAction*>(sender());
	QString name = sendingMenuItem->objectName();
	if (name == QString("n_predictor_stepsBox")) {
		state.jetParams.n_predictor_steps = value;
	}
	else if (name == QString("n_corrector_stepsBox")) {
		state.jetParams.n_corrector_steps = value;
	}
	else if (name == QString("max_steps_below_speed_threshBox")) {
		state.jetParams.max_steps_below_speed_thresh = value;
	}
	else if (name == QString("stopping_criteria_jetBox")) {
		state.jetParams.stopping_criteria_jet = value;
	}

	updateActor(jet_vis::ACTORS::Jet);
	updateActor(jet_vis::ACTORS::JetColorBar);
	menuWidget->update();
}
void MainWindow::changeJetParamsDouble(const double& value) {
	state.jet.use_preprocessed_jets = false;
	QAction* sendingMenuItem = static_cast<QAction*>(sender());
	QString name = sendingMenuItem->objectName();
	if (name == QString("wind_speed_thresholdBox")) {
		state.jetParams.wind_speed_threshold = value;
	}
	else if (name == QString("integration_stepsizeBox")) {
		state.jetParams.integration_stepsize = value;
	}
	else if (name == QString("kdtree_radiusBox")) {
		state.jetParams.kdtree_radius = value;
	}
	else if (name == QString("ps_min_valBox")) {
		state.jetParams.ps_min_val = value;
	}
	else if (name == QString("ps_max_valBox")) {
		state.jetParams.ps_max_val = value;
	}
	else if (name == QString("split_merge_thresholdBox")) {
		state.jetParams.split_merge_threshold = value;
	}
	else if (name == QString("min_jet_distance")) {
		state.jetParams.min_jet_distance = value;
	}
	updateActor(jet_vis::ACTORS::Jet);
	updateActor(jet_vis::ACTORS::JetColorBar);
	menuWidget->update();

}
void MainWindow::changeJetConstantTubeRadius(const int& check_state) {
	state.jet.constant_tube_radius = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::Jet);
	updateActor(jet_vis::ACTORS::JetColorBar);
	menuWidget->update();
}

void MainWindow::changeTropoActive(const int& check_state) {
	state.tropo.active = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::Tropo);
	menuWidget->update();
}

void MainWindow::changeWcbActive(const int& check_state) {
	state.wcb.active = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::WcbPoints);
	updateActor(jet_vis::ACTORS::WcbExtendedPoints);
	menuWidget->update();
}

void MainWindow::changeVolumeActive(const int& check_state) {
	state.volume.active = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::Volume);
	menuWidget->update();
}

void MainWindow::changeSliceActive(const int& check_state) {
	state.slice.active = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::Slice);
	menuWidget->update();
}

void MainWindow::changeIsoActive(const int& check_state) {
	state.iso.active = (check_state == Qt::Checked);
	updateActor(jet_vis::ACTORS::Iso);
	menuWidget->update();
}

void MainWindow::changeHorizontalFilter(const int& val) {
	state.wcb.horizontal_filter = val;
	updateActor(jet_vis::ACTORS::WcbPoints);
}

void MainWindow::changeVerticalFilter(const int& val) {
	state.wcb.vertical_filter = val;
	updateActor(jet_vis::ACTORS::WcbPoints);
}

void MainWindow::changePointsBefore(const int& val) {
	state.wcb.points_before = val;
	updateActor(jet_vis::ACTORS::WcbPoints);
}

void MainWindow::changePointsAfter(const int& val) {
	state.wcb.points_after = val;
	updateActor(jet_vis::ACTORS::WcbPoints);
}

void MainWindow::changeView() {
	QAction* act = static_cast<QAction*>(sender());
	QString name = act->objectName();
	if (name == tr("&Classic")) {
		state.general.view = jet_vis::VIEW::CLASSIC;
		updateScene(true);
		sw->addAxes();
	}
	else if (name == tr("&PolarNorth")) {
		state.general.view = jet_vis::VIEW::POLAR_NORTH;
		state.volume.active = false;
		state.slice.active = false;
		state.iso.active = false;
		sw->removeAxes();
		updateScene(true);
	}
	else if (name == tr("&PolarSouth")) {
		state.general.view = jet_vis::VIEW::POLAR_SOUTH;
		state.volume.active = false;
		state.slice.active = false;
		state.iso.active = false;
		sw->removeAxes();
		updateScene(true);
	}
	else if (name == tr("&Globe")) {
		state.general.view = jet_vis::VIEW::GLOBE;
		state.volume.active = false;
		sw->removeAxes();
		updateScene(true);
	}

	updateViewChecks();
	menuWidget->update();
}
void MainWindow::changeLabelActorActive() {
	if (state.general.time_label_actor_active) {
		state.general.time_label_actor_active = false;
	}
	else {
		state.general.time_label_actor_active = true;
	}
	dateLabelAct->setChecked(state.general.time_label_actor_active);
	updateActor(jet_vis::ACTORS::DateLabel);
}
/*
	The following funtions add actors to the scene.
*/
void MainWindow::addActors(SceneWidget* sw) {
	addMap(sw);
	if (state.general.time_label_actor_active) {
		addDateLabel(sw);
	}
	if (state.jet.active) {
		addJet(sw);
	}
	if (state.tropo.active) {
		addTropopause(sw);

	}
	if (state.wcb.active) {
		addWcb(sw);
	}
	if (state.volume.active) {
		addVolume(sw);
	}
	if (state.slice.active) {
		addSlice(sw);
	}
	if (state.iso.active) {
		addIso(sw);
	}
	if (state.jet.seed_points_active) {
		addJetSeedPoints(sw);
	}

	sw->RenderBackground(state.general.view == jet_vis::VIEW::CLASSIC);

}

void MainWindow::addJet(SceneWidget* sw) {
	vtkSmartPointer<vtkActor> jetLines;

	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	JetActor jetAct(state.general.time, state.jet.jet_color_transferfunction, state.general.view, op, state.jetParams, state.jet.use_preprocessed_jets, state.jet.constant_tube_radius);
	jetLines = jetAct.get();
	jetStream = jetAct.get_jet();
	jet_vis::view_transform(jetLines, state.general.view, sw);
	vtkSmartPointer<vtkActor2D> colorBarAct = Colors::CreateColorBarActor("Jet speed\n [m/s]\n", state.jet.jet_color_transferfunction, 10, 110, 1.15);

	r->AddActor(jetLines);
	r->AddActor2D(colorBarAct);

	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Jet)] = jetLines;
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetColorBar)] = colorBarAct;
	//vtkSmartPointer<vtkActor2D> colorBarAct = Colors::CreateColorBarActor("Jet speed\n [m/s]\n", state.jet.jet_color_transferfunction, pc.getMinAttributeValue(), pc.getMaxAttributeValue(), 1.15);
	vtkSmartPointer<vtkActor2D> colorBarAct2 = Colors::CreateColorBarActor("      PV     \n [pvu]\n", jet_vis::TRANSFERFUNCTION::GREEN_TO_VIOLET, -2, 2, 0);
	//vtkSmartPointer<vtkActor2D> colorBarAct = Colors::CreateColorBarActor("Wind \n magnit. \n [m/s]\n", Colors::TRANSFERFUNCTION::YELLOW_TO_RED, 40, 109, 1.15);

	//r->AddActor2D(colorBarAct2);

	//activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::PVColoBar)] = colorBarAct2;
}
void MainWindow::addJetSeedPoints(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkActor> seed_act = JetSeedsActor(op, state.general.time, state.jetParams, jetStream).get();
	r->AddActor(seed_act);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetSeeds)] = seed_act;
}

void MainWindow::addJetSplitMergePoints(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();

	JetSplitMergeActor splitMerge = JetSplitMergeActor(state.general.time, op, state.general.view, jetStream);
	vtkSmartPointer<vtkActor> merge_act = splitMerge.getMergeActor();
	vtkSmartPointer<vtkActor> split_act = splitMerge.getSplitActor();

	jet_vis::view_transform(merge_act, state.general.view, sw);
	jet_vis::view_transform(split_act, state.general.view, sw);

	r->AddActor(merge_act);
	r->AddActor(split_act);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetMergePoints)] = merge_act;
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::JetSplitPoints)] = split_act;

}
void MainWindow::addTropopause(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	TropopauseVisualization tropo(op, state.general.time);
	vtkSmartPointer<vtkActor> act = tropo.getActor(state.general.view);;
	jet_vis::view_transform(act, state.general.view, sw);
	r->AddActor(act);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Tropo)] = act;

}
void MainWindow::addWcb(SceneWidget* sw) {
	WcbVisualization wcbVis(op, state.general.time,
		state.general.view,
		state.wcb.horizontal_filter,
		state.wcb.vertical_filter,
		state.wcb.points_before,
		state.wcb.points_after);
	vtkSmartPointer<vtkActor> wcbPointsActor = wcbVis.getPointsActor();
	vtkSmartPointer<vtkActor> wcbExtendedPointsActor = wcbVis.getExtensionsActor();

	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();

	r->AddActor(wcbPointsActor);
	r->AddActor(wcbExtendedPointsActor);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::WcbPoints)] = wcbPointsActor;
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::WcbExtendedPoints)] = wcbExtendedPointsActor;

}
void MainWindow::addMap(SceneWidget* sw) {

	vtkSmartPointer<vtkActor> mapAct = GeometryVisualization::create_map_actor(state.general.view);
	jet_vis::view_transform(mapAct, state.general.view, sw);

	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	r->AddActor(mapAct);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Map)] = mapAct;
}
void MainWindow::addDateLabel(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkTextActor> act = GeometryVisualization::create_date_label_follower(state.general.time, op);
	r->AddActor2D(act);
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::DateLabel)] = act;
}
void MainWindow::addVolume(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkVolume> act = data_visualization->create_volume_actor(state.volume.dataName, state.general.time, state.volume.minOpacityValue, state.volume.maxOpacityValue, state.volume.transferfunction);
	r->AddActor(act);
	state.volume.minDataValue = data_visualization->get_volume_data().min_data_value;
	state.volume.maxDataValue = data_visualization->get_volume_data().max_data_value;
	if (state.volume.minOpacityValue > state.volume.maxOpacityValue) {
		state.volume.minOpacityValue = state.volume.minDataValue;
		state.volume.maxOpacityValue = state.volume.maxDataValue;
	}
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Volume)] = act;
	menuWidget->update();
}
void MainWindow::addSlice(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkActor> act = data_visualization->create_slice_actor(state.slice.dataName, state.general.time, state.slice.orientation, state.slice.location, state.slice.transferfunction, state.slice.minOpacityValue, state.slice.maxOpacityValue);
	if (state.general.view == jet_vis::VIEW::GLOBE) {
		jet_vis::view_transform(act, state.general.view, sw);
	}

	r->AddActor(act);
	state.slice.minDataValue = data_visualization->get_slice_data().min_data_value;
	state.slice.maxDataValue = data_visualization->get_slice_data().max_data_value;
	if (state.slice.minOpacityValue > state.slice.maxOpacityValue) {
		state.slice.minOpacityValue = state.slice.minDataValue;
		state.slice.maxOpacityValue = state.slice.maxDataValue;
	}
	state.slice.dataDimensions = data_visualization->get_slice_data().dimensions;
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Slice)] = act;
	menuWidget->update();
}
void MainWindow::addIso(SceneWidget* sw) {
	vtkSmartPointer<vtkRenderer> r = sw->renderWindow()->GetRenderers()->GetFirstRenderer();
	vtkSmartPointer<vtkActor> act = data_visualization->create_iso_actor(state.iso.dataName, state.general.time, state.iso.level);
	if (state.general.view == jet_vis::VIEW::GLOBE) {
		jet_vis::view_transform(act, state.general.view, sw);
	}
	r->AddActor(act);
	state.iso.minDataValue = data_visualization->get_iso_data().min_data_value;
	state.iso.maxDataValue = data_visualization->get_iso_data().max_data_value;
	activeActors[jet_vis::actor_id_map.at(jet_vis::ACTORS::Iso)] = act;
}

/*
	Helper functions
*/
void MainWindow::takeScreenshots()
{
	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);
	std::string name = std::to_string(now->tm_mday)
		+ "_" + std::to_string(now->tm_mon + 1)
		+ "_" + std::to_string(now->tm_year + 1900)
		+ "_" + std::to_string(now->tm_hour)
		+ "_" + std::to_string(now->tm_min)
		+ "_" + state.general.screenshot_label + "\\";
	size_t start_time = state.general.time;
	for (size_t i = state.general.time; i <= state.general.screenshot_end_time; i++) { //walk through timesteps
		if (i != start_time) {
			state.general.time = i;
			updateScene(true);
			menuWidget->update();
		}
		QCoreApplication::processEvents();
		QPixmap map = sw->grab();
		std::string path = "Screenshots\\";
		if (!std::filesystem::exists(path)) { std::filesystem::create_directory("Screenshots\\"); }

		std::string path_sub = path + name;
		std::filesystem::create_directory(path_sub);
		std::string old_string = std::to_string(i);
		std::string new_string = std::string(4 - old_string.length(), '0') + old_string;

		std::string pic_name = path_sub + "frame" + new_string + ".png";

		bool result = map.save(pic_name.c_str(), "png");

	}
}
void MainWindow::screenshotButtonClicked() {
	takeScreenshots();
}
