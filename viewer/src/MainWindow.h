#pragma once
#include <QMainWindow>
#include <QtWidgets>
#include <vtkSmartPointer.h>
#include <vtkVolume.h>

#include "core/JetStream.hpp"

#include "VisualizationHelper.h"
#include "VisualizationDataLoader.h"
#include "MenuWidget.h"

class DataVisualization;
class SceneWidget;
class vtkActor;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private:
	//Parameter
	std::vector<vtkSmartPointer<vtkProp>> activeActors;
	std::vector<std::string> time_steps;

	//Functions
	void createActions();
	void createMenus();

	void updateScene(const bool& reloadAllActors);
	void updateViewChecks();
	void updateActor(const jet_vis::ACTORS& act);
	void updateActors(const std::vector<jet_vis::ACTORS>& acts);

	void addActors(SceneWidget* sw);
	void addJet(SceneWidget* sw);
	void addTropopause(SceneWidget* sw);
	void addWcb(SceneWidget* sw);
	void addMap(SceneWidget* sw);
	void addVolume(SceneWidget* sw);
	void addSlice(SceneWidget* sw);
	void addIso(SceneWidget* sw);
	void addDateLabel(SceneWidget* sw);
	void addJetSplitMergePoints(SceneWidget* sw);
	void addJetSeedPoints(SceneWidget* sw);


	//Window menus and actions
	QMenu* viewMenu;
	QAction* classicAct;
	QAction* polarNorthAct;
	QAction* polarSouthAct;
	QAction* globeAct;

	QMenu* devMenu;
	QAction* startScreenshotAct;
	QAction* dateLabelAct;

	//MenuWidget widgets
	QSplitter* splitter;
	MenuWidget* menuWidget;
	QVBoxLayout* menuWidgetLayout;
	SceneWidget* sw;

public:

	//GUI State:
	struct GeneralState {
		size_t time = -1;
		size_t screenshot_end_time = -1;
		std::string screenshot_label = "";
		bool time_label_actor_active = false;
		jet_vis::VIEW view = jet_vis::VIEW::CLASSIC;
	};
	struct VolumeState {
		bool active = false;
		std::string dataName = "Not_Initialized";
		double minOpacityValue = 1;
		double maxOpacityValue = -1;
		double minDataValue = 1;
		double maxDataValue = -1;
		jet_vis::TRANSFERFUNCTION transferfunction = jet_vis::TRANSFERFUNCTION::HEAT;
	};
	struct SliceState {
		bool active = false;
		std::string dataName = "Not_Initialized";
		double minDataValue = 1;
		double maxDataValue = -1;
		int location = -1;
		jet_vis::ORIENTATION orientation = jet_vis::ORIENTATION::Z;
		int* dataDimensions = nullptr;
		jet_vis::TRANSFERFUNCTION transferfunction = jet_vis::TRANSFERFUNCTION::HEAT;
		double minOpacityValue = 1;
		double maxOpacityValue = -1;
	};
	struct IsoSurfaceState {
		bool active = false;
		std::string dataName = "Not_Initialized";
		double minDataValue = 1;
		double maxDataValue = -1;
		double level = -1;
	};
	struct WCBState {
		bool active = false;
		float horizontal_filter = -1;
		float vertical_filter = -1;
		long long points_before = -1;
		long long points_after = -1;
	};
	struct TropopauseState {
		bool active = false;
	};
	struct JetState {
		bool active = false;
		jet_vis::TRANSFERFUNCTION jet_color_transferfunction = jet_vis::TRANSFERFUNCTION::HEAT;
		bool split_merge_points_active = false;
		bool seed_points_active = false;
		bool use_preprocessed_jets = false;
		bool constant_tube_radius = true;
	};
	struct GuiState {
		VolumeState volume;
		SliceState slice;
		IsoSurfaceState iso;
		WCBState wcb;
		TropopauseState tropo;
		JetState jet;
		JetStream::JetParameters jetParams;
		GeneralState general;
	};
	GuiState state;

	//Fields
	JetStream* jetStream;
	VisualizationDataLoader* op;
	DataVisualization* data_visualization;
	std::vector<float> lonAxis;
	std::vector<float> latAxis;
	std::vector<float> psAxis;


public slots:
	//volume
	void changeVolumeData(const QString& newData);
	void changeVolumeActive(const int& check_state);
	void changeVolumeTransferfuntion(const QString& newFunction);
	void changeMinOpacityValueDoubleSliderVolume(const double& newValue);
	void changeMaxOpacityValueDoubleSliderVolume(const double& newValue);
	//slice
	void changeSliceData(const QString& newData);
	void changeSliceTransferfuntion(const QString& newFunction);
	void changeSliceActive(const int& check_state);
	void changeLocation(const double& value);
	void changeLocationSlider();
	void changeOrientation(const QString& newOrientation);
	void changeMinOpacityValueDoubleSliderSlice(const double& newValue);
	void changeMaxOpacityValueDoubleSliderSlice(const double& newValue);
	//iso
	void changeIsoData(const QString& newData);
	void changeIsoActive(const int& check_state);
	void changeIsoValue(const double& value);
	//jet
	void changeJetActive(const int& check_state);
	void changeJetSplitMergeActive(const int& check_state);
	void changeJetSeedsActive(const int& check_state);
	void changeJetParamsDouble(const double& value);
	void changeJetParamsInt(const int& value);
	void changeJetConstantTubeRadius(const int& check_state);

	//tropo
	void changeTropoActive(const int& check_state);
	//wcb
	void changeWcbActive(const int& check_state);
	void changeHorizontalFilter(const int& val);
	void changeVerticalFilter(const int& val);
	void changePointsBefore(const int& val);
	void changePointsAfter(const int& val);
	//general
	void changeView();
	void changeLabelActorActive();
	void changeTime();
	void changeScreenshotEndTime(const QDateTime& datetime);
	void changeScreenShotLabel(const QString& text);
	void changeQTime(const QDateTime& datetime);
	void screenshotButtonClicked();
	void takeScreenshots();

public:
	GuiState* getState() { return &state; }
	VisualizationDataLoader* getDataloader() { return op; }
	std::vector<float>* getLonAxis() { return &lonAxis; }
	std::vector<float>* getLatAxis() { return &latAxis; }
	std::vector<float>* getPsAxis() { return &psAxis; }
};
