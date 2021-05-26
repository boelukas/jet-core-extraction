#pragma once
#include <QtWidgets>

class MainWindow;
class TimeGroupBox;
class ActorsGroupBox;
class OptionTabWidget;

class MenuWidget : public QWidget {
	Q_OBJECT
public:
	MenuWidget(MainWindow* parent);
	void update();
	MainWindow* parent;
	QVBoxLayout* menuWidgetLayout;
	TimeGroupBox* timeGroupBox;
	ActorsGroupBox* actorsGroupBox;
	OptionTabWidget* optionTabWidget;
};
