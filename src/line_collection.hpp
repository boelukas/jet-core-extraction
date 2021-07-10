#pragma once
#include "math.hpp"

class LineCollection {
	/*
	Handles a collection of lines of different length with vertex attributes.
	Format if lines_ array:
	[size_of_Lines_array|nr_of_attributes|attribute 0 size, ..., attribute n size| nr_of_lines | nr_of_vertexes_of_line_0, ... ,  nr_of_vertexes_of_line_n | vertexes_of_line_0, ..., vertexes_of_line_n|attribute_0,...,attribute_n]
	*/
public:
	struct Attribute {
		std::vector<float> data;
		std::string name;
	};
	struct Point {
		Vec3d coord;
		std::vector<float> attributes;
	};
	LineCollection();

	void SetData(const std::vector<std::vector<Vec3d>>& lines);
	void SetData(const std::vector<float>& lines);

	const size_t& GetNumberOfLines()const;
	size_t GetTotalNumberOfPoints() const;
	size_t GetNumberOfPointsOfLine(const size_t& line_nr) const;
	std::vector<std::vector<Vec3d>> GetLinesInVectorOfVector() const;
	std::vector<Vec3d> GetAllPointsInVector() const;
	const std::vector<float>& GetAttributeByName(const std::string& attribute_name) const;

	void ExportTxtFile(const char* path, const std::vector<float>& ps_axis_values) const;
	void ExportVtp(const char* path, const std::vector<float>& ps_axis_values) const;

	void Clear();

private:
	std::vector<float> lines_;
	std::vector<Attribute*> attributes_;

	size_t n_lines_;
};
