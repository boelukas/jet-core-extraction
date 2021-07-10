#pragma once
#include <string>

class TimeHelper
{
public:
	static time_t ToTimeT(std::string& date);
	static std::string ConvertHoursToDate(const size_t& time, const std::string& data_start_date);
	static size_t ConvertDateToHours(const std::string& date, const std::string& data_start_date);
	static size_t GetMonthFromHours(const size_t& time, const std::string& data_start_date);
	static size_t GetYearFromHours(const size_t& time, const std::string& data_start_date);
};
