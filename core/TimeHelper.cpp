#include <time.h>

#include "DataHelper.hpp"

#include "TimeHelper.hpp"


struct tm TimeHelper::toTmStruct(std::string date) {
	int startYear = std::stoi(date.substr(0, 4));
	int startMonth = std::stoi(date.substr(4, 2));
	int startDay = std::stoi(date.substr(6, 2));
	int startHour = std::stoi(date.substr(9, 2));
	struct tm data_tm = { 0, 0, startHour, startDay, startMonth - 1, startYear - 1900 };//-isdst = -1 lets mktime decide the summer or wintertime flag
	return data_tm;
}

time_t TimeHelper::toTimeT(std::string date) {
	int startYear = std::stoi(date.substr(0, 4));
	int startMonth = std::stoi(date.substr(4, 2));
	int startDay = std::stoi(date.substr(6, 2));
	int startHour = std::stoi(date.substr(9, 2));
	//struct tm data_start_time2 = {.tm_hour=startHour, .tm_mday=startDay, .tm_mon=startMonth - 1,  .tm_year= startYear - 1900, .tm_isdst=-1};//-isdst = -1 lets mktime decide the summer or wintertime flag
	struct tm data_tm = { 0, 0, startHour, startDay, startMonth - 1, startYear - 1900 };//-isdst = -1 lets mktime decide the summer or wintertime flag
	data_tm.tm_isdst = -1;
	return mktime(&data_tm);
}
/*
	Time converter functions
*/
std::string TimeHelper::convertHoursToDate(const size_t& hours, const std::string data_start_date) {
	/*
		Converts hours since the first time step to date. Need to take care of summer and winter time. Thats why we add one hour in the winter.
	*/
	time_t data_start_time_t = toTimeT(data_start_date);
	char out[30];
	struct tm* stm = localtime(&data_start_time_t);
	stm->tm_hour += (hours);
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
size_t TimeHelper::convertDateToHours(const std::string& date, const std::string data_start_date) {
	int year = std::stoi(date.substr(0, 4));
	int month = std::stoi(date.substr(4, 2));
	int day = std::stoi(date.substr(6, 2));
	int hour = std::stoi(date.substr(9, 2));
	//struct tm stm = {.tm_hour=hour, .tm_mday=day, .tm_mon=month - 1,  .tm_year= year - 1900, .tm_isdst=-1};//isdst=-1 lets mktime decide the summer or wintertime flag
	struct tm stm = { 0, 0, hour, day, month - 1, year - 1900 };//-isdst = -1 lets mktime decide the summer or wintertime flag
	stm.tm_isdst = -1;

	time_t new_time = mktime(&stm);
	int test = stm.tm_isdst;
	double diff = difftime(new_time, toTimeT(data_start_date));
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
		/*
			What happens in the data
		*/
		hours = (size_t)std::round(diff);
	}
	return hours;
}
size_t TimeHelper::convertHoursToHoursOfMonth(const size_t& time, const std::string data_start_date) {
	std::string date = convertHoursToDate(time, data_start_date);
	size_t day = std::stoi(date.substr(6, 2));
	size_t hour = std::stoi(date.substr(9, 2));
	return (day - 1) * 24 + hour;
}
size_t TimeHelper::getMonthFromHours(const size_t& time, const std::string data_start_date) {
	std::string date = convertHoursToDate(time, data_start_date);
	return std::stoi(date.substr(4, 2));
}
size_t TimeHelper::getYearFromHours(const size_t& time, const std::string data_start_date) {
	std::string date = convertHoursToDate(time, data_start_date);
	return std::stoi(date.substr(0, 4));
}

size_t TimeHelper::getTotalNrHoursInWCBTrajecories() {
	std::vector<std::string> monthes = DataHelper::collectWcbTimes();
	int low_y = 9000000;
	int low_m = 13;

	std::string lowest = "";
	int high_y = -9000000;
	int high_m = -1;
	std::string highest = "";
	for (const auto& entry : monthes) {
		int year = std::stoi(entry.substr(0, 4));
		int month = std::stoi(entry.substr(4, 2));

		if ((year < low_y) ||
			(year == low_y && month < low_m))
		{
			lowest = entry;
			low_y = year;
			low_m = month;
		}
		if ((year > high_y) ||
			(year == high_y && month > high_m))
		{
			highest = entry;
			high_y = year;
			high_m = month;
		}
	}

	time_t rawtime;
	time(&rawtime);
	struct tm* start = localtime(&rawtime);
	start->tm_year = std::stoi(lowest.substr(0, 4)) - 1900;
	start->tm_mon = std::stoi(lowest.substr(4, 2)) - 1;
	start->tm_mday = 1;
	start->tm_hour = 0;
	start->tm_min = 0;
	start->tm_sec = 0;
	time_t start_time = mktime(start);

	struct tm* end = localtime(&rawtime);
	end->tm_year = std::stoi(highest.substr(0, 4)) - 1900;
	end->tm_mon = std::stoi(highest.substr(4, 2));
	end->tm_mday = 1;
	end->tm_hour = 0;
	end->tm_min = 0;
	end->tm_sec = 0;
	time_t end_time = mktime(end);

	double diff = difftime(end_time, start_time);
	diff = diff / 3600;

	size_t hours = (size_t)std::round(diff);
	return hours;
}
size_t TimeHelper::hoursBeteenWCBTrajectoryStartAndDataStart() {
	std::string data_start = DataHelper::getDataStartDate();
	std::vector<std::string> monthes = DataHelper::collectWcbTimes();
	int low_y = 9000000;
	int low_m = 13;
	std::string wcb_start;
	for (const auto& entry : monthes) {
		int year = std::stoi(entry.substr(0, 4));
		int month = std::stoi(entry.substr(4, 2));

		if ((year < low_y) ||
			(year == low_y && month < low_m))
		{
			wcb_start = entry;
			low_y = year;
			low_m = month;
		}
	}
	time_t rawtime;
	time(&rawtime);
	struct tm* start = localtime(&rawtime);
	start->tm_year = std::stoi(wcb_start.substr(0, 4)) - 1900;
	start->tm_mon = std::stoi(wcb_start.substr(4, 2)) - 1;
	start->tm_mday = 1;
	start->tm_hour = 0;
	start->tm_min = 0;
	start->tm_sec = 0;
	time_t start_time = mktime(start);

	struct tm* end = localtime(&rawtime);
	end->tm_year = std::stoi(data_start.substr(0, 4)) - 1900;
	end->tm_mon = std::stoi(data_start.substr(4, 2)) - 1;
	end->tm_mday = std::stoi(data_start.substr(6, 2));
	end->tm_hour = std::stoi(data_start.substr(9, 2));
	end->tm_min = 0;
	end->tm_sec = 0;
	time_t end_time = mktime(end);

	double diff = difftime(end_time, start_time);
	diff = diff / 3600;
	size_t hours = (size_t)std::round(diff);
	return hours;
}
