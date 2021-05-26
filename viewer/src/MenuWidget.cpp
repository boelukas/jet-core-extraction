#pragma once
#include "MenuWidget.h"
#include "MainWindow.h"
#include "TimeGroupBox.h"
#include "ActorsGroupBox.h"
#include "OptionTabWidget.h"


MenuWidget::MenuWidget(MainWindow* parent):parent(parent){
	/*
		Creates the left widget in the splitter, the menu widget.
	*/
	setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
	menuWidgetLayout = new QVBoxLayout();
	timeGroupBox = new TimeGroupBox(this);
	actorsGroupBox = new ActorsGroupBox(this);
	optionTabWidget = new OptionTabWidget(this);
	menuWidgetLayout->addWidget(actorsGroupBox);
	menuWidgetLayout->addWidget(optionTabWidget);
	menuWidgetLayout->addWidget(timeGroupBox);
	menuWidgetLayout->addStretch(1);
	setLayout(menuWidgetLayout);
}
void MenuWidget::update() {
	optionTabWidget->update();
	actorsGroupBox->update();
	timeGroupBox->update();
}