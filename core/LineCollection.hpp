#pragma once
#include "Math.hpp"

class LineCollection {
	/*
	This class handles lines of different lenght with attributes.
	Layout in Memory:
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

	//Setters
	void setData(const std::vector<std::vector<Vec3d>>& lines);
	void setData(const std::vector<float>& lines);

	//Getters
	const std::vector<float>& getLinesAsVector() const;
	const size_t& getNumberOfLines()const;
	size_t getTotalNumberOfPoints() const;
	size_t getIndexInAttributeArray(const size_t& line_nr, const size_t& point_nr) const;
	Vec3d getPoint(const size_t& index) const;
	Vec3d getPointOfLine(const size_t& line_nr, const size_t& point_nr) const;
	size_t getNumberOfPointsOfLine(const size_t& line_nr) const;
	std::vector<Vec3d> getAllPointsInVector() const;
	std::vector<Point> getAllPointsWithAttributesInVector() const;
	std::vector<std::vector<Vec3d>> getLinesInVectorOfVector() const;
	size_t getLineIndex(const size_t& point_index) const;
	Vec2i getLineVertexPosition(const size_t& index) const;

	//Attribute functions
	void addAttribute(const std::vector<float>& attribute, const std::string& attribute_name);
	const std::vector<float>& getAttributeByName(const std::string& attribute_name) const;

	//IO
	void Export(const char* path);
	void exportTxtFile(const char* path, const std::vector<float>& PSaxisValues);
	void Import(const char* path, const std::vector<std::string>& attributeNames = std::vector<std::string>(0));

	//Memory management
	void clear();


private:
	std::vector<float> Lines;
	std::vector<Attribute*> attributes;

	size_t n_lines;
};
