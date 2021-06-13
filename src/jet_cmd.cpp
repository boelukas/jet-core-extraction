#include <fstream>
#include <filesystem>
#include <string>

#include "data_helper.hpp"
#include "time_helper.hpp"
#include "jet_stream.hpp"
#include "progress_bar.hpp"

std::string ConvertPath(std::string path) {
	char last_character = path[path.length() - 1];
	char second_last_character = path[path.length() - 2];
#ifdef _WIN32
	if (last_character != '\\') {
		path += "\\";
	}
#endif
#ifdef linux
	if (last_character != '/') {
		path += "/";
	}
#endif
	return path;
}


// ---------------------------------------
// Entry point
// ---------------------------------------
int main(int argc, char* argv[])
{
	std::string src_path;
	std::string dst_path;
	bool flag_preprocess = false;
	bool src_found = false;
	bool dst_found = false;
	bool recompute = false;
	bool export_txt = false;
	JetStream::JetParameters jet_params;

  for (int i = 1; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-pMin") {
      i++;
      if (i < argc) {
        jet_params.ps_min_val = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-pMax") {
      i++;
      if (i < argc) {
        jet_params.ps_max_val = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-windspeedThreshold") {
      i++;
      if (i < argc) {
        jet_params.wind_speed_threshold = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-nStepsBelowThreshold") {
      i++;
      if (i < argc) {
        jet_params.max_steps_below_speed_thresh = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-nPredictorSteps") {
      i++;
      if (i < argc) {
        jet_params.n_predictor_steps = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-nCorrectorSteps") {
      i++;
      if (i < argc) {
        jet_params.n_corrector_steps = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-integrationStepsize") {
      i++;
      if (i < argc) {
        jet_params.integration_stepsize = atof(argv[i]);
      }
      else {
        std::cout << "Not enough arguments." << std::endl;
      }
    }
    else if (arg == "-exportTxt") {
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
        if (!src_found) {
          src_path = arg;
          src_found = true;
        }
        else if (!dst_found) {
          dst_path = arg;
          dst_found = true;
        }
        else {
          std::cout << "Unknown argument." << std::endl;
          return 0;
        }
      }
    }
  }
	if (!(src_found && dst_found)) {
		std::cout << "Source or destination directory not set. Using demo data." << std::endl;
#ifdef _WIN32
    src_path = "..\\demo_data\\";
    dst_path = "..\\demo_data\\";
#endif
#ifdef linux
    src_path = "../demo_data/";
    dst_path = "../demo_data/";
#endif
  }

	src_path = ConvertPath(src_path);
	dst_path = ConvertPath(dst_path);

  std::ofstream settingsFile;
  settingsFile.open("settings.txt");
  settingsFile << "SourceDirectory=" << src_path << "\n";
  settingsFile << "DestDirectory=" << dst_path << "\n";
  settingsFile.close();

  if (!std::filesystem::exists(dst_path)) {
		std::filesystem::create_directory(dst_path);
	}

  std::vector<std::string> time_steps = DataHelper::CollectTimes();
  std::string data_start_date = DataHelper::GetDataStartDate();
  ProgressBar pb(time_steps.size());
  JetStream* previous_jet = nullptr;
  std::vector<float> ps_axis = DataHelper::GetPsAxis();
  for (const auto& time_step : time_steps)
  {
    std::string jet_name;
    if (export_txt)
    {
      jet_name = dst_path + time_step + "_jet.txt";
    }
    else{
      jet_name = dst_path + time_step + "_jet.vtp";
    }

    if (!recompute && std::filesystem::exists(jet_name)) { pb.Print(); continue; }
    size_t hours = TimeHelper::ConvertDateToHours(time_step, data_start_date);
    JetStream* jet_stream = new JetStream(hours, jet_params, false);

    if (previous_jet != nullptr && previous_jet->GetTime() == hours - 1)
    {
      jet_stream->SetPreviousJet(previous_jet);
    }
    LineCollection jet = jet_stream->GetJetCoreLines();
    if (export_txt)
    {
      jet.ExportTxtFile(jet_name.c_str(), ps_axis);
    }
    else
    {
      jet.ExportVtp(jet_name.c_str(), ps_axis);
    }
    jet.Clear();

    jet_stream->DeletePreviousJet();
    previous_jet = jet_stream;

    pb.Print();
  }
  delete previous_jet;
  previous_jet = nullptr;

  pb.Close();

  return 0;
}
