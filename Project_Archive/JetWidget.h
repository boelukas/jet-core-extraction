#pragma once
#include "ISceneWidget.h"

class JetWidget : public ISceneWidget
{
public:
	JetWidget(std::string dataName, int time, DataLoader_Amira* op);

private:
	void CreateTestScene();
	vtkSmartPointer<vtkActor> createLineActor(const std::vector<Vec3d>& line, const Vec3f& color);

};