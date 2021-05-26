#pragma once
#include "core/TimeHelper.hpp"
#include "core/DataHelper.hpp"

#include "MainWindow.h"

#include "TimeGroupBox.h"

TimeGroupBox::TimeGroupBox(MenuWidget* _parent) :QGroupBox(tr("Time")), parent(_parent), mainWindow(parent->parent) {
	MainWindow::GuiState* state = mainWindow->getState();
	timeBoxLayout = new QGridLayout();

	timeSlider = new QSlider();
	timeSlider->setRange(0, DataHelper::collectTimes().size() - 1);
	timeSlider->setSliderPosition(state->general.time);
	timeSlider->setOrientation(Qt::Horizontal);
	connect(timeSlider, &QSlider::valueChanged, parent->parent, &::MainWindow::changeTime);
	//connect(const QObject * sender, const char* signal, const QObject * receiver, const char* method, Qt::ConnectionType type = Qt::AutoConnection
	timeBoxLayout->addWidget(timeSlider, 1, 0);

	timeEdit = new QDateTimeEdit();
	std::string time_s = TimeHelper::convertHoursToDate(state->general.time, DataHelper::getDataStartDate()).c_str();
	QDateTime timeEditTime = QDateTime::fromString(TimeHelper::convertHoursToDate(state->general.time, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	timeEdit->setDateTime(timeEditTime);
	timeEdit->setDisplayFormat("yyyy.mm.dd_hh");
	QDateTime minTime = QDateTime::fromString(TimeHelper::convertHoursToDate(0, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	QDateTime maxTime = QDateTime::fromString(TimeHelper::convertHoursToDate(DataHelper::collectTimes().size() - 1, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	timeEdit->setDateTimeRange(minTime, maxTime);
	connect(timeEdit, &QDateTimeEdit::dateTimeChanged, mainWindow, &::MainWindow::changeQTime);
	timeBoxLayout->addWidget(timeEdit, 1, 1);

	QLabel* screenshotEndtimeLabel = new QLabel("Screenshot end time:");
	timeBoxLayout->addWidget(screenshotEndtimeLabel, 2, 0);
	screenshotEndtimeEdit = new QDateTimeEdit();
	QDateTime screenshotEndtime = QDateTime::fromString(TimeHelper::convertHoursToDate(state->general.screenshot_end_time, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	screenshotEndtimeEdit->setDisplayFormat("yyyy.mm.dd_hh");
	QDateTime minScTime = QDateTime::fromString(TimeHelper::convertHoursToDate(state->general.time, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	QDateTime maxScTime = QDateTime::fromString(TimeHelper::convertHoursToDate(DataHelper::collectTimes().size() - 1, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	screenshotEndtimeEdit->setDateTimeRange(minScTime, maxScTime);
	connect(screenshotEndtimeEdit, &QDateTimeEdit::dateTimeChanged, mainWindow, &::MainWindow::changeScreenshotEndTime);
	timeBoxLayout->addWidget(screenshotEndtimeEdit, 2, 1);

	takeScreenshotButton = new QPushButton("Take Screenshots");
	connect(takeScreenshotButton, &QPushButton::clicked, mainWindow, &::MainWindow::takeScreenshots);
	timeBoxLayout->addWidget(takeScreenshotButton, 3, 0);

	screenShotLabelEdit = new QLineEdit();
	screenShotLabelEdit->setText(QString::fromStdString(state->general.screenshot_label));
	connect(screenShotLabelEdit, &QLineEdit::textChanged, mainWindow, &::MainWindow::changeScreenShotLabel);
	timeBoxLayout->addWidget(screenShotLabelEdit, 3, 1);

	setLayout(timeBoxLayout);
}
void TimeGroupBox::update() {
	MainWindow::GuiState* state = mainWindow->getState();
	timeEdit->blockSignals(true);
	timeSlider->blockSignals(true);
	QDateTime timeEditTime = QDateTime::fromString(TimeHelper::convertHoursToDate(state->general.time, DataHelper::getDataStartDate()).c_str(), "yyyymmdd_hh");
	timeEdit->setDateTime(timeEditTime);
	timeSlider->setSliderPosition(state->general.time);
	timeEdit->blockSignals(false);
	timeSlider->blockSignals(false);
}
