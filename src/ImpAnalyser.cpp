#include "ImpAnalyser.h"
#include "FSHelper.h"

ImpAnalyser::ImpAnalyser(){
	wire_mode = WIRE_MODE::FOUR_WIRE;
}

void ImpAnalyser::setStartFrequency(int staFreq){
	if (staFreq < getMinFreq()){
		staFreq = getMinFreq();
	}
	if (staFreq > getMaxFreq()){
		staFreq = getMaxFreq();
	}
	
	ImpAnalyser::startFrequency = staFreq;
}
void ImpAnalyser::setStopFrequency(int stoFreq){
	if (stoFreq < getMinFreq()){
		stoFreq = getMinFreq();
	}
	if (stoFreq > getMaxFreq()){
		stoFreq = getMaxFreq();
	}
	
	ImpAnalyser::stopFrequency = stoFreq;
}
void ImpAnalyser::setVoltage(double volt){
	if (volt < getMinVolt()){
		volt = getMinVolt();
	}
	if (volt > getMaxVolt()){
		volt = getMaxVolt();
	}
	
	ImpAnalyser::voltage = volt;
}
void ImpAnalyser::setPoints(int points){
	if (points < getMinPoints()){
		points = getMinPoints();
	}
	if (points > getMaxPoints()){
		points = getMaxPoints();
	}
	
	ImpAnalyser::points = points;
}
void ImpAnalyser::setPointAverage(int pointAv){
	if (pointAv < 1){
		pointAv = 1;
	}
	if (pointAv > getMaxPointAverage()){
		pointAv = getMaxPointAverage();
	}
	
	ImpAnalyser::pointAverage = pointAv;
}

int ImpAnalyser::getStartFrequency() const{
	return startFrequency;
}
int ImpAnalyser::getStopFrequency() const{
	return stopFrequency;
}
double ImpAnalyser::getVoltage() const{
	return voltage;
}
int ImpAnalyser::getPoints() const{
	return points;
}
int ImpAnalyser::getPointAverage() const{
	return pointAverage;
}

void ImpAnalyser::save_spectrumCSV(const std::vector<DataP::DataP_ptr>& spectre, std::string path, int position, double timediff){
	std::string header = "";
	
	if (position =! -1){
		header += "position="+ std::to_string(position) + "\n";
	}
	header += "timediff=" + std::to_string(timediff);
	
	FSHelper::save_dataPToCsv(spectre, path, true, true, true, true, "frequency", "impedance", header);
}

std::string ImpAnalyser::to_string() const{
	std::string return_string = "";
	return_string += std::to_string(getStartFrequency()) + "Hz - " + std::to_string(getStopFrequency()) + "Hz, ";
	return_string += std::to_string(getPoints()) + "pts, ";
	return_string += FSHelper::formatDouble(getVoltage()) + "V";
	
	return return_string;
}

ImpAnalyser::WIRE_MODE ImpAnalyser::getWireMode(){
	return wire_mode;
}
void ImpAnalyser::setWireMode(ImpAnalyser::WIRE_MODE w){
	wire_mode = w;
}