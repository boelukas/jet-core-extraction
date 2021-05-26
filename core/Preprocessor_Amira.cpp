#include <filesystem>
#include <iostream>

#include "Amira.hpp"
#include "DataHelper.hpp"
#include "TimeHelper.hpp"
#include "JetStream.hpp"
#include "ProgressBar.hpp"
#include "WindFields.hpp"
#include "Wcb.hpp"
#include "NetCDF.hpp"

#include "Preprocessor_Amira.hpp"

namespace fs = std::filesystem;


//Preprocessor_Amira::Preprocessor_Amira(DataLoader_Amira& dataloader, JetStream& jetAlgo, const bool& recompute)
Preprocessor_Amira::Preprocessor_Amira(const JetStream::JetParameters& _jetParameter, const bool& recompute, const bool& export_txt) :export_txt_(export_txt), jetParameter(_jetParameter)
{
	srcPath = DataHelper::getSrcPath();
	destPath = DataHelper::getPreprocPath();
	mode = recompute ? EPreProcMode::REPLACE_EXISTING : EPreProcMode::KEEP_EXISTING;
	time_steps = DataHelper::collectTimes();
}

void Preprocessor_Amira::process() {
	std::cout << "Computing axes: " << std::endl;
	compute_axes();
	std::cout << "Computing axes: done." << std::endl;
	std::cout << "Computing PS3D: " << std::endl;
	compute_PS3D();
	std::cout << "Computing PS3D: done." << std::endl;
	std::cout << "Computing Scalar Fields: " << std::endl;
	resampleFieldsToEra();
	std::cout << "Computing Scalar Fields: done." << std::endl;
	std::cout << "Computing Jet: " << std::endl;
	compute_jet();
	std::cout << "Computing Jet: done." << std::endl;
	std::cout << "Computing windMagnitude: " << std::endl;
	compute_windMagnitude();
	std::cout << "Computing windMagnitude: done" << std::endl;
	std::cout << "Computing WCBs: " << std::endl;
	compute_WCB();
	std::cout << "Computing WCBs: done" << std::endl;
}
/*
	Resamples and saves the pfields and sfields specified in the header.
	Needs PS3D to be precomputed.
*/
bool Preprocessor_Amira::resampleFieldsToEra() const {
	int numFiles = std::distance(std::filesystem::directory_iterator(srcPath), std::filesystem::directory_iterator{}) - 1;
	//bool exported_Axes = false;
	ProgressBar pb = ProgressBar(time_steps.size());
	for (const auto& time : time_steps)
	{
		std::string entryName = 'P' + time;
		std::string entryPath = srcPath + entryName;

		RegScalarField3f* PS = nullptr;
		for (int i = 0; i < pfields.size(); i++) {
			std::string pfield_filename = createFileName(entryName, pfields[i]);
			if (mode == EPreProcMode::KEEP_EXISTING && fs::exists(pfield_filename)) { continue; }
			PS = Amira::ImportScalarField3f(createFileName(entryName, custom_fields[0]).c_str());
			RegScalarField3f* data = NetCDF::ImportScalarField3f(entryPath, pfields[i], "lon", "lat", "lev");
			EraGrid<float> era = EraGrid<float>(data, PS);
			Vec3d min_dom = Vec3d({ data->GetDomain().GetMin()[0], data->GetDomain().GetMin()[1], PS_domain_min });
			Vec3d max_dom = Vec3d({ data->GetDomain().GetMax()[0], data->GetDomain().GetMax()[1], PS_domain_max });
			BoundingBox3d resample_domain = BoundingBox3d(min_dom, max_dom);

			RegScalarField3f* data_era = new RegScalarField3f(Vec3i({ data->GetResolution()[0],data->GetResolution()[1], pressureResolution }), resample_domain);
			era.Resample_Amira(*data_era, stepSize);

			Amira::Export(pfield_filename.c_str(), data_era);
			delete data;
			delete data_era;
		}

		entryName = 'S' + time;
		entryPath = srcPath + entryName;
		for (int i = 0; i < sfields.size(); i++) {
			std::string sfield_filename = createFileName(entryName, sfields[i]);
			if (mode == EPreProcMode::KEEP_EXISTING && fs::exists(sfield_filename)) { continue; }
			if (PS == nullptr) { PS = Amira::ImportScalarField3f(createFileName(entryName, custom_fields[0]).c_str()); }
			RegScalarField3f* data = NetCDF::ImportScalarField3f(entryPath, sfields[i], "lon", "lat", "lev");
			EraGrid<float> era = EraGrid<float>(data, PS);

			Vec3d min_dom = Vec3d({ data->GetDomain().GetMin()[0], data->GetDomain().GetMin()[1], PS_domain_min });
			Vec3d max_dom = Vec3d({ data->GetDomain().GetMax()[0], data->GetDomain().GetMax()[1], PS_domain_max });
			BoundingBox3d resample_domain = BoundingBox3d(min_dom, max_dom);

			RegScalarField3f* data_era = new RegScalarField3f(Vec3i({ data->GetResolution()[0], data->GetResolution()[1], pressureResolution }), resample_domain);
			era.Resample_Amira(*data_era, stepSize);

			Amira::Export(sfield_filename.c_str(), data_era);
			delete data;
			delete data_era;
		}
		if (PS != nullptr) { delete PS; }
		pb.print();
	}
	pb.close();
	return true;
}
/*
	Computes the Jet core lines for all timesteps.
*/
bool Preprocessor_Amira::compute_jet() const {
	//[pmin, pmax, tauWind, dt, stepsBelowThresh, nPredSteps, nCorrSteps, seedFilterStrength]

	ProgressBar pb = ProgressBar(time_steps.size());
	for (const auto& elem : time_steps) {
		std::string jet_name = destPath + elem + "_jet";
		if (mode == EPreProcMode::KEEP_EXISTING && fs::exists(jet_name)) { pb.print(); continue; }

		size_t hours = TimeHelper::convertDateToHours(elem, DataHelper::getDataStartDate());
		JetStream jetStream(hours, jetParameter, true);
		if (hours != 0) {
			jetStream.setUsePreviousTimeStep();
			jetStream.setUsePreprocessedPreviousJet();
		}
		LineCollection jet = jetStream.getJetCoreLines();

		jet.Export(jet_name.c_str());
		if (export_txt_) {
			jet_name = destPath + elem + "_jet.txt";
			jet.exportTxtFile(jet_name.c_str(), jetStream.getPsAxis());
		}
		jet.clear();
		pb.print();
	}
	pb.close();
	return true;
}
/*
	Computes the WCB for all time steps.
*/
bool Preprocessor_Amira::compute_WCB() const {
	//DataLoader_Amira dl = DataLoader_Amira();
	std::string wcb_src = srcPath;
	std::string wcb_dest = destPath;
	std::vector<std::string> times = DataHelper::collectWcbTimes();
	for (const auto& time : times) {
		LineCollection trajectories = LineCollection();
		std::string f_name = wcb_dest + time + "_wcb";
		if (!fs::exists(f_name) || mode == EPreProcMode::REPLACE_EXISTING) {
			Wcb::readInputFileAndExportAsLineCollection(wcb_src + "lcy_" + time, wcb_dest + time + "_wcb", trajectories);
		}
		else {
			std::vector<std::string> attr_names = { "time" };
			trajectories.Import(f_name.c_str(), attr_names);
		}
	}
	return true;
}


/*
	Computes the wind magnitudes in level and ps.
*/
bool Preprocessor_Amira::compute_windMagnitude() const {
	//DataLoader_Amira dl = DataLoader_Amira();
	WindFields wf = WindFields();
	std::vector<RegScalarField3f*> fields;
	ProgressBar pb = ProgressBar(time_steps.size());
	for (const auto& elem : time_steps) {
		std::string wind_mag_lev_name = destPath + elem + "_windMagnitude_lev";
		std::string wind_mag_ps_name = destPath + elem + "_windMagnitude_ps";

		if (mode == EPreProcMode::KEEP_EXISTING && fs::exists(wind_mag_lev_name) && fs::exists(wind_mag_ps_name)) { pb.print(); continue; }
		size_t hours = TimeHelper::convertDateToHours(elem, DataHelper::getDataStartDate());
		std::vector<RegScalarField3f*> fields = DataHelper::loadScalarFields(hours, std::vector<std::string>({ "PS3D", "U", "V", "OMEGA", "T" }), std::vector<bool>({ true, false, false, false, false }));
		RegScalarField3f* PS3D = fields[0];
		RegScalarField3f* U = fields[1];
		RegScalarField3f* V = fields[2];
		RegScalarField3f* OMEGA = fields[3];
		RegScalarField3f* T = fields[4];
		EraScalarField3f* windMagnitude = wf.GetWindMagnitudeEra(hours, DataHelper::loadFloatArray("ps"), PS3D, U, V, OMEGA, T);
		Amira::Export(wind_mag_lev_name.c_str(), windMagnitude->GetField());

		Vec3d min_dom = Vec3d({ windMagnitude->GetField()->GetDomain().GetMin()[0], windMagnitude->GetField()->GetDomain().GetMin()[1], PS_domain_min });
		Vec3d max_dom = Vec3d({ windMagnitude->GetField()->GetDomain().GetMax()[0],  windMagnitude->GetField()->GetDomain().GetMax()[1], PS_domain_max });
		BoundingBox3d resample_domain = BoundingBox3d(min_dom, max_dom);
		RegScalarField3f* windMagnitude_resampled = new RegScalarField3f(Vec3i({ windMagnitude->GetField()->GetResolution()[0], windMagnitude->GetField()->GetResolution()[1], pressureResolution }), resample_domain);
		windMagnitude->Resample_Amira(*windMagnitude_resampled, stepSize);
		Amira::Export(wind_mag_ps_name.c_str(), windMagnitude_resampled);

		for (size_t i = 0; i < fields.size(); i++) {
			delete fields[i];
		}
		fields.clear();
		delete windMagnitude_resampled;
		delete windMagnitude->GetField();
		delete windMagnitude;
		pb.print();
	}
	pb.close();
	return true;
}
/*
	Computes the 3D pressure.
*/
bool Preprocessor_Amira::compute_PS3D() const {
	RegScalarField3f* PS;
	std::string path = DataHelper::getSrcPath() + "P" + DataHelper::getDataStartDate();
	RegScalarField3f* U = NetCDF::ImportScalarField3f(path, "U", "lon", "lat", "lev");
	ProgressBar pb = ProgressBar(time_steps.size());
	for (const auto& elem : time_steps) {
		//size_t hours = dl.convertDateToHours(elem);
		std::string PS_filename = destPath + elem + "_PS3D";

		if (mode == EPreProcMode::KEEP_EXISTING && fs::exists(PS_filename)) { pb.print(); continue; }

		RegScalarField3f* PS = DataHelper::compute_PS3D(elem, U->GetResolution(), U->GetDomain());

		Amira::Export(PS_filename.c_str(), PS);
		delete PS;
		pb.print();
	}
	pb.close();
	return true;
}
/*
	Reads the coordinate axes and exports them.
*/
bool Preprocessor_Amira::compute_axes() const {
	for (const auto& elem : time_steps) {

		std::vector<float> longitude;
		std::vector<float> latitude;
		std::vector<float> pressure(pressureResolution);
		pressure[0] = PS_domain_min;
		for (size_t i = 1; i < pressureResolution; i++) {
			pressure[i] = PS_domain_min + i * stepSize;
		}
		NetCDF::ImportFloatArray(srcPath + "P" + elem, "lon", longitude);
		NetCDF::ImportFloatArray(srcPath + "P" + elem, "lat", latitude);
		std::string long_name = destPath + "lon";
		std::string lat_name = destPath + "lat";
		std::string ps_name = destPath + "ps";

		Amira::Export(long_name.c_str(), longitude);
		Amira::Export(lat_name.c_str(), latitude);
		Amira::Export(ps_name.c_str(), pressure);
		break;
	}
	return true;
}
//Helper functions
std::string Preprocessor_Amira::createFileName(const std::string& entryName, const std::string& fieldName) const {
	std::string time = entryName.substr(1, entryName.size() - 1);
	std::string t = destPath + time + "_" + fieldName;
	return t;
}

void Preprocessor_Amira::printProgressBar(double& progress, const size_t& total) const {
	progress++;
	//Progress Bar
	std::cout << "[";
	int pos = 50 * progress / total;
	for (int i = 0; i < 50; ++i) {
		if (i <= pos) std::cout << "#";
		else std::cout << " ";
	}
	int pr = progress / total * 100.0;
	std::cout << "] " << pr << " %\r";
	std::cout.flush();
	//End Progress Bar
}
