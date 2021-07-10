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
		lines_[0] = (float)line_vector.size();
		pos_pointer++;
		for (size_t i = 0; i < line_vector.size(); i++) {
			lines_[pos_pointer] = (float)line_vector[i].size();
			pos_pointer++;
		}
		for (size_t i = 0; i < line_vector.size(); i++) {
			for (size_t j = 0; j < line_vector[i].size(); j++) {
				for (size_t k = 0; k < 3; k++) {
					lines_[pos_pointer] = (float)line_vector[i][j][k];
					pos_pointer++;
				}
			}
		}
	}
}
/*
	Takes a float vector as input which is already in the format described in the header.
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
		points_all_lines[i] = Vec3d({ x, y, z });
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

std::vector<std::vector<Vec3d>> LineCollection::GetLinesInVectorOfVector() const
{
	std::vector<std::vector<Vec3d>> result;
	int ptr = 1;
	result.resize(n_lines_);
	for (int i = 0; i < n_lines_; i++)
	{
		result[i].resize((size_t)lines_[ptr]);
		ptr++;
	}
	for (int i = 0; i < n_lines_; i++)
	{
		for (int j = 0; j < result[i].size(); j++)
		{
			result[i][j] = Vec3d({ lines_[ptr], lines_[ptr + 1ll], lines_[ptr + 2ll] });
			ptr += 3;
		}
	}
	return result;
}

void LineCollection::ExportTxtFile(const char* path, const std::vector<float>& ps_axis_values) const {
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
               std::to_string(CoordinateConverter::ValueOfIndexInArray(ps_axis_values, (float)vec_lines[i][j][2], true) * 0.1)) +
                  "\n";
    }
	}
	file.close();
}

void LineCollection::ExportVtp(const char* path, const std::vector<float>& ps_axis_values) const
{
	std::vector<std::vector<Vec3d>> vec_lines = GetLinesInVectorOfVector();
	std::vector<Vec3d> vec_points = GetAllPointsInVector();
	std::string offsets = "";
	size_t offset_sum = 0;
	for (size_t i = 0; i < vec_lines.size(); i++) {
		offset_sum += vec_lines[i].size();
		offsets += std::to_string(offset_sum);
		if (i != vec_lines.size() - 1) {
			offsets += " ";
		}
		else {
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
		points += std::to_string(CoordinateConverter::ValueOfIndexInArray(ps_axis_values, (float)vec_points[i][2], true) * 0.1);
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

