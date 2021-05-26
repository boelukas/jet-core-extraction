#pragma once
#include "ISceneWidget.h"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <string>


class PC_TestWidget : public ISceneWidget
{
public:
	PC_TestWidget(std::string _dataName, int time, DataLoader_Amira *op);

private:


	double level;
};