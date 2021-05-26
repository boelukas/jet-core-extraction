/*
    Source code taken from Rafael Boduryan.
*/
#pragma once
#include "DoubleRangeSliderBox.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include "QDoubleRangeSlider.h"

DoubleRangeSliderBox::DoubleRangeSliderBox() : DoubleRangeSliderBox("Iso surface Value", 0, 1) {}

DoubleRangeSliderBox::DoubleRangeSliderBox(const QString& title, double minValue,
                                 double maxValue, QWidget* parent) : QGroupBox("", parent)
{
    this->gridLayout = new QGridLayout();
    this->label = new QLabel(title);
    this->spinBoxMin = new QDoubleSpinBox();
    this->spinBoxMax = new QDoubleSpinBox();
    this->slider = new QDoubleRangeSlider(Qt::Horizontal);
    int scaleFactor = this->slider->intScaleFactor();

    //this->slider->setTickPosition(QDoubleSlider::TicksBothSides);
    this->slider->setTickInterval(0.1 * scaleFactor);
    this->slider->setSingleStep(0.1 * scaleFactor);
    this->spinBoxMin->setDecimals(5);
    this->spinBoxMin->setSingleStep(static_cast<double>(1./scaleFactor));
    this->spinBoxMax->setDecimals(5);
    this->spinBoxMax->setSingleStep(static_cast<double>(1./scaleFactor));
    this->setLowerBound(minValue);
    this->setUpperBound(maxValue);
    this->setValues(minValue, maxValue);

    //Internal connections
    connect(this->slider, &QDoubleRangeSlider::doubleMinValueChanged,
            this->spinBoxMin, &QDoubleSpinBox::setValue);
    connect(this->spinBoxMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->slider, &QDoubleRangeSlider::setMinDoubleValue);

    connect(this->slider, &QDoubleRangeSlider::doubleMaxValueChanged,
            this->spinBoxMax, &QDoubleSpinBox::setValue);
    connect(this->spinBoxMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->slider, &QDoubleRangeSlider::setMaxDoubleValue);

    //Signal forwarding
    connect(this->slider, &QDoubleRangeSlider::doubleMinValueChanged,
            this, &DoubleRangeSliderBox::minValueChanged);
    connect(this->slider, &QDoubleRangeSlider::doubleMaxValueChanged,
            this, &DoubleRangeSliderBox::maxValueChanged);
    connect(this->slider, &QDoubleRangeSlider::doubleValuesChanged,
            this, &DoubleRangeSliderBox::filterValuesChanged);

    gridLayout->addWidget(slider, 0, 0, 1, 3);
    gridLayout->addWidget(label, 1, 0);
    gridLayout->addWidget(spinBoxMin, 1, 1);
    gridLayout->addWidget(spinBoxMax, 1, 2);

    //this->setBoxTitle(title);
    this->setLayout(gridLayout);
    this->setFlat(true);
    this->setStyleSheet("border:0;");
}

double DoubleRangeSliderBox::GetMinValue(){ return spinBoxMin->value(); }
double DoubleRangeSliderBox::GetMaxValue(){ return spinBoxMax->value(); }

void DoubleRangeSliderBox::setMinimumValue(double newMin){
    int scaleFactor = this->slider->intScaleFactor();
    this->slider->setMinimumPosition(newMin * scaleFactor);
    this->spinBoxMin->setValue(newMin);
}

void DoubleRangeSliderBox::setMaximumValue(double newMaxValue){
    int scaleFactor = this->slider->intScaleFactor();
    this->slider->setMaximumPosition(newMaxValue * scaleFactor);
    this->spinBoxMax->setValue(newMaxValue);
}

void DoubleRangeSliderBox::setValues(double newMinValue, double newMaxValue){
    this->setMinimumValue(newMinValue);
    this->setMaximumValue(newMaxValue);
}

void DoubleRangeSliderBox::setLowerBound(double newLowerBound)
{
    int scaleFactor = this->slider->intScaleFactor();
    this->slider->setMinimum(newLowerBound * scaleFactor);
    this->spinBoxMin->setMinimum(newLowerBound);
    this->spinBoxMax->setMinimum(newLowerBound);
}

void DoubleRangeSliderBox::setUpperBound(double newUpperBound)
{
    int scaleFactor = this->slider->intScaleFactor();
    this->slider->setMaximum(newUpperBound * scaleFactor);
    this->spinBoxMin->setMaximum(newUpperBound);
    this->spinBoxMax->setMaximum(newUpperBound);
}

void DoubleRangeSliderBox::setBoxTitle(const QString& title)
{
    this->setTitle(title);
}
void DoubleRangeSliderBox::setStepSize(double stepSize) {
    this->spinBoxMax->setSingleStep(stepSize);
    this->spinBoxMin->setSingleStep(stepSize);
}
void DoubleRangeSliderBox::setLabelText(std::string text) {
    this->label->setText(text.c_str());
}