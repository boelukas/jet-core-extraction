#include <fstream>
#include <string>

#include "LineCollection.hpp"

LineCollection::LineCollection() :n_lines(0) {}
/*
	Takes a vector of vectors V as input where V[i] is the ith line V[i][j] is the jth vertex of line i.
*/
void LineCollection::setData(const std::vector<std::vector<Vec3d>>& lines) {
	clear();
	size_t pos_pointer = 0;
	size_t totalSize = 0;
	n_lines = lines.size();
	if (n_lines != 0) {
		totalSize++;
	}
	for (size_t i = 0; i < lines.size(); i++) {
		totalSize += lines[i].size() * 3 + 1;
	}
	Lines = std::vector<float>(totalSize);
	if (totalSize > 0) {
		Lines[0] = lines.size();
		pos_pointer++;
		for (size_t i = 0; i < lines.size(); i++) {
			Lines[pos_pointer] = lines[i].size();
			pos_pointer++;
		}
		for (size_t i = 0; i < lines.size(); i++) {
			for (size_t j = 0; j < lines[i].size(); j++) {
				for (size_t k = 0; k < 3; k++) {
					Lines[pos_pointer] = lines[i][j][k];
					pos_pointer++;
				}
			}
		}
	}
}
/*
	Takes a float vector as input which is already in the correct format.
*/
void LineCollection::setData(const std::vector<float>& lines) {
	clear();
	Lines = lines;
	n_lines = (size_t)std::round(lines[(size_t)std::round(lines[1]) + 2]);
}
const std::vector<float>& LineCollection::getLinesAsVector() const {
	return Lines;
}
const size_t& LineCollection::getNumberOfLines() const {
	return n_lines;
}
size_t LineCollection::getTotalNumberOfPoints() const {
	size_t result = (Lines.size());
	if (result == 0) { return result; }
	result = result - 1 - (size_t)std::round(Lines[0]);
	result /= 3;
	return result;
}
size_t LineCollection::getIndexInAttributeArray(const size_t& line_nr, const size_t& point_nr) const {
	size_t sum_points = 0;
	for (size_t i = 0; i < line_nr; i++) {
		sum_points += getNumberOfPointsOfLine(i);
	}
	sum_points += point_nr;
	return sum_points;
}
Vec3d LineCollection::getPoint(const size_t& index) const {
	size_t x = 1 + n_lines + (index * 3);
	size_t y = 1 + n_lines + (index * 3) + 1;
	size_t z = 1 + n_lines + (index * 3) + 2;
	return Vec3d({ Lines[x], Lines[y], Lines[z] });
}
size_t LineCollection::getLineIndex(const size_t& point_index) const {
	int idx = point_index;
	size_t line_idx = 0;
	while (idx >= 0) {
		idx -= Lines[1 + line_idx];
		line_idx++;
	}
	return line_idx - 1;
}
Vec2i LineCollection::getLineVertexPosition(const size_t& index) const {
	int idx = index;
	size_t line_idx = 0;
	while (idx >= 0) {
		idx -= Lines[1 + line_idx];
		line_idx++;
	}
	return Vec2i({ (int)line_idx - 1, idx + (int)Lines[line_idx] });
}
Vec3d LineCollection::getPointOfLine(const size_t& line_nr, const size_t& point_nr) const {
	size_t line_start = (size_t)std::round(Lines[0]) + 1;
	for (size_t i = 0; i < line_nr; i++) {
		line_start += (size_t)std::round(Lines[i + 1]) * 3;
	}
	float x = Lines[line_start + point_nr * 3];
	float y = Lines[line_start + point_nr * 3 + 1];
	float z = Lines[line_start + point_nr * 3 + 2];
	return Vec3d({ x, y, z });
}

/*
	Frees all memory used by the line collection.
*/
void LineCollection::clear() {
	Lines.clear();
	n_lines = 0;
	for (size_t i = 0; i < attributes.size(); i++) {
		delete attributes[i];
	}
	attributes.clear();
}
size_t LineCollection::getNumberOfPointsOfLine(const size_t& line_nr) const {
	return (size_t)std::round(Lines[line_nr + 1]);
}
std::vector<Vec3d> LineCollection::getAllPointsInVector() const {
	size_t total_pts = getTotalNumberOfPoints();
	std::vector<Vec3d> points_all_lines(total_pts);
	size_t data_ptr = 1 + n_lines;
	for (size_t i = 0; i < total_pts; i++) {
		double x = Lines[data_ptr++];
		double y = Lines[data_ptr++];
		double z = Lines[data_ptr++];
		points_all_lines[i] = Vec3d({ x, y, z });
	}
	return points_all_lines;
}
/*
	Returns a vector with points, where each point has its attributes stored in a vector.
*/
std::vector<LineCollection::Point> LineCollection::getAllPointsWithAttributesInVector() const {
	size_t total_pts = getTotalNumberOfPoints();
	std::vector<LineCollection::Point> points_all_lines(total_pts);
	size_t data_ptr = 1 + n_lines;
	for (size_t i = 0; i < total_pts; i++) {
		Point p;
		double x = Lines[data_ptr++];
		double y = Lines[data_ptr++];
		double z = Lines[data_ptr++];
		p.coord = Vec3d({ x, y, z });
		p.attributes = std::vector<float>(attributes.size() + 1);
		for (size_t j = 0; j < attributes.size(); j++) {
			p.attributes[j] = attributes[j]->data[i];
		}
		p.attributes[attributes.size()] = i;
		points_all_lines[i] = p;
	}
	return points_all_lines;
}
/*
	Adds an attribute with attribute_name to the lines.
*/
void LineCollection::addAttribute(const std::vector<float>& attribute, const std::string& attribute_name) {
	Attribute* attr = new Attribute;
	attr->data = attribute;
	attr->name = attribute_name;
	attributes.push_back(attr);
}
const std::vector<float>& LineCollection::getAttributeByName(const std::string& attribute_name) const {
	for (size_t i = 0; i < attributes.size(); i++) {
		if (attributes[i]->name == attribute_name) {
			return attributes[i]->data;
		}
	}
}
/*
	Exports the line collection to path. The memory layout is as described in the header.
*/
void LineCollection::Export(const char* path) {
	size_t lines_size = Lines.size();
	std::vector<size_t> attribute_sizes(attributes.size());
	std::vector<size_t> header(1 + 1 + attributes.size()); //[length of Lines, #attributes, attribute0.length, attribute1.length...]
	header[0] = lines_size;
	header[1] = attributes.size();

	for (int i = 0; i < attributes.size(); i++) {
		attribute_sizes[i] = attributes[i]->data.size();
		header[(size_t)i + 2] = attributes[i]->data.size();
	}
	{
		//write header
		size_t* headData = (size_t*)(&header[0]);
		std::ofstream outStream(path, std::ios::out | std::ios::binary);
		outStream.write((char*)headData, sizeof(size_t) * header.size());
		outStream.close();
	}
	{
		float* fltData = (float*)(&Lines[0]);
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)fltData, sizeof(float) * lines_size);
		outStream.close();
	}

	for (int i = 0; i < attributes.size(); i++) {
		float* attributeData = (float*)(&attributes[i]->data[0]);
		std::ofstream outStream(path, std::ios::out | std::ios::app | std::ios::binary);
		outStream.write((char*)attributeData, sizeof(float) * attribute_sizes[i]);
		outStream.close();
	}


}
void LineCollection::exportTxtFile(const char* path, const std::vector<float>& PSaxisValues) {
	std::vector<std::vector<Vec3d>> vec_lines = getLinesInVectorOfVector();
	std::ofstream myfile;
	myfile.open(path, std::ios::out);
	myfile << "N_LINES:\n";
	myfile << std::to_string(vec_lines.size()) + "\n";
	myfile << "N_POINTS_PER_LINE:\n";
	for (size_t i = 0; i < vec_lines.size(); i++) {
		myfile << std::to_string(vec_lines[i].size()) + "\n";
	}
	myfile << "LINES (lon, lat, ps):\n";
	for (size_t i = 0; i < vec_lines.size(); i++) {
		for (size_t j = 0; j < vec_lines[i].size(); j++) {
			myfile << (std::to_string(vec_lines[i][j][0]) + "," + std::to_string(vec_lines[i][j][1]) + "," +
				std::to_string(CoordinateConverter::valueOfIndexInArray(PSaxisValues, vec_lines[i][j][2], true))) + "\n";
		}
	}
	myfile.close();
}

/*
	Imports a line collection. Stays empty if the file at path does not exist.
*/
void LineCollection::Import(const char* path, const std::vector<std::string>& attributeNames) {
	FILE* fp = fopen(path, "rb");
	if (fp == nullptr) {
		return;
	}
	size_t* head = new size_t[2];
	fread(head, sizeof(size_t), 2, fp);
	size_t length_lines = head[0];
	size_t nr_attributes = head[1];
	std::vector<size_t> attribute_sizes(nr_attributes);

	if (nr_attributes != 0) {
		size_t* attributes_size_p = (size_t*)(&attribute_sizes[0]);
		fread(attributes_size_p, sizeof(size_t), nr_attributes, fp);
	}

	Lines = std::vector<float>(length_lines);
	float* lines_p = (float*)(&Lines[0]);
	fread(lines_p, sizeof(float), length_lines, fp);
	n_lines = Lines[0];

	attributes = std::vector<Attribute*>(nr_attributes);
	for (int i = 0; i < nr_attributes; i++) {
		Attribute* attr = new Attribute;
		attr->name = attributeNames[i];
		attr->data = std::vector<float>(attribute_sizes[i]);

		float* attr_p = (float*)(&attr->data[0]);
		fread(attr_p, sizeof(float), attribute_sizes[i], fp);
		if (attributeNames[i] != "") {
			attributes[i] = attr;
		}
	}
	fclose(fp);
}

std::vector<std::vector<Vec3d>> LineCollection::getLinesInVectorOfVector() const {
	std::vector<std::vector<Vec3d>> result;
	int ptr = 1;
	result.resize(n_lines);
	for (int i = 0; i < n_lines; i++) {
		result[i].resize(Lines[ptr]);
		ptr++;
	}
	for (int i = 0; i < n_lines; i++) {
		for (int j = 0; j < result[i].size(); j++) {
			result[i][j] = Vec3d({ Lines[ptr], Lines[ptr + 1], Lines[ptr + 2] });
			ptr += 3;
		}
	}

	return result;
}
