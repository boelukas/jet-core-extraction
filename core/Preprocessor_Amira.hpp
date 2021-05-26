#pragma once
#include "JetStream.hpp"

class Preprocessor_Amira
{
	typedef std::vector<std::string>  vecStr;
	typedef std::vector<RegScalarField3f*> vecScalarFields;
public:
	enum class EPreProcMode { REPLACE_EXISTING, KEEP_EXISTING };
	Preprocessor_Amira(const JetStream::JetParameters& jetParameter, const bool& recompute, const bool& export_txt);
	void process();

	//Preprocessing functions
	bool resampleFieldsToEra() const;
	bool compute_jet() const;
	bool compute_WCB() const;
	bool compute_PS3D() const;
	bool compute_axes() const;
	bool compute_windMagnitude() const;
private:
	//Helper functions
	std::string createFileName(const std::string& entryName, const std::string& fieldName) const;
	void printProgressBar(double& progress, const size_t& total) const;

	//Fields
	std::string srcPath;
	std::string destPath;
	EPreProcMode mode;
	const JetStream::JetParameters& jetParameter;
	vecStr pfields = { "Q", "T", "OMEGA","LWC", "IWC", "RWC", "SWC", "U", "V" };
	vecStr sfields = { "PV", "THE" };
	vecStr tfields = { "RI", "TI" };
	vecStr custom_fields = { "PS3D" };
	std::string preprocessing_directory_name = "Preprocessing_Amira";
	std::vector <std::string> time_steps;
	bool export_txt_;

	//Step size for computing the 3D pressure. This will be the stepsize of the pressure axis.
	const int stepSize = 10;
	const double PS_domain_min = 10.;
	const double PS_domain_max = 1040.;
	const int pressureResolution = 104;

};
