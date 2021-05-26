#pragma once

#include <iostream>
#include <iomanip>
#include <chrono>

class Timer
{
public:
	void tic()
	{
		t_start = std::chrono::high_resolution_clock::now();
	}
	double toc(bool report = true)
	{
		auto t_end = std::chrono::high_resolution_clock::now();

		double timings = std::chrono::duration<double, std::milli>(t_end - t_start).count();
		if (report)
		{
			std::cout << std::fixed << std::setprecision(2)
				<< "Wall clock time passed: "
				<< timings
				<< " ms\n";
		}
		return timings;
	}
private:

	std::chrono::high_resolution_clock::time_point t_start;
};
