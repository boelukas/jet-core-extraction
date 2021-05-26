#pragma once
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>

class VisualizationDataLoader;
class SceneWidget : public QVTKOpenGLNativeWidget
{
public:
	SceneWidget(VisualizationDataLoader* _op);
	void RenderBackground(bool axes_active);
	void removeAxes();
	void addAxes();
private:
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> mRenderWindow;
	vtkSmartPointer<vtkCubeAxesActor> axes;
	VisualizationDataLoader* op;
};
