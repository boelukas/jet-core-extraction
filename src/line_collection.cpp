#include <fstream>
#include <string>

#include "line_collection.hpp"

LineCollection::LineCollection() :n_lines_(0) {}
/*
	Takes a vector of vectors V as input where V[i] is the ith line V[i][j] is the jth vertex of line i.
*/
void LineCollection::SetData(const std::vector<std::vector<Vec3d>>& line_vector) {
	Clear();
	size_t pos_pointer = 0;
	size_t total_size = 0;
	n_lines_ = line_vector.size();
	if (n_lines_ != 0) {
		total_size++;
	}
	for (size_t i = 0; i < line_vector.size(); i++) {
		total_size += line_vector[i].size() * 3 + 1;
	}
	lines_ = std::vector<float>(total_size);
	if (total_size > 0) {
		lines_[0] = line_vector.size();
		pos_pointer++;
		for (size_t i = 0; i < line_vector.size(); i++) {
			lines_[pos_pointer] = line_vector[i].size();
			pos_pointer++;
		}
		for (size_t i = 0; i < line_vector.size(); i++) {
			for (size_t j = 0; j < line_vector[i].size(); j++) {
				for (size_t k = 0; k < 3; k++) {
					lines_[pos_pointer] = line_vector[i][j][k];
					pos_pointer++;
				}
			}
		}
	}
}
/*
	Takes a float vector as input which is already in the correct format.
*/
void LineCollection::SetData(const std::vector<float>& lines) {
	Clear();
	lines_ = lines;
	n_lines_ = (size_t)std::round(lines[(size_t)std::round(lines[1]) + 2]);
}
const size_t& LineCollection::GetNumberOfLines() const {
	return n_lines_;
}
size_t LineCollection::GetTotalNumberOfPoints() const {
	size_t result = (lines_.size());
	if (result == 0) { return result; }
	result = result - 1 - (size_t)std::round(lines_[0]);
	result /= 3;
	return result;
}

/*
	Frees all memory used by the line collection.
*/
void LineCollection::Clear() {
	lines_.clear();
	n_lines_ = 0;
	for (size_t i = 0; i < attributes_.size(); i++) {
		delete attributes_[i];
	}
	attributes_.clear();
}
size_t LineCollection::GetNumberOfPointsOfLine(const size_t& line_nr) const {
	return (size_t)std::round(lines_[line_nr + 1]);
}
std::vector<Vec3d> LineCollection::GetAllPointsInVector() const
{
  size_t total_pts = GetTotalNumberOfPoints();
  std::vector<Vec3d> points_all_lines(total_pts);
  size_t data_ptr = 1 + n_lines_;
  for (size_t i = 0; i < total_pts; i++)
  {
    double x = lines_[data_ptr++];
    double y = lines_[data_ptr++];
    double z = lines_[data_ptr++];
    points_all_lines[i] = Vec3d({x, y, z});
  }
  return points_all_lines;
}
const std::vector<float>& LineCollection::GetAttributeByName(const std::string& attribute_name) const {
	for (size_t i = 0; i < attributes_.size(); i++) {
		if (attributes_[i]->name == attribute_name) {
			return attributes_[i]->data;
		}
	}
  return *(std::vector<float> *)nullptr;
}
/*
	Exports the line collection to path. The memory layout is as described in the header.
*/
void LineCollection::Export(const char* path) {
	size_t lines_size = lines_.size();
	std::vector<size_t> attribute_sizes(attributes_.size());
	std::vector<size_t> header(1 + 1 + attributes_.size()); //[length of Lines, #attributes, attribute0.length, attribute1.length...]
	header[0] = lines_size;
	header[1] = attributes_.size();

	for (int i = 0; i < attributes_.size(); i++) {
		attribute_sizes[i] = attributes_[i]->data.size();
		header[(size_t)i + 2] = attributes_[i]->data.size();
	}
	{
		//write header
		size_t* header_data = (size_t*)(&header[0]);
		std::ofstream out_stream(path, std::ios::out | std::ios::binary);
		out_stream.write((char*)header_data, sizeof(size_t) * header.size());
		out_stream.close();
	}
	{
		float* float_data = (float*)(&lines_[0]);
		std::ofstream out_stream(path, std::ios::out | std::ios::app | std::ios::binary);
		out_stream.write((char*)float_data, sizeof(float) * lines_size);
		out_stream.close();
	}

	for (int i = 0; i < attributes_.size(); i++) {
		float* attribute_data = (float*)(&attributes_[i]->data[0]);
		std::ofstream out_stream(path, std::ios::out | std::ios::app | std::ios::binary);
		out_stream.write((char*)attribute_data, sizeof(float) * attribute_sizes[i]);
		out_stream.close();
	}


}
void LineCollection::ExportTxtFile(const char* path, const std::vector<float>& ps_axis_values) {
	std::vector<std::vector<Vec3d>> vec_lines = GetLinesInVectorOfVector();
	std::ofstream file;
	file.open(path, std::ios::out);
	file << "N_LINES:\n";
	file << std::to_string(vec_lines.size()) + "\n";
	file << "N_POINTS_PER_LINE:\n";
	for (size_t i = 0; i < vec_lines.size(); i++) {
		file << std::to_string(vec_lines[i].size()) + "\n";
	}
	file << "LINES (lon, lat, ps):\n";
	for (size_t i = 0; i < vec_lines.size(); i++) {
		for (size_t j = 0; j < vec_lines[i].size(); j++) {
			file << (std::to_string(vec_lines[i][j][0]) + "," + std::to_string(vec_lines[i][j][1]) + "," +
				std::to_string(CoordinateConverter::ValueOfIndexInArray(ps_axis_values, vec_lines[i][j][2], true))) + "\n";
		}
	}
	file.close();
}

void LineCollection::ExportVtp(const char *path, const std::vector<float> &ps_axis_values)
{
  std::vector<std::vector<Vec3d>> vec_lines = GetLinesInVectorOfVector();
  std::vector<Vec3d> vec_points = GetAllPointsInVector();
  std::string offsets = "";
  size_t offset_sum = 0;
  for(size_t i = 0; i < vec_lines.size(); i++){
    offset_sum += vec_lines[i].size();
    offsets += std::to_string(offset_sum);
    if(i != vec_lines.size() - 1){
      offsets += " ";
    }else{
      offsets += "\n";
    }
  }
  std::string connectivity = "";
  std::string points = "";
  for (size_t i = 0; i < vec_points.size(); i++)
  {
    points += std::to_string(vec_points[i][0]);
    points += " ";
    points += std::to_string(vec_points[i][1]);
    points += " ";
    // Convert to 10hPa scale
    points += std::to_string(CoordinateConverter::ValueOfIndexInArray(ps_axis_values, vec_points[i][2], true) * 0.1);
    connectivity += std::to_string(i);
    if (i != vec_points.size() - 1)
    {
      points += " ";
      connectivity += " ";
    }
    else
    {
      points += "\n";
      connectivity += "\n";
    }
  }


  std::ofstream file;
  file.open(path, std::ios::out);
  file << "<VTKFile type=\"PolyData\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
  file << "  <PolyData>\n";
  file << "    <Piece NumberOfPoints=\"" << GetTotalNumberOfPoints() << "\" NumberOfLines=\"" << GetNumberOfLines() << "\">\n";
  file << "      <Points>\n";
  file << "        <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n";
  file << "          " << points;
  file << "        </DataArray>\n";
  file << "      </Points>\n";
  file << "      <Lines>\n";
  file << "        <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";
  file << "          " << connectivity;
  file << "        </DataArray>\n";
  file << "        <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n";
  file << "          " << offsets;
  file << "        </DataArray>\n";
  file << "      </Lines>\n";
  file << "    </Piece>\n";
  file << "  </PolyData>\n";
  file << "</VTKFile>\n";

  file.close();
}

/*
	Imports a line collection. Stays empty if the file at path does not exist.
*/
void LineCollection::Import(const char* path, const std::vector<std::string>& attribute_names) {
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

	lines_ = std::vector<float>(length_lines);
	float* lines_p = (float*)(&lines_[0]);
	fread(lines_p, sizeof(float), length_lines, fp);
	n_lines_ = lines_[0];

	attributes_ = std::vector<Attribute*>(nr_attributes);
	for (int i = 0; i < nr_attributes; i++) {
		Attribute* attr = new Attribute;
		attr->name = attribute_names[i];
		attr->data = std::vector<float>(attribute_sizes[i]);

		float* attr_p = (float*)(&attr->data[0]);
		fread(attr_p, sizeof(float), attribute_sizes[i], fp);
		if (attribute_names[i] != "") {
			attributes_[i] = attr;
		}
	}
	fclose(fp);
}

std::vector<std::vector<Vec3d>> LineCollection::GetLinesInVectorOfVector() const {
	std::vector<std::vector<Vec3d>> result;
	int ptr = 1;
	result.resize(n_lines_);
	for (int i = 0; i < n_lines_; i++) {
		result[i].resize(lines_[ptr]);
		ptr++;
	}
	for (int i = 0; i < n_lines_; i++) {
		for (int j = 0; j < result[i].size(); j++) {
			result[i][j] = Vec3d({ lines_[ptr], lines_[ptr + 1], lines_[ptr + 2] });
			ptr += 3;
		}
	}

	return result;
}
