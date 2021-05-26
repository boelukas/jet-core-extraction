#pragma once
#include <chrono> 

#include "Math.hpp"

class ProgressBar
{
	/*
		Class to pretty print a progress bar.
	*/
private:
	size_t size;
	double progress;
	std::chrono::time_point< std::chrono::high_resolution_clock > start;
	bool startup;
public:
	/*
		The first time step is started when the progress bar is initialized.
	*/
	ProgressBar(const size_t& _size) :size(_size), progress(0.0) {
		startup = true;
		start = std::chrono::high_resolution_clock::now();
		progress = -1;
		print();

	}
	/*
		Call print to print the progress.
	*/
	void print() {
		progress++;
		//Progress Bar
		std::cout << "[";
		int pos = 50 * progress / size;
		for (int i = 0; i < 50; ++i) {
			if (i <= pos) std::cout << "#";
			else std::cout << " ";
		}
		int pr = progress / size * 100.0;
		//std::cout << "] " << pr << " %\r";
		//std::cout.flush();
		//End Progress Bar
		std::chrono::time_point< std::chrono::high_resolution_clock > stop = std::chrono::high_resolution_clock::now();
		long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		double estim = (size - progress) * (double)(duration);
		double estim_s = estim / 1000;
		double estim_m = estim_s / 60;
		double estim_h = estim_m / 60;
		double estim_d = estim_h / 24;
		if (estim_d >= 1) {
			std::cout << "] " << pr << " %, " << "Estim: " << estim_d << " days     \r";
		}
		else if (estim_h >= 1) {
			std::cout << "] " << pr << " %, " << "Estim: " << estim_h << " hours    \r";
		}
		else if (estim_m >= 1) {
			std::cout << "] " << pr << " %, " << "Estim: " << estim_m << " min      \r";
		}
		else if (estim_s >= 1) {
			std::cout << "] " << pr << " %, " << "Estim: " << estim_s << " s          \r";
		}
		else {
			std::cout << "] " << pr << " %, " << "Estim: " << estim << " ms         \r";
		}
		std::cout.flush();

		start = std::chrono::high_resolution_clock::now();

	}
	/*
		Always call close when the task is finished.
	*/
	void close() const {
		std::cout << std::endl;
	}
};
