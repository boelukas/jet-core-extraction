#include <time.h>

#include "data_helper.hpp"

#include "time_helper.hpp"

time_t TimeHelper::ToTimeT(std::string& date) {
	int start_year = std::stoi(date.substr(0, 4));
	int start_month = std::stoi(date.substr(4, 2));
	int start_day = std::stoi(date.substr(6, 2));
	int start_hour = std::stoi(date.substr(9, 2));
	struct tm data_tm = { 0, 0, start_hour, start_day, start_month - 1, start_year - 1900 };//-isdst = -1 lets mktime decide the summer or wintertime flag
	data_tm.tm_isdst = -1;
	return mktime(&data_tm);
}

#pragma warning(push)
#pragma warning(disable: 4996)
/*
		Converts hours since the first time step to date. Need to take care of summer and winter time. Thats why we add one hour in the winter.
*/
std::string TimeHelper::ConvertHoursToDate(const size_t& hours, const std::string& data_start_date) {
	std::string data_start_date_c = data_start_date;
	time_t data_start_time_t = ToTimeT(data_start_date_c);
	char out[30];
	struct tm* stm = localtime(&data_start_time_t);
	stm->tm_hour += (int)(hours);
	mktime(stm);
	if (stm->tm_isdst == 0) {//In winter time, add 1 hour. Otherwise two hours will map to the same time step.
		stm->tm_hour++;
	}
	mktime(stm);
	strftime(out, 30, "%Y%m%d_%H", stm);

	char out2[30];
	strftime(out2, 30, "%Y%m%d_%H", localtime(&data_start_time_t));

	return std::string(out);
}
#pragma warning(pop)

size_t TimeHelper::ConvertDateToHours(const std::string& date, const std::string& data_start_date) {
	std::string data_start_date_c = data_start_date;
	int year = std::stoi(date.substr(0, 4));
	int month = std::stoi(date.substr(4, 2));
	int day = std::stoi(date.substr(6, 2));
	int hour = std::stoi(date.substr(9, 2));
	struct tm stm = { 0, 0, hour, day, month - 1, year - 1900 };//-isdst = -1 lets mktime decide the summer or wintertime flag
	stm.tm_isdst = -1;

	time_t new_time = mktime(&stm);
	int test = stm.tm_isdst;
	double diff = difftime(new_time, ToTimeT(data_start_date_c));
	diff = diff / 3600;
	size_t hours;
	if (stm.tm_isdst == 0) {
		/*
			If we have winter time, then the clocks are set one hour back. This means, we should have two time steps for the hour where the time is set back.(As phyisally two hours passed)
			Because we don't have this additional time step, we need to subtract 1 hour in the winter time. Otherwise we would be shifted by 1 hour.
		*/
		hours = (size_t)std::round(diff - 1);
	}
	else {
		hours = (size_t)std::round(diff);
	}
	return hours;
}
size_t TimeHelper::GetMonthFromHours(const size_t& time, const std::string& data_start_date) {
	std::string date = ConvertHoursToDate(time, data_start_date);
	return std::stoi(date.substr(4, 2));
}
size_t TimeHelper::GetYearFromHours(const size_t& time, const std::string& data_start_date) {
	std::string date = ConvertHoursToDate(time, data_start_date);
	return std::stoi(date.substr(0, 4));
}
