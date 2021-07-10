#pragma once
#include <chrono>

#include "math.hpp"

class ProgressBar
{
	/*
		Class to pretty print a progress bar.
	*/
private:
	size_t size_;
	double progress_;
	std::chrono::time_point< std::chrono::high_resolution_clock > start_;
	bool startup_;
public:
	/*
		The first time step is started when the progress bar is initialized.
	*/
	ProgressBar(const size_t& _size) :size_(_size), progress_(0.0) {
		startup_ = true;
		start_ = std::chrono::high_resolution_clock::now();
		progress_ = -1;
		Print();

	}
	/*
		Call Print to Print the progress.
	*/
	void Print() {
		progress_++;
		//Progress Bar
		std::cout << "[";
		int pos = (int)(50 * progress_ / size_);
		for (int i = 0; i < 50; ++i) {
			if (i <= pos) std::cout << "#";
			else std::cout << " ";
		}
		int pr = (int)(progress_ / size_ * 100.0);

		//End Progress Bar
		std::chrono::time_point< std::chrono::high_resolution_clock > stop = std::chrono::high_resolution_clock::now();
		long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_).count();
		double estim = (size_ - progress_) * (double)(duration);
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

		start_ = std::chrono::high_resolution_clock::now();

	}
	/*
		Always call Close when the task is finished.
	*/
	void Close() const {
		std::cout << std::endl;
	}
};
