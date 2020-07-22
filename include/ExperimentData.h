#pragma once
/**
 * @file ExperimentData.h
 * 
 * @class ExperimentData
 * @author Nils Bosbach
 * @date 16.05.2019
 * @brief the class stores data which is caputred during the execution of an experiment. The data will be stored
 * in the objects during the execute function of the Tasks. The class is used to save all the captured data
 * in the experiment folder and make the data accessible after the execution of an experiment.
 */
#include "DataP.h"
#include "Logbook.h"
#include "TransSpect.h"

#include <vector>
#include <mutex>
#include <memory>
#include <gtkmm.h>
#include <sigc++/sigc++.h>
#include <chrono>

class ExperimentData{
public:
	typedef std::shared_ptr<ExperimentData> ExperimentData_ptr;
	typedef std::chrono::system_clock::time_point TimePoint;
	
	/**
	 * @brief sets the name of the experiment and the path of the project
	 * @param experimentName used to create a folder with that name inside the project folder where the captured data will be saved
	 * @param projectPath path of the project folder
	 */
	ExperimentData(std::string experimentName, std::string projectPath);
	virtual ~ExperimentData();
	
	/**
	 * @brief adds a captured Spectrum to the spectrums vector and saves the Spectrum in the experiment folder
	 * @param spectrum the spectrum which should be added
	 *
	 * called by SpectrometerTask during the execute function. 
	 */
	void addSpectrum(std::vector<DataP::DataP_ptr> spectrum);
	
	/**
	 * @brief adds a captured Impedance Spectrum to the impedanceMeasurements vector and saves the Impedance Spectrum in the experiment folder
	 * @param spectrum the impedance spectrum which should be added
	 *
	 * called by HP4294ATask / NovocontrolTask during the execute function. 
	 */
	void addImpedanceSpectrum(std::vector<DataP::DataP_ptr> spectrum);
	
	/**
	 * @brief add a transient impedance spectrum. on_Spec_added_listener is connected to onTransImpSpecAdded, so onTransImpSpecAdded will be
	 * called each time a spectrum is added to the transient spectrum
	 * @param transSpectrum the spectrum which should be added
	 *
	 * this function is called once for each transient spectrum. A transient spectrum contains multiple spectrums. Each time a spectrum is
	 * added to the transient spectrum, not this method is called but onTransImpSpecAdded.
	 */
	void addTransImpedanceSpectrum(TransSpect::TransSpect_ptr transSpectrum);
	
	/**
	 * @brief get the last added Spectrum
	 * @return the last element of the spectrums vector, nullptr if vector is empty
	 */
	std::vector<DataP::DataP_ptr> getLastSpectreDataPoints();
	
	/**
	 * @brief get the last added Impedance spectrum
	 * @return the last element of the measured impedance vector, nullptr if vector is empty
	 */
	std::vector<DataP::DataP_ptr> getLastImpedanceDataPoints();
	
	/**
	 * @brief check if there was a Spectrum added 
	 * @return true if the spectrums vector is not empty
	 */
	bool capturedSpectrum() const;
	
	/**
	 * @brief check if there was an impedance spectrum added 
	 * @return true if the impedanceMeasurements vector is not empty
	 */
	bool capturedImpedance() const;
	
	/**
	 * @brief get the path where the captured video should be stored
	 * @return the path in the experiment folder where the captured video will be stored
	 */
	std::string getVideoPath() const;
	
	/**
	 * @brief saves  the logfile of the experiment in the experiment folder
	 */
	void saveLogfile();
	
	/**
	 * @brief get the path where the recipe should be stored in the project folder
	 * @return path of the recipe
	 */
	std::string getRecipePath() const;
	
	/**
	 * @brief get the path where Measurements are stored <Project_Folder>/<Experiment_Folder>/<Measurement_Folder> - if the folder has not been created, yet it will be created
	 * @return path of the Measurement Folder
	 */
	std::string getMeasurementsFolderPath();
	
	/**
	 * @brief sets an extern method which will be called each time a spectrum is added to a transient spectrum (can be used to update the gui)
	 * @param listener the method which should be called when a spectrum is added to a transient spectrum
	 */
	void connectOnTransImpSpecAddedListener(sigc::slot<void, TransSpect::TransSpect_ptr, TransSpect::Spectrum, unsigned int, double, std::string> listener);
	
	/**
	 * @brief get the elapsed seconds from the creation of this object until now
	 * @return senconds from creation until now
	 */
	double getElapsedSeconds();
	
	/// the temporary log which additionally used during the execution a recipe
	Logbook::Logbook_ptr log;
	
private:
	std::vector<std::vector<DataP::DataP_ptr>> spectrums;
	std::vector<TransSpect::TransSpect_ptr> transImpedanceMeasurements;
	std::vector<std::vector<DataP::DataP_ptr>> impedanceMeasurements;
	std::string projectPath;
	std::string experimentName;
	std::string experimentPath;
	std::mutex spectrumsMtx;
	std::mutex impSpectrumsMtx;
	std::mutex transImpSpectrumsMtx;
	TimePoint recipeStartTime;
	
	static sigc::slot<void, TransSpect::TransSpect_ptr, TransSpect::Spectrum,unsigned int, double, std::string> slot_onTransImpSpecAdded;
	static TimePoint getCurrentTime();
	static void onTransImpSpecAdded(TransSpect::TransSpect_ptr t, TransSpect::Spectrum s, unsigned int position, double timediff);
};
