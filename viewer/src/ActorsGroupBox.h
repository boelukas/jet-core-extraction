#pragma once
#include <QtWidgets>

class MenuWidget;
class MainWindow;
class ActorsGroupBox : public QGroupBox {
	Q_OBJECT
public:
	ActorsGroupBox(MenuWidget* parent);
	void update();
	MenuWidget* parent;
	MainWindow* mainWindow;

	QVBoxLayout* actorsBoxLayout;
	QCheckBox* jetCheckBox;
	QCheckBox* jetSplitMergeCheckBox;
	QCheckBox* jetSeedsCheckBox;
	QCheckBox* tropoCheckBox;
	QCheckBox* wcbCheckBox;
	QCheckBox* volumeCheckBox;
	QCheckBox* sliceCheckBox;
	QCheckBox* isoCheckBox;

};
