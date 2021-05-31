#pragma once
#include "RegularGrid.hpp"
#include "LineCollection.hpp"

class Amira
{
public:
	static const char* FindAndJump(const char* buffer, const char* SearchString);
	static bool IsLineGeometry(const char* path);
	static bool IsSteadyScalarField2d(const char* path);
	static bool IsSteadyScalarField3d(const char* path);
	static bool IsSteadyVectorField3d(const char* path);
	static bool IsUnsteadyScalarField2d(const char* path);
	static bool IsUnsteadyVectorField2d(const char* path);
	static bool IsUnsteadyVectorField3d(const char* path);

	// exports a 2D scalar field as 2D grid with float attributes.
	static void Export(const char* path, const RegScalarField2f* scalarField);

	// exports a 2D vector field as 2D grid with float[2] attributes
	static void Export(const char* path, const RegVectorField2f* vectorField);

	// exports a 3D scalar field as 3D grid with float attributes.
	static void Export(const char* path, const RegScalarField3f* scalarField);

	// exports a 3D vector field as 3D grid with float[3] attributes
	static void Export(const char* path, const RegVectorField3f* vectorField);

	// exports a  vector with float attributes
	static void Export(const char* path, const std::vector<float> vector);


	// imports a 2d scalar field from an amira file
	static RegScalarField2f* ImportScalarField2f(const char* path);

	// imports a 2d vector field from an amira file
	static RegVectorField2f* ImportVectorField2f(const char* path);

	// imports a 3d scalar field from an amira file
	static RegScalarField3f* ImportScalarField3f(const char* path);

	// imports a 3d velocity field from an amira file
	static RegVectorField3f* ImportVectorField3f(const char* path);

	// imports a 1d vector from an amira file
	static std::vector<float> ImportStdVectorf(const char* path);

	// impors a line collection with all attributes.
	//static LineCollection ImportLineCollection(const char* path);

	static void ImportLineGeometry(const char* path, LineCollection& output);

};
