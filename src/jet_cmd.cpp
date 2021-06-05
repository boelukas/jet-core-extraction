#include <fstream>
#include <filesystem>
#include <string>

#include "DataHelper.hpp"
#include "TimeHelper.hpp"
#include "JetStream.hpp"
#include "ProgressBar.hpp"

std::string convertPath(std::string p) {
	char last = p[p.length() - 1];
	char secondLast = p[p.length() - 2];
#ifdef _WIN32
	if (last != '\\') {
		p += "\\";
	}
#endif
#ifdef linux
	if (last != '/') {
		p += "/";
	}
#endif
	return p;
}


// ---------------------------------------
// Entry point
// ---------------------------------------
int main(int argc, char* argv[])
{
	std::string srcPath;
	std::string dstPath;
	bool flagPreprocess = false;
	bool srcFound = false;
	bool dstFound = false;
	bool recompute = false;
	bool export_txt = false;
	JetStream::JetParameters jetParams;
	if (argc >= 3)
	{
		for (int i = 1; i < argc; i++) {
			std::string arg = std::string(argv[i]);
			if (arg == "-pMin") {
				i++;
				if (i < argc) {
					jetParams.ps_min_val = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-pMax") {
				i++;
				if (i < argc) {
					jetParams.ps_max_val = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-windspeedThreshold") {
				i++;
				if (i < argc) {
					jetParams.wind_speed_threshold = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-nStepsBelowThreshold") {
				i++;
				if (i < argc) {
					jetParams.max_steps_below_speed_thresh = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-nPredictorSteps") {
				i++;
				if (i < argc) {
					jetParams.n_predictor_steps = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-nCorrectorSteps") {
				i++;
				if (i < argc) {
					jetParams.n_corrector_steps = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-integrationStepsize") {
				i++;
				if (i < argc) {
					jetParams.integration_stepsize = atof(argv[i]);
				}
				else {
					std::cout << "Not enough arguments." << std::endl;
				}
			}
			else if (arg == "-export_txt") {
				export_txt = true;
			}
			else if (arg == "-recompute") {
				recompute = true;
			}
			else {
				if (arg[0] == '-') {
					std::cout << "Unknown parameter: " << arg << std::endl;
					return 0;
				}
				else {
					if (!srcFound) {
						srcPath = arg;
						srcFound = true;
					}
					else if (!dstFound) {
						dstPath = arg;
						dstFound = true;
					}
					else {
						std::cout << "Unknown argument." << std::endl;
						return 0;
					}
				}
			}
		}
	}
	if (!(srcFound && dstFound)) {
		std::cout << "Source or destination directory not found. Using demo data." << std::endl;
#ifdef _WIN32
    srcPath = "..\\demo_data\\";
    dstPath = "..\\demo_data\\";
#endif
#ifdef linux
    srcPath = "../demo_data/";
    dstPath = "../demo_data/";
#endif
  }

	srcPath = convertPath(srcPath);
	dstPath = convertPath(dstPath);



	std::ofstream settingsFile;
	settingsFile.open("settings.txt");
	settingsFile << "SourceDirectory=" << srcPath << "\n";
	settingsFile << "DestDirectory=" << dstPath << "\n";
	settingsFile.close();
	if (!std::filesystem::exists(dstPath)) {
		std::filesystem::create_directory(dstPath);
	}

  std::vector<std::string> time_steps = DataHelper::collectTimes();
  ProgressBar pb(time_steps.size());
  for (const auto& elem : time_steps) {
    std::string jet_name = "";
    if (export_txt) {
      jet_name = dstPath + elem + "_jet.txt";
    }
    else {
      jet_name = dstPath + elem + "_jet.vtp";
    }

    if (!recompute && std::filesystem::exists(jet_name)) { pb.print(); continue; }
    size_t hours = TimeHelper::convertDateToHours(elem, DataHelper::getDataStartDate());
    JetStream jetStream(hours, jetParams, false);

    if (hours != 0) {
      jetStream.setUsePreviousTimeStep();
      jetStream.setUsePreprocessedPreviousJet();
    }
    LineCollection jet = jetStream.getJetCoreLines();
    if (export_txt) {
      jet.exportTxtFile(jet_name.c_str(), jetStream.getPsAxis());
    }
    else {
      jet.exportVtp(jet_name.c_str(), jetStream.getPsAxis());
    }
    jet.clear();
    pb.print();
  }
  pb.close();

	return 0;
}
