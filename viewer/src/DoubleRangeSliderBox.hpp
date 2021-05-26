#pragma once

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QDoubleSpinBox;
class QDoubleRangeSlider;
QT_END_NAMESPACE

class DoubleRangeSliderBox : public QGroupBox
{
    Q_OBJECT

public:
    DoubleRangeSliderBox();
    DoubleRangeSliderBox(const QString& title, double minValue,
                    double maxValue, QWidget* parent = nullptr);
    double GetMinValue();
    double GetMaxValue();

signals:
    void minValueChanged(double newMinValue);
    void maxValueChanged(double newMaxValue);
    void filterValuesChanged(double newMinValue, double newMaxValue);

public slots:
    void setValues(double newMinValue, double newMaxValue);
    void setMinimumValue(double newMinValue);
    void setMaximumValue(double newMaxValue);
    void setLowerBound(double newLowerBound);
    void setUpperBound(double newUpperBound);
    void setBoxTitle(const QString& title);
    void setStepSize(double stepSize);

	void setLabelText(std::string text);

private:
    QGridLayout* gridLayout;
    QLabel* label;
    QDoubleRangeSlider* slider;
    QDoubleSpinBox* spinBoxMin, *spinBoxMax;
};
