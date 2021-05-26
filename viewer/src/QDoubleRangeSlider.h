#pragma once

#include "ctkrangeslider.h"

class QDoubleRangeSlider : public ctkRangeSlider {
	Q_OBJECT

public:
	QDoubleRangeSlider(Qt::Orientation orientation = Qt::Orientation::Horizontal,
		QWidget* parent = 0) : ctkRangeSlider(orientation, parent) {
		connect(this, &ctkRangeSlider::minimumValueChanged,
			this, &QDoubleRangeSlider::notifyMinValueChanged);
		connect(this, &ctkRangeSlider::maximumValueChanged,
			this, &QDoubleRangeSlider::notifyMaxValueChanged);
		connect(this, &ctkRangeSlider::filterValuesChanged,
			this, &QDoubleRangeSlider::notifyValuesChanged);
		scaleFactor = 100000;
	}

	double GetMinDoubleValue() { return minimumValue() / (double)intScaleFactor(); }
	double GetMaxDoubleValue() { return maximumValue() / (double)intScaleFactor(); }
	int intScaleFactor() { return scaleFactor; }

	void setMinDoubleValue(double v) {
		int newvalue = v * (double)intScaleFactor();
		if (newvalue == 0)
			return;
		int curvalue = minimumValue();
		if (newvalue != curvalue)
			setMinimumValue(newvalue);
	}
	void setMaxDoubleValue(double v) {
		int newvalue = v * (double)intScaleFactor();
		if (newvalue == 0)
			return;
		int curvalue = maximumValue();
		if (newvalue != curvalue)
			setMaximumValue(newvalue);
	}

Q_SIGNALS:
	void doubleMinValueChanged(double newMinValue);
	void doubleMaxValueChanged(double newMaxValue);
	void doubleValuesChanged(double newMinValue, double newMaxValue);

public Q_SLOTS:
	void notifyMinValueChanged(int newMinVale) {
		double doubleMinValue = newMinVale / (double)intScaleFactor();
		emit doubleMinValueChanged(doubleMinValue);
	}
	void notifyMaxValueChanged(int newMaxVale) {
		double doubleMaxValue = newMaxVale / (double)intScaleFactor();
		emit doubleMaxValueChanged(doubleMaxValue);
	}
	void notifyValuesChanged(int newMinVale, int newMaxVale) {
		double doubleMinValue = newMinVale / (double)intScaleFactor();
		double doubleMaxValue = newMaxVale / (double)intScaleFactor();

		emit doubleValuesChanged(doubleMinValue, doubleMaxValue);
	}


private:
	int scaleFactor;

};
