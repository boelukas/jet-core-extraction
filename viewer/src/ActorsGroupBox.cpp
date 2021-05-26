#pragma once
#include "MainWindow.h"

#include "ActorsGroupBox.h"

ActorsGroupBox::ActorsGroupBox(MenuWidget* _parent) :QGroupBox(tr("Scene Elements")), parent(_parent), mainWindow(parent->parent) {
	MainWindow::GuiState* state = mainWindow->getState();
	//Scene elements box
	actorsBoxLayout = new QVBoxLayout;

	jetCheckBox = new QCheckBox("Jet stream");
	int jet_state = state->jet.active ? Qt::Checked : Qt::Unchecked;
	jetCheckBox->setCheckState(Qt::CheckState(jet_state));
	connect(jetCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeJetActive);

	jetSplitMergeCheckBox = new QCheckBox("Split and Merge Points");
	int split_state = state->jet.split_merge_points_active ? Qt::Checked : Qt::Unchecked;
	jetSplitMergeCheckBox->setCheckState(Qt::CheckState(split_state));
	connect(jetSplitMergeCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeJetSplitMergeActive);

	jetSeedsCheckBox = new QCheckBox("Jet Seeds");
	int seed_state = state->jet.seed_points_active ? Qt::Checked : Qt::Unchecked;
	jetSeedsCheckBox->setCheckState(Qt::CheckState(seed_state));
	jetSeedsCheckBox->setVisible(!state->jet.use_preprocessed_jets);
	connect(jetSeedsCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeJetSeedsActive);

	tropoCheckBox = new QCheckBox("Tropopause");
	int tropo_state = state->tropo.active ? Qt::Checked : Qt::Unchecked;
	tropoCheckBox->setCheckState(Qt::CheckState(tropo_state));
	connect(tropoCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeTropoActive);

	wcbCheckBox = new QCheckBox("WCB");
	int wcb_state = state->wcb.active ? Qt::Checked : Qt::Unchecked;
	wcbCheckBox->setCheckState(Qt::CheckState(wcb_state));
	connect(wcbCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeWcbActive);

	volumeCheckBox = new QCheckBox("Volume");
	int volume_state = state->volume.active ? Qt::Checked : Qt::Unchecked;
	volumeCheckBox->setCheckState(Qt::CheckState(volume_state));
	connect(volumeCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeVolumeActive);

	sliceCheckBox = new QCheckBox("Slice");
	int slice_state = state->slice.active ? Qt::Checked : Qt::Unchecked;
	sliceCheckBox->setCheckState(Qt::CheckState(slice_state));
	connect(sliceCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeSliceActive);

	isoCheckBox = new QCheckBox("Iso surface");
	int iso_state = state->iso.active ? Qt::Checked : Qt::Unchecked;
	isoCheckBox->setCheckState(Qt::CheckState(iso_state));
	connect(isoCheckBox, &QCheckBox::stateChanged, mainWindow, &MainWindow::changeIsoActive);

	actorsBoxLayout->addWidget(jetCheckBox);
	actorsBoxLayout->addWidget(jetSplitMergeCheckBox);
	actorsBoxLayout->addWidget(jetSeedsCheckBox);
	actorsBoxLayout->addWidget(tropoCheckBox);
	actorsBoxLayout->addWidget(wcbCheckBox);
	actorsBoxLayout->addWidget(volumeCheckBox);
	actorsBoxLayout->addWidget(sliceCheckBox);
	actorsBoxLayout->addWidget(isoCheckBox);

	setLayout(actorsBoxLayout);
}
void ActorsGroupBox::update() {
	MainWindow::GuiState* state = mainWindow->getState();
	volumeCheckBox->blockSignals(true);
	sliceCheckBox->blockSignals(true);
	isoCheckBox->blockSignals(true);
	jetSeedsCheckBox->blockSignals(true);
	jetSeedsCheckBox->setVisible(!state->jet.use_preprocessed_jets);
	jetSeedsCheckBox->setChecked(state->jet.seed_points_active);
	volumeCheckBox->setVisible(state->general.view == jet_vis::VIEW::CLASSIC);
	volumeCheckBox->setChecked(state->volume.active);
	sliceCheckBox->setVisible(state->general.view == jet_vis::VIEW::CLASSIC || state->general.view == jet_vis::VIEW::GLOBE);
	sliceCheckBox->setChecked(state->slice.active);
	isoCheckBox->setVisible(state->general.view == jet_vis::VIEW::CLASSIC || state->general.view == jet_vis::VIEW::GLOBE);
	isoCheckBox->setChecked(state->iso.active);
	jetSeedsCheckBox->blockSignals(false);
	volumeCheckBox->blockSignals(false);
	sliceCheckBox->blockSignals(false);
	isoCheckBox->blockSignals(false);
}