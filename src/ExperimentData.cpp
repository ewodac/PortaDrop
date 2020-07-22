#include "ExperimentData.h"
#include "FSHelper.h"
#include "Spectrometer.h"
#include "ImpAnalyser.h"

#include <string>
#include <stdio.h>
#include <iostream>

#define MEASUREMENTS_FOLDER_NAME "measurements"
#define VIDEO_FOLDER_NAME "video"

sigc::slot<void, TransSpect::TransSpect_ptr, TransSpect::Spectrum, unsigned int, double, std::string> ExperimentData::slot_onTransImpSpecAdded;

ExperimentData::ExperimentData(std::string experimentName, std::string projectPath): experimentName(experimentName), projectPath(projectPath){
	log = Logbook::create();
	experimentPath = FSHelper::composePath(projectPath, FSHelper::getCurrentTimestamp() + "_" + experimentName);
	if (!FSHelper::folderExists(projectPath)){ // create Project folder
		FSHelper::createDirectory(projectPath);
	}
	FSHelper::createDirectory(experimentPath);
	
	recipeStartTime = getCurrentTime();
}
ExperimentData::~ExperimentData(){
	
}


void ExperimentData::addSpectrum(std::vector<DataP::DataP_ptr> spectrum){
	spectrumsMtx.lock();
	spectrums.push_back(spectrum);
	spectrumsMtx.unlock();
	
	std::string measurementsFolderPath = getMeasurementsFolderPath();
	
	Spectrometer::save_spectrumCSV(spectrum, FSHelper::getNextAvailablePath(FSHelper::composePath(measurementsFolderPath, std::string("opt_spectrum")), "csv", true), getElapsedSeconds());
}

void ExperimentData::addImpedanceSpectrum(std::vector<DataP::DataP_ptr> spectrum){
	impSpectrumsMtx.lock();
	impedanceMeasurements.push_back(spectrum);
	impSpectrumsMtx.unlock();
	
	std::string measurementsFolderPath = getMeasurementsFolderPath();
	
	ImpAnalyser::save_spectrumCSV(spectrum, FSHelper::getNextAvailablePath(FSHelper::composePath(measurementsFolderPath, std::string("imp_spectrum")), "csv", true), 0, getElapsedSeconds());
}

void ExperimentData::addTransImpedanceSpectrum(TransSpect::TransSpect_ptr spectrum){
	std::string base_path; 
	unsigned int appendix = 0;
	std::string measurementsFolderPath = getMeasurementsFolderPath();
	
	do {
		base_path = FSHelper::composePath(measurementsFolderPath, "trans_imp_spectrum_") + std::to_string(appendix++); 
	}while (FSHelper::fileExists(base_path + "_0.csv"));
	spectrum->setBasePath(base_path);
	spectrum->setOn_Spec_added_listener(onTransImpSpecAdded);
	
//	std::cout << "ExperimentData::addTransImpedanceSpectrum -- lock" << std::endl;
	transImpSpectrumsMtx.lock();
//	std::cout << "ExperimentData::addTransImpedanceSpectrum -- got lock" << std::endl;
	transImpedanceMeasurements.push_back(spectrum);
	transImpSpectrumsMtx.unlock();
}

std::vector<DataP::DataP_ptr> ExperimentData::getLastSpectreDataPoints(){
	std::vector<DataP::DataP_ptr> lastSpectre;
	spectrumsMtx.lock();
	if (spectrums.size() != 0){
		lastSpectre = spectrums.back();
	}
	spectrumsMtx.unlock();
	
	return lastSpectre;
}

std::vector<DataP::DataP_ptr> ExperimentData::getLastImpedanceDataPoints(){
	std::vector<DataP::DataP_ptr> lastSpectre;
	impSpectrumsMtx.lock();
	if (impedanceMeasurements.size() != 0){
		lastSpectre = impedanceMeasurements.back();
	}
	impSpectrumsMtx.unlock();
	
	return lastSpectre;
}

bool ExperimentData::capturedSpectrum() const{
	return (spectrums.size() != 0);
}
bool ExperimentData::capturedImpedance() const{
	return (impedanceMeasurements.size() != 0);
}

std::string ExperimentData::getVideoPath() const{
	std::string videoFolderPath = FSHelper::composePath(experimentPath, VIDEO_FOLDER_NAME);
	if (!FSHelper::folderExists(videoFolderPath)){ // create Measurements folder
		FSHelper::createDirectory(videoFolderPath);
	}
	
	return FSHelper::composePath(videoFolderPath, "video.mpeg");
}

void ExperimentData::saveLogfile(){
	log->save_log_file(FSHelper::composePath(experimentPath, "logfile.log"));
}

std::string ExperimentData::getRecipePath() const{
	return FSHelper::composePath(experimentPath, "recipe.xml");
}

void ExperimentData::onTransImpSpecAdded(TransSpect::TransSpect_ptr t, TransSpect::Spectrum spectrum, unsigned int position, double timediff){
	std::string path = FSHelper::getNextAvailablePath(t->getBasePath(), "csv", true);
	ImpAnalyser::save_spectrumCSV(spectrum, path, position, timediff);
	
	if (!slot_onTransImpSpecAdded.empty()) slot_onTransImpSpecAdded(t, spectrum, position, timediff, path);
}

void ExperimentData::connectOnTransImpSpecAddedListener(sigc::slot<void, TransSpect::TransSpect_ptr, TransSpect::Spectrum, unsigned int, double, std::string> listener){
	slot_onTransImpSpecAdded = listener;
}

std::string ExperimentData::getMeasurementsFolderPath(){
	std::string measurementsFolderPath = FSHelper::composePath(experimentPath, MEASUREMENTS_FOLDER_NAME);
	
	if (!FSHelper::folderExists(measurementsFolderPath)){ // create Measurements folder
		FSHelper::createDirectory(measurementsFolderPath);
	}
	return measurementsFolderPath;
}

ExperimentData::TimePoint ExperimentData::getCurrentTime(){
	return std::chrono::system_clock::now();
}
double ExperimentData::getElapsedSeconds(){
	std::chrono::duration<double> elapsed_seconds = getCurrentTime() - recipeStartTime;
	
	return elapsed_seconds.count();
}