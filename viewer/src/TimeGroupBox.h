#pragma once
#include <QtWidgets>

class MenuWidget;
class MainWindow;

class TimeGroupBox : public QGroupBox {
	Q_OBJECT
public:
	TimeGroupBox(MenuWidget* parent);
	void update();
	MenuWidget* parent;
	MainWindow* mainWindow;
	QGridLayout* timeBoxLayout;
	QSlider* timeSlider;
	QDateTimeEdit* timeEdit;
	QDateTimeEdit* screenshotEndtimeEdit;
	QPushButton* takeScreenshotButton;
	QLineEdit* screenShotLabelEdit;
};
