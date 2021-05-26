#include <fstream>

#include "Amira.hpp"

#pragma warning( push )
#pragma warning( disable: 4996 )

// ===================================================================================
const char* Amira::FindAndJump(const char* buffer, const char* SearchString)
{
	const char* FoundLoc = strstr(buffer, SearchString);
	if (FoundLoc) return FoundLoc + strlen(SearchString);
	return buffer;
}

// ===================================================================================
template <typename T>
static T ToNumber(const std::string& Text) {
	std::stringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}

// ===============================================================================================
bool Amira::IsLineGeometry(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int numLines(0);
	if (sscanf(FindAndJump(buffer, "define Lines"), "%d", &numLines) == 1)
	{
		fclose(fp);
		return true;
	}
	fclose(fp);
	return false;
}

// ===============================================================================================
bool Amira::IsSteadyScalarField2d(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (!bIsUniform) return false;

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);

	return NumComponents == 1 && zDim == 1;
}

// ===============================================================================================
bool Amira::IsSteadyScalarField3d(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (!bIsUniform) return false;

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	return NumComponents == 1;
}

// ===============================================================================================
bool Amira::IsSteadyVectorField3d(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (!bIsUniform) return false;

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	return NumComponents == 3;
}

bool Amira::IsUnsteadyScalarField2d(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (!bIsUniform) return false;

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	return NumComponents == 1;
}

bool Amira::IsUnsteadyVectorField2d(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return false;
	}

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (!bIsUniform) return false;

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	return NumComponents == 2;
}



// ===============================================================================================


void Amira::Export(const char* path, const std::vector<float> vector)
{
	// Get basic information about vector
	int size = vector.size();
	float minCorner = *std::min_element(vector.begin(), vector.end());
	float maxCorner = *std::max_element(vector.begin(), vector.end());

	float* fltData = (float*)(&vector[0]);

	// Write header
	{
		std::ofstream outStream(path);
		outStream << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1" << std::endl;
		outStream << std::endl;
		outStream << std::endl;

		outStream << "define Lattice " << size << " " << " 1 " << " 1 " << std::endl;
		outStream << std::endl;

		outStream << "Parameters {" << std::endl;
		outStream << "Content \"" << size << "x" << 1 << "x" << 1 << " float, uniform coordinates\"," << std::endl;

		outStream << "\tBoundingBox " << minCorner << " " << maxCorner << " " << "0" << " " << "0" << " " << "0" << " " << "0" << "," << std::endl;
		outStream << "\tCoordType \"uniform\"" << std::endl;
		outStream << "}" << std::endl;
		outStream << std::endl;

		outStream << "Lattice { float Data } @1" << std::endl;
		outStream << std::endl;

		outStream << "# Data section follows" << std::endl;
		outStream << "@1" << std::endl;

		outStream.close();
	}

	// Write data
	{
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * size);
		outStream.close();
	}
}
// ===============================================================================================
void Amira::Export(const char* path, const RegScalarField2f* scalarField)
{
	// Get basic information about field
	Vec2i resolution = scalarField->GetResolution();
	int numElements = resolution[0] * resolution[1];
	Vec2f minCorner = static_cast<Vec2f>(scalarField->GetDomain().GetMin());
	Vec2f maxCorner = static_cast<Vec2f>(scalarField->GetDomain().GetMax());

	// Copy data to float array if necessary
	float* fltData = NULL;
	fltData = (float*)(&scalarField->GetData()[0]);


	// Write header
	{
		std::ofstream outStream(path);
		outStream << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1" << std::endl;
		outStream << std::endl;
		outStream << std::endl;

		outStream << "define Lattice " << resolution[0] << " " << resolution[1] << " 1 " << std::endl;
		outStream << std::endl;

		outStream << "Parameters {" << std::endl;
		outStream << "Content \"" << resolution[0] << "x" << resolution[1] << "x" << 1 << " float, uniform coordinates\"," << std::endl;

		outStream << "\tBoundingBox " << minCorner[0] << " " << maxCorner[0] << " " << minCorner[1] << " " << maxCorner[1] << " " << "0" << " " << "0" << "," << std::endl;
		outStream << "\tCoordType \"uniform\"" << std::endl;
		outStream << "}" << std::endl;
		outStream << std::endl;

		outStream << "Lattice { float Data } @1" << std::endl;
		//outStream << "Lattice { float verticalDist } @2" << std::endl;

		outStream << std::endl;

		outStream << "# Data section follows" << std::endl;
		outStream << "@1" << std::endl;

		outStream.close();
	}

	// Write data
	{
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * numElements);
		//outStream.write((char*)fltData, sizeof(float) * numElements); Kann auch merken wie oft geschrieben wurde.

		outStream.close();

	}
}

// ===============================================================================================
bool Amira::IsUnsteadyVectorField3d(const char* path)
{
	std::string s(path);
	return s.size() > 10 &&
		s[s.size() - 10] == 'f' &&
		s[s.size() - 9] == 'i' &&
		s[s.size() - 8] == 'l' &&
		s[s.size() - 7] == 'e' &&
		s[s.size() - 6] == 's' &&
		s[s.size() - 5] == 'e' &&
		s[s.size() - 4] == 'r' &&
		s[s.size() - 3] == 'i' &&
		s[s.size() - 2] == 'e' &&
		s[s.size() - 1] == 's';
}

void Amira::Export(const char* path, const RegVectorField2f* vectorField)
{
	// Get basic information about field
	Vec2i resolution = vectorField->GetResolution();
	int numElements = resolution[0] * resolution[1];
	Vec2f minCorner = static_cast<Vec2f>(vectorField->GetDomain().GetMin());
	Vec2f maxCorner = static_cast<Vec2f>(vectorField->GetDomain().GetMax());

	// Copy data to float array
	float* fltData = NULL;
	fltData = (float*)(&vectorField->GetData()[0]);


	// Write header
	{
		std::ofstream outStream(path);
		outStream << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1" << std::endl;
		outStream << std::endl;
		outStream << std::endl;

		outStream << "define Lattice " << resolution[0] << " " << resolution[1] << " 1" << std::endl;
		outStream << std::endl;

		outStream << "Parameters {" << std::endl;
		outStream << "Content \"" << resolution[0] << "x" << resolution[1] << "x1" << " float[2], uniform coordinates\"," << std::endl;

		outStream << "\tBoundingBox " << minCorner[0] << " " << maxCorner[0] << " " << minCorner[1] << " " << maxCorner[1] << " -1 1," << std::endl;
		outStream << "\tCoordType \"uniform\"" << std::endl;
		outStream << "}" << std::endl;
		outStream << std::endl;

		outStream << "Lattice { float[2] Data } @1" << std::endl;
		outStream << std::endl;

		outStream << "# Data section follows" << std::endl;
		outStream << "@1" << std::endl;

		outStream.close();
	}

	// Write data
	{
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * numElements * 2);
		outStream.close();
	}


}

// exports a steady 3D scalar field as 3D grid with float attributes.
void Amira::Export(const char* path, const RegScalarField3f* scalarField)
{
	// Get basic information about field
	Vec3i resolution = scalarField->GetResolution();
	int numElements = resolution[0] * resolution[1] * resolution[2];
	Vec3f minCorner = static_cast<Vec3f>(scalarField->GetDomain().GetMin());
	Vec3f maxCorner = static_cast<Vec3f>(scalarField->GetDomain().GetMax());

	// Copy data to float array if necessary
	float* fltData = NULL;
	fltData = (float*)(&scalarField->GetData()[0]);


	// Write header
	{
		std::ofstream outStream(path);
		outStream << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1" << std::endl;
		outStream << std::endl;
		outStream << std::endl;

		outStream << "define Lattice " << resolution[0] << " " << resolution[1] << " " << resolution[2] << std::endl;
		outStream << std::endl;

		outStream << "Parameters {" << std::endl;
		outStream << "Content \"" << resolution[0] << "x" << resolution[1] << "x" << resolution[2] << " float, uniform coordinates\"," << std::endl;

		outStream << "\tBoundingBox " << minCorner[0] << " " << maxCorner[0] << " " << minCorner[1] << " " << maxCorner[1] << " " << minCorner[2] << " " << maxCorner[2] << "," << std::endl;
		outStream << "\tCoordType \"uniform\"" << std::endl;
		outStream << "}" << std::endl;
		outStream << std::endl;

		outStream << "Lattice { float Data } @1" << std::endl;
		outStream << std::endl;

		outStream << "# Data section follows" << std::endl;
		outStream << "@1" << std::endl;

		outStream.close();
	}

	// Write data
	{
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * numElements);
		outStream.close();
	}

}


void Amira::Export(const char* path, const RegVectorField3f* vectorField)
{
	// Get basic information about field
	Vec3i resolution = vectorField->GetResolution();
	int numElements = resolution[0] * resolution[1] * resolution[2];
	Vec3f minCorner = static_cast<Vec3f>(vectorField->GetDomain().GetMin());
	Vec3f maxCorner = static_cast<Vec3f>(vectorField->GetDomain().GetMax());

	// Copy data to float array
	float* fltData = NULL;
	fltData = (float*)(&vectorField->GetData()[0]);


	// Write header
	{
		std::ofstream outStream(path);
		outStream << "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1" << std::endl;
		outStream << std::endl;
		outStream << std::endl;

		outStream << "define Lattice " << resolution[0] << " " << resolution[1] << " " << resolution[2] << std::endl;
		outStream << std::endl;

		outStream << "Parameters {" << std::endl;
		outStream << "Content \"" << resolution[0] << "x" << resolution[1] << "x" << resolution[2] << " float[3], uniform coordinates\"," << std::endl;

		outStream << "\tBoundingBox " << minCorner[0] << " " << maxCorner[0] << " " << minCorner[1] << " " << maxCorner[1] << " " << minCorner[2] << " " << maxCorner[2] << "," << std::endl;
		outStream << "\tCoordType \"uniform\"" << std::endl;
		outStream << "}" << std::endl;
		outStream << std::endl;

		outStream << "Lattice { float[3] Data } @1" << std::endl;
		outStream << std::endl;

		outStream << "# Data section follows" << std::endl;
		outStream << "@1" << std::endl;

		outStream.close();
	}

	// Write data
	{
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * numElements * 3);
		outStream.close();
	}

}
std::vector<float> Amira::ImportStdVectorf(const char* path) {
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return std::vector<float>();
	}
	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return std::vector<float>();
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return std::vector<float>();
	}

	if (NumComponents != 1)
	{
		printf("AmiraLoader: Not a scalar field!\n");
		fclose(fp);
		return std::vector<float>();
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim /** yDim * zDim*/ * NumComponents;

		std::vector<float> vector(NumToRead);

		float* fltData = (float*)(&vector[0]);
		const size_t ActRead = fread((void*)fltData, sizeof(float), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
			fclose(fp);
			return std::vector<float>();
		}


		fclose(fp);
		return vector;
	}

	fclose(fp);
	return std::vector<float>();
}


// ===============================================================================================
RegScalarField2f* Amira::ImportScalarField2f(const char* path)
{
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return NULL;
	}

	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return NULL;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return NULL;
	}

	if (NumComponents != 1)
	{
		printf("AmiraLoader: Not a scalar field!\n");
		fclose(fp);
		return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim * yDim /** zDim*/ * NumComponents;

		RegScalarField2f* field = new RegScalarField2f(
			Vec2i({ xDim, yDim }),
			BoundingBox2d(Vec2d({ xmin, ymin }), Vec2d({ xmax, ymax })));



		float* fltData = (float*)(&field->GetData()[0]);
		const size_t ActRead = fread((void*)fltData, sizeof(float), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
			fclose(fp);
			delete field;
			return NULL;
		}


		fclose(fp);
		return field;
	}

	fclose(fp);
	return NULL;
}

// ===============================================================================================
RegVectorField2f* Amira::ImportVectorField2f(const char* path)
{
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return NULL;
	}

	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return NULL;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	zDim = 1;

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return NULL;
	}

	if (NumComponents != 2)
	{
		printf("AmiraLoader: Not a 3D vector field!\n");
		fclose(fp);
		return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim * yDim;

		RegVectorField2f* field = new RegVectorField2f(
			Vec2i({ xDim, yDim }),
			BoundingBox2d(Vec2d({ xmin, ymin }), Vec2d({ xmax, ymax })));


		if (NumComponents == 2)
		{
			Vec2f* fltData = new Vec2f[xDim * yDim];
			const size_t ActRead = fread((void*)fltData, sizeof(Vec2f), NumToRead, fp);
			if (NumToRead != ActRead)
			{
				printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
				fclose(fp);
				delete field;
				return NULL;
			}
			for (int i = 0; i < xDim * yDim; ++i) {
				Vec2i coord = field->GetGridCoord(i);
				field->SetVertexDataAt(coord, fltData[i]);
			}

			delete[] fltData;
		}


		fclose(fp);
		return field;
	}

	fclose(fp);
	return NULL;
}

RegScalarField3f* Amira::ImportScalarField3f(const char* path)
{
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return NULL;
	}

	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return NULL;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else if (strstr(buffer, "Lattice { double Data }"))
	{
		//Scalar field
		isFloat = false;
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return NULL;
	}

	if (NumComponents != 1)
	{
		printf("AmiraLoader: Not a scalar field!\n");
		fclose(fp);
		return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim * yDim * zDim * NumComponents;

		RegScalarField3f* field = new RegScalarField3f(
			Vec3i({ xDim, yDim, zDim }),
			BoundingBox3d(Vec3d({ xmin, ymin, zmin }), Vec3d({ xmax, ymax, zmax })));



		float* fltData = (float*)(&field->GetData()[0]);
		const size_t ActRead = fread((void*)fltData, sizeof(float), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
			fclose(fp);
			delete field;
			return NULL;
		}

		fclose(fp);
		return field;
	}

	fclose(fp);
	return NULL;
}


RegVectorField3f* Amira::ImportVectorField3f(const char* path)
{
	const bool verbose = false;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		return NULL;
	}

	if (verbose) printf("AmiraLoader: Reading %s\n", path);

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		return NULL;
	}

	//Find the Lattice definition, i.e., the dimensions of the uniform grid
	int xDim(0), yDim(0), zDim(0);
	sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
	if (verbose) printf("AmiraLoader: Grid Dimensions: %i %i %i", xDim, yDim, zDim);

	//Find the BoundingBox
	float xmin(1.0f), ymin(1.0f), zmin(1.0f);
	float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
	sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (verbose) printf("AmiraLoader: BoundingBox in x-Direction: [%f ... %f]\n", xmin, xmax);
	if (verbose) printf("AmiraLoader: BoundingBox in y-Direction: [%f ... %f]\n", ymin, ymax);
	if (verbose) printf("AmiraLoader: BoundingBox in z-Direction: [%f ... %f]\n", zmin, zmax);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
	if (verbose)
	{
		if (bIsUniform)
			printf("AmiraLoader: GridType: uniform");
		else printf("AmiraLoader: GridType: UNKNOWN");
	}

	//Type of the field: scalar, vector
	int NumComponents(0);
	bool isFloat = true;
	if (strstr(buffer, "Lattice { float Data }"))
	{
		//Scalar field
		NumComponents = 1;
	}
	else
	{
		//A field with more than one component, i.e., a vector field
		if (sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents) == 1) isFloat = true;
		else if (sscanf(FindAndJump(buffer, "Lattice { double["), "%d", &NumComponents) == 1) isFloat = false;
	}
	if (verbose) printf("AmiraLoader: Number of Components: %i\n", NumComponents);

	//Sanity check
	if (xDim <= 0 || yDim <= 0 || zDim <= 0
		|| xmin > xmax || ymin > ymax || zmin > zmax
		|| !bIsUniform || NumComponents <= 0)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		return NULL;
	}

	if (NumComponents != 3)
	{
		printf("AmiraLoader: Not a 3D vector field!\n");
		fclose(fp);
		return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		// - how much to read
		const size_t NumToRead = xDim * yDim * zDim;

		RegVectorField3f* field = new RegVectorField3f(
			Vec3i({ xDim, yDim, zDim }),
			BoundingBox3d(Vec3d({ xmin, ymin, zmin }), Vec3d({ xmax, ymax, zmax })));


		if (NumComponents == 3)
		{
			Vec3f* fltData = new Vec3f[xDim * yDim * zDim];
			const size_t ActRead = fread((void*)fltData, sizeof(Vec3f), NumToRead, fp);
			if (NumToRead != ActRead)
			{
				printf("AmiraLoader: Something went wrong while reading the binary data section.\nPremature end of file?");
				fclose(fp);
				delete field;
				return NULL;
			}
			for (int i = 0; i < xDim * yDim * zDim; ++i) {
				Vec3i coord = field->GetGridCoord(i);
				field->SetVertexDataAt(coord, fltData[i]);
			}
			delete[] fltData;
		}

		fclose(fp);
		return field;
	}

	fclose(fp);
	return NULL;
}

void Amira::ImportLineGeometry(const char* path, LineCollection& output) {
	//LineGeometry3f* result = NULL;
	std::vector<int> rawIndices;
	std::vector<Vec3f> rawVertices;

	FILE* fp = fopen(path, "rb");
	if (!fp)
	{
		printf(("AmiraLoader: Could not find " + std::string(path) + "\n").c_str());
		//return NULL;
	}

	//We read the first 2k bytes into memory to parse the header.
	//The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
	char buffer[2048];
	fread(buffer, sizeof(char), 2047, fp);
	buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

	if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
	{
		printf("AmiraLoader: Not a proper amira file.");
		fclose(fp);
		//return NULL;
	}

	int numLines(0), numVertices(0);
	sscanf(FindAndJump(buffer, "define Lines"), "%d", &numLines);
	sscanf(FindAndJump(buffer, "nVertices"), "%d", &numVertices);
	rawIndices.resize(numLines);
	rawVertices.resize(numVertices);

	//Is it a uniform grid? We need this only for the sanity check below.
	const bool bIsHxLineSet = (strstr(buffer, "ContentType \"HxLineSet\"") != NULL);

	//Type of the field: scalar, vector
	int NumComponents(0);
	sscanf(FindAndJump(buffer, "Vertices { float["), "%d", &NumComponents);

	//Sanity check
	if (!bIsHxLineSet)
	{
		printf("AmiraLoader: Something went wrong");
		fclose(fp);
		//return NULL;
	}

	//Find the beginning of the data section
	const long idxStartData = (long)(strstr(buffer, "# Data section follows") - buffer);
	if (idxStartData > 0)
	{
		//Set the file pointer to the beginning of "# Data section follows"
		fseek(fp, idxStartData, SEEK_SET);
		//Consume this line, which is "# Data section follows"
		fgets(buffer, 2047, fp);
		//Consume the next line, which is "@1"
		fgets(buffer, 2047, fp);

		//Read the data
		size_t NumToRead = (size_t)numLines;
		size_t ActRead = fread((void*)rawIndices.data(), sizeof(int), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong");
			fclose(fp);
			//return NULL;
		}

		//Consume the next line, which is "@2"
		fgets(buffer, 2047, fp);
		fgets(buffer, 2047, fp);

		NumToRead = (size_t)numVertices;
		ActRead = fread((void*)rawVertices.data(), sizeof(Vec3f), NumToRead, fp);
		if (NumToRead != ActRead)
		{
			printf("AmiraLoader: Something went wrong");
			fclose(fp);
			//return NULL;
		}
	}
	fclose(fp);
	std::vector<std::vector<Vec3d>> lines;
	int index = 0;
	int total_nr_lines = numLines - numVertices;

	lines.resize(total_nr_lines);
	for (int i = 0; i < total_nr_lines; i++) {
		lines[i] = std::vector<Vec3d>();
		while (rawIndices[index] != -1) {
			Vec3d p = rawVertices[rawIndices[index]];
			p[0] = p[0] * 2 + 360;
			p[1] = p[1] * 2 + 180;
			p[2] = 103 - p[2];
			lines[i].push_back(p);
			index++;
		}
		index++;
	}
	//result = new LineGeometry3f();
	//Vec3fArray* pos = &result->CreateLine()->GetVertices();
	output.setData(lines);

	//result->RecomputeBoundingBox();
	//return result;
}
#pragma warning( pop )
