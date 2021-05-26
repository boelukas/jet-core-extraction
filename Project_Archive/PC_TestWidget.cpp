#include "Core/DataLoader_Amira.hpp"
#include "PC_TestWidget.h"
#include "Core/WindFields.hpp"

#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkRenderer.h>
#include <vtkDelaunay2D.h>
#include <vtkProperty.h>
#include <vtkHedgeHog.h>
#include <vtkInformation.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkGlyph3D.h>
#include <vtkArrowSource.h>
#include <vtkXMLImageDataWriter.h>

PC_TestWidget::PC_TestWidget(std::string _dataName, int _time, DataLoader_Amira *op)
:ISceneWidget(_dataName, _time, op)
{
}
