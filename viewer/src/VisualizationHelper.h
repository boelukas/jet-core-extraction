#pragma once
#include <QtWidgets>

#include "core/Math.hpp"

class LineCollection;
class SceneWidget;
class vtkActor;
template <class vtkActor> class vtkSmartPointer;

namespace jet_vis
{
	enum class HEMISPHERE { BOTH, NORTH, SOUTH };
	enum class REPRESENTATION { VOLUME, ISO, SLICE };
	enum class ACTORS { Volume, Iso, Slice, Tropo, Map, Jet, WcbPoints, WcbExtendedPoints, JetColorBar, DateLabel, JetMergePoints, JetSplitPoints, PVColoBar, JetSeeds };
	enum class VIEW { CLASSIC, POLAR_NORTH, POLAR_SOUTH, GLOBE };
	enum class TRANSFERFUNCTION { HEAT, RED_TO_BLUE, WHITE_TO_BLUE, WHITE_TO_RED, YELLOW_TO_RED, GREEN_TO_VIOLET };
	enum class ORIENTATION { X, Y, Z };

	const std::vector<float> qgOmegaAxisValues = std::vector<float>({ 200,225,250,275,300,325,350,375,400,425,450,475,500,525,550,575,600,625,650,675,700,725,750,775,800,825,850,875,900,925,950,975,1000 });
	const QStringList dataItems = { "Q", "T", "OMEGA","LWC", "IWC", "RWC", "SWC", "U", "V", "PV", "THE", "Wind Magnitude", "QGOmega" };
	const QStringList transferItems = { "Heat", "Red to Blue", "White to Blue", "White to Red", "Yellow to Red", "Green to Violet" };
	const QStringList sliceSelectionItems = { "Pressure", "Longitude", "Latitude" };
	const std::unordered_map<std::string, std::string> scalarFieldUnits = { {"Q", "[kg/kg]"},{"T", "[K]"}, {"OMEGA", "[Pa/s]"}, {"LWC", "[mg/kg]"}, {"RWC", "[mg/kg]"}, {"IWC", "[mg/kg]"},{"SWC", "[mg/kg]"}, {"U", "[m/s]"}, {"V", "[m/s]"}, {"PV", "[pvu]"}, {"THE", "[K]"}, {"Wind Magnitude", "[m/s]"} };
	const std::unordered_map< ACTORS, int> actor_id_map = {
		{ACTORS::Volume, 0},
		{ACTORS::Iso, 1},
		{ACTORS::Slice, 2},
		{ACTORS::Tropo, 3},
		{ACTORS::Map, 4},
		{ACTORS::Jet, 5},
		{ACTORS::WcbPoints, 6},
		{ACTORS::WcbExtendedPoints, 7},
		{ACTORS::JetColorBar, 8},
		{ACTORS::DateLabel, 9},
		{ACTORS::JetMergePoints, 10},
		{ACTORS::JetSplitPoints, 11},
		{ACTORS::PVColoBar, 12},
		{ACTORS::JetSeeds, 13}
	};
	const std::unordered_map<std::string, TRANSFERFUNCTION> tfName_tf_map = {
		{"Heat", TRANSFERFUNCTION::HEAT},
		{"Red to Blue",TRANSFERFUNCTION::RED_TO_BLUE},
		{"White to Blue", TRANSFERFUNCTION::WHITE_TO_BLUE},
		{"White to Red", TRANSFERFUNCTION::WHITE_TO_RED},
		{"Yellow to Red", TRANSFERFUNCTION::YELLOW_TO_RED},
		{"Green to Violet", TRANSFERFUNCTION::GREEN_TO_VIOLET }
	};
	const std::unordered_map<TRANSFERFUNCTION, std::string> tf_tfName_map = {
	{TRANSFERFUNCTION::HEAT,"Heat"},
	{TRANSFERFUNCTION::RED_TO_BLUE,"Red to Blue"},
	{TRANSFERFUNCTION::WHITE_TO_BLUE, "White to Blue"},
	{TRANSFERFUNCTION::WHITE_TO_RED, "White to Red"},
	{TRANSFERFUNCTION::YELLOW_TO_RED, "Yellow to Red"},
	{TRANSFERFUNCTION::GREEN_TO_VIOLET, "Green to Violet"}
	};
	const int MAP_RADIUS = 200;
	const double SPACING[] = { 0.5, 0.5, 1.0 };

	void view_filter(LineCollection& lines, const VIEW& view);
	void view_filter(Points3d& points, const VIEW& view);
	void view_transform(Points3d& points, const VIEW& view);
	void view_transform(LineCollection& lines, const VIEW& view);
	void view_transform(vtkSmartPointer<vtkActor> act, const VIEW& view, SceneWidget* sw);

}
