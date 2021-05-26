#pragma once
#include "ISceneWidget.h"

class ParticleWidget : public ISceneWidget
{
public:
	ParticleWidget(std::string dataName, int time, DataLoader_Amira* op);

private:
	void CreateTestScene();
	vtkSmartPointer<vtkActor> createLineActor(const std::vector<Vec3f>& line, const Vec3f& color);

};