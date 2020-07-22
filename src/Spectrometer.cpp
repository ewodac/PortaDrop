#include "Spectrometer.h"
#include "FSHelper.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdexcept>

Spectrometer::Spectrometer(){
	spectr = SeaBreezeAPI::getInstance();
	scansToAverage = 1;
	triggerMode = TRIGGER_MODE::TRIGGER_MODE_NORMAL;
	init();
}
Spectrometer::Spectrometer_ptr Spectrometer::create(){
	return std::make_shared<Spectrometer>();
}

Spectrometer::~Spectrometer(){
	
}

double Spectrometer::getMaxIntensity() {
	int error = 0;
	double maxIntensity = -1;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		spectr->openDevice(id, &error); 
		if (error != 0) throw std::runtime_error("openDevice - " + std::string(sbapi_get_error_string(error))); 
		
		
		maxIntensity = spectr->spectrometerGetMaximumIntensity(id, spectrometer_id, &error); if (error != 0) throw std::runtime_error("spectrometerGetMaximumIntensity - " + std::string(sbapi_get_error_string(error)));
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
	return maxIntensity;
}
unsigned long Spectrometer::getMinIntegrationTimeMicros() {
	int error = 0;
	double minIntegrationTime = -1;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice();
		minIntegrationTime = spectr->spectrometerGetMinimumIntegrationTimeMicros(id, spectrometer_id, &error); if (error != 0) throw std::runtime_error("spectrometerGetMinimumIntegrationTimeMicros - " + std::string(sbapi_get_error_string(error)));
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
	return minIntegrationTime;
}
unsigned long Spectrometer::getMaxIntegrationTimeMicros() {
	int error = 0;
	double maxIntegrationTime = -1;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice();
		maxIntegrationTime = spectr->spectrometerGetMaximumIntegrationTimeMicros(id, spectrometer_id, &error); if (error != 0) throw std::runtime_error("spectrometerGetMaximumIntegrationTimeMicros - " + std::string(sbapi_get_error_string(error)));
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
	return maxIntegrationTime;
}
std::vector<double> Spectrometer::getFormattedSpectrum() {
	int error = 0;
	std::vector<double> spectrum;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice();
		spectrum = getSpectrum();
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
	return spectrum;
}
std::vector<DataP::DataP_ptr> Spectrometer::getFormattedSpectrum_DataPoints() {
	int error = 0;
	std::vector<DataP::DataP_ptr> spectrum;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice();
		
		std::vector<double> formattedSpectrum = getSpectrum();
		double *wavelengths = new double[formattedSpectrum.size()];
		
		spectr->spectrometerGetWavelengths(id, spectrometer_id, &error, wavelengths, formattedSpectrum.size()); if (error != 0) throw std::runtime_error("spectrometerGetWavelengths - " + std::string(sbapi_get_error_string(error)));
		
		
		for (int i = 0; i < formattedSpectrum.size(); i++){
			spectrum.push_back(std::make_shared<DataP>(wavelengths[i], formattedSpectrum.at(i)));
		}
		delete [] wavelengths;
		
		spectr->closeDevice(id, &error);
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
	return spectrum;
}
Spectrometer::TRIGGER_MODE Spectrometer::getTriggerMode() {
	return triggerMode;
}
unsigned long Spectrometer::getIntegrationTimeMicros() {
	return integrationTimeMicros;
}
unsigned int Spectrometer::getScansToAverage() const{
	return scansToAverage;
}

void Spectrometer::setTriggerMode(TRIGGER_MODE m){
	int error = 0;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice(); 
		spectr->spectrometerSetTriggerMode(id, spectrometer_id, &error, m); if (error != 0) throw std::runtime_error("spectrometerSetTriggerMode - " + std::string(sbapi_get_error_string(error)));
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
		triggerMode = m;
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
}
void Spectrometer::setIntegrationTimeMicros(unsigned long integrationtime_us){
	int error = 0;
	
	if (id == -1){ // spectrometer has not been initialized
		init();
	}
	
	if (id != -1 && spectrometer_id != -1){
		openDevice();
		std::cout << "setting integrationtime to " << integrationtime_us << std::endl;
		spectr->spectrometerSetIntegrationTimeMicros(id, spectrometer_id, &error, integrationtime_us); if (error != 0) throw std::runtime_error("spectrometerSetIntegrationTimeMicros - " + std::string(sbapi_get_error_string(error)));
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}else{ //error
		throw std::runtime_error("no spectrometer found");
	}
}
void Spectrometer::save_spectrumCSV(const std::vector<DataP::DataP_ptr> &spectre, std::string path, double timediff){
	FSHelper::save_dataPToCsv(spectre, path, true, false, false, false, "wavelength", "intensity", "timediff=" + FSHelper::formatDouble(timediff)+ "s");
}
void Spectrometer::setScansToAverage(unsigned int scansToAverage){
	Spectrometer::scansToAverage = scansToAverage;
}

std::string Spectrometer::triggerModeToString(TRIGGER_MODE t){
	switch(t){
		case TRIGGER_MODE::TRIGGER_MODE_EXTERNAL_HARDWARE:
			return "external";
		case TRIGGER_MODE::TRIGGER_MODE_NORMAL:
			return "normal";
		case TRIGGER_MODE::TRIGGER_MODE_SOFTWARE:
			return "software";
		case TRIGGER_MODE::TRIGGER_MODE_SYNCHRONIZATION:
			return "synchronization";
	}
}

void Spectrometer::init(){
	int error = 0;
	
	spectr->probeDevices();
	int noOfDevices = spectr->getNumberOfDeviceIDs();
	if (noOfDevices == 0){
//		std::cout << "no spectrometer found" << std::endl;
		id = -1;
	}else{
		spectr->getDeviceIDs(&id, 1);
		
		char deviceTypeBuffer[80];
		spectr->getDeviceType(id, &error, deviceTypeBuffer, 79); if (error != 0) throw std::runtime_error("getDeviceType - " + std::string(sbapi_get_error_string(error)));
		device_type = deviceTypeBuffer;
		
		spectr->openDevice(id, &error); if (error != 0) throw std::runtime_error("openDevice - " + std::string(sbapi_get_error_string(error)));
		
		int numberOfSpectrometerFeatures = spectr->getNumberOfSpectrometerFeatures(id, &error); if (error != 0) throw std::runtime_error("getNumberOfSpectrometerFeatures - " + std::string(sbapi_get_error_string(error)));
		
		if (numberOfSpectrometerFeatures == 0){ // no spectrometer found
			spectrometer_id = -1;
			throw std::runtime_error("no spectrometer found");
		}else{
			spectr->getSpectrometerFeatures(id, &error, &spectrometer_id, 1); if (error != 0) throw std::runtime_error("getSpectrometerFeatures - " + std::string(sbapi_get_error_string(error)));
			
			//integration time
			integrationTimeMicros = spectr->spectrometerGetMinimumIntegrationTimeMicros(id, spectrometer_id, &error); if (error != 0) throw std::runtime_error("spectrometerGetMinimumIntegrationTimeMicros - " + std::string(sbapi_get_error_string(error)));
			spectr->spectrometerSetIntegrationTimeMicros(id, spectrometer_id, &error, integrationTimeMicros); if (error != 0) throw std::runtime_error("spectrometerSetIntegrationTimeMicros - " + std::string(sbapi_get_error_string(error)));
			
			//trigger mode
			triggerMode = TRIGGER_MODE::TRIGGER_MODE_NORMAL;
			spectr->spectrometerSetTriggerMode(id, spectrometer_id, &error, triggerMode); if (error != 0) throw std::runtime_error("spectrometerSetTriggerMode - " + std::string(sbapi_get_error_string(error)));
		}
		
		spectr->closeDevice(id, &error); if (error != 0) throw std::runtime_error("closeDevice - " + std::string(sbapi_get_error_string(error)));
	}
}

void Spectrometer::openDevice(){
	int error = 0;
	
	if (id == -1){ // no spectrometer found in the past
		init(); // try to find a device
	}
	
	if (id != -1){ // found a device in the past
		spectr->openDevice(id, &error); // try to open device
		if (error != 0){ // error occurred
			if (error == 2){ //device not connected
				id = -1;
				init(); // try to connect to device
			}
			if (id != -1){ // found a device during init()
				spectr->openDevice(id, &error); // try to open device second time
				if (error != 0){ // error happened during open
					throw std::runtime_error("openDevice - " + std::string(sbapi_get_error_string(error)));
				}
			}else{ // found no device during init()
				throw std::runtime_error("no spectrometer connected");
			}
		}
	}else{
		throw std::runtime_error("no spectrometer connected");
	}
	
}


// !! spectr needs to be opened and closed outside this function !!
std::vector<double> Spectrometer::getSpectrum(){
	int error = 0;
	std::vector<double> spectrum;
	int formattedSpectrumLength = spectr->spectrometerGetFormattedSpectrumLength(id, spectrometer_id, &error);  if (error != 0) throw std::runtime_error("spectrometerGetFormattedSpectrumLength - " + std::string(sbapi_get_error_string(error)));
	double *formattedSpectrum = new double[formattedSpectrumLength];
	
	//init spectrum with zeros
	for (int i = 0; i < formattedSpectrumLength; i++){
		spectrum.push_back(0);
	}
	
	for (int i = 0; i < scansToAverage; i++){
		//request spectrum
		spectr->spectrometerGetFormattedSpectrum(id, spectrometer_id, &error, formattedSpectrum, formattedSpectrumLength); if (error != 0) throw std::runtime_error("spectrometerGetFormattedSpectrum - " + std::string(sbapi_get_error_string(error)));
		
		
		//add the recieved values to the existing ones
		for (int j = 0; j < formattedSpectrumLength; j++){
			spectrum[j] += formattedSpectrum[j];
		}
	}
	
	//scale values
	for (int i = 0; i < formattedSpectrumLength; i++){
		spectrum[i] /= scansToAverage;
	}
	delete [] formattedSpectrum;
	
	return spectrum;
}
bool Spectrometer::isConnected() const{
	return id != -1;
}
std::string Spectrometer::to_string(){
	std::string retStr = "";
	retStr += "int. time: " + FSHelper::formatDouble(integrationTimeMicros) + "us";
	retStr += " - scans: " + FSHelper::formatDouble(scansToAverage);
	return retStr;
}







