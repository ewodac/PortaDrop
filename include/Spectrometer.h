#pragma once
/**
 * @file Spectrometer.h
 * 
 * @class Spectrometer
 * @author Nils Bosbach
 * @date 05.08.2019
 * @brief implements the connection to the optical spectrometer
 */
#include "DataP.h"

#include <string>
#include <vector>
#include <api/seabreezeapi/SeaBreezeAPI.h>
#include <memory>

class Spectrometer{
public:
	
	/// smart pointer
	typedef std::shared_ptr<Spectrometer> Spectrometer_ptr;
	
	/// possible trigger modes
	enum TRIGGER_MODE {TRIGGER_MODE_NORMAL = 0, TRIGGER_MODE_SOFTWARE = 1, TRIGGER_MODE_SYNCHRONIZATION = 2, TRIGGER_MODE_EXTERNAL_HARDWARE = 3};
	
	/**
	 * @brief constructor
	 */
	Spectrometer();
	
	/**
	 * @brief destructor
	 */
	virtual ~Spectrometer();
	
	/**
	 * @brief create a new Spectrometer and return a smart pointer to the created object
	 * @return smart pointer to the created object
	 */
	static Spectrometer_ptr create();
	
	/**
	 * @brief request the highest possible intensity value from the spectrometer
	 * @return max intensity value
	 */
	double getMaxIntensity();
	
	/**
	 * @brief reuqest the mininal integration time in us from the spectrometer
	 * @return min inegration time [us]
	 */
	unsigned long getMinIntegrationTimeMicros();
	
	/**
	 * @brief reuqest the maximal integration time in us from the spectrometer
	 * @return max inegration time [us]
	 */
	unsigned long getMaxIntegrationTimeMicros();
	
	/**
	 * @brief request a spectrum from the spectrometer and return it as double array
	 * @return spectrum
	 */
	std::vector<double> getFormattedSpectrum();
	
	/**
	 * @brief request a spectrum from the spectrometer and return it as DataP array
	 * @return spectrum
	 */
	std::vector<DataP::DataP_ptr> getFormattedSpectrum_DataPoints();
	
	/**
	 * @brief request a spectrum from the spectrometer and return it as double array
	 * @return spectrum
	 */
	TRIGGER_MODE getTriggerMode();
	
	/**
	 * @brief request the current integration time from the spectrometer
	 * @return current integration time [us]
	 */
	unsigned long getIntegrationTimeMicros();
	
	/**
	 * @brief get the number of scans which are averaged for one meausrement
	 * @return number of scans which are averaged to one spectrum
	 */
	unsigned int getScansToAverage() const;
	
	/**
	 * @brief check, if the spectrometer is connected
	 * @return true, if the spectrometer can be accessed
	 */
	bool isConnected() const;
	
	/**
	 * @brief set the trigger mode of the spectrometer
	 * @param m trigger mode
	 */
	void setTriggerMode(TRIGGER_MODE m);
	
	/**
	 * @brief set the integration time of the spectrometer
	 * @param integrationtime_us integration time in us
	 */
	void setIntegrationTimeMicros(unsigned long integrationtime_us);
	
	/**
	 * @brief set the number of scans, which are averaged to one spectrum
	 * @param scansToAverage number of scans which are averaged to one spectrum
	 */
	void setScansToAverage(unsigned int scansToAverage);
	
	/**
	 * @brief get a string containing the current params
	 * @return a string conataining the current params
	 */
	std::string to_string();
	
	/**
	 * @brief save a spectrum as csv file
	 * @param spectre the sprectrum which should be saved
	 * @param path the full path of the destination of the spectrum
	 * @param timediff the eleapsed seconds from the beginning of the current recipe
	 */
	static void save_spectrumCSV(const std::vector<DataP::DataP_ptr> &spectre, std::string path, double timediff);
	
	/**
	 * @brief convert a TRIGGER_MODE to string
	 * @param t TRIGGER_MODE which should be converted
	 * @return converted TRIGGER_MODE
	 */
	static std::string triggerModeToString(TRIGGER_MODE t);
	
private:
	SeaBreezeAPI* spectr;
	std::string device_type;
	long id;
	long spectrometer_id;
	
	TRIGGER_MODE triggerMode;
	unsigned long integrationTimeMicros;
	unsigned int scansToAverage;
	
	void init();
	void openDevice();
	
	std::vector<double> getSpectrum();
};
