#pragma once
/**
 * @file ImpAnalyser.h
 * 
 * @class ImpAnalyser
 * @author Nils Bosbach
 * @date 03.06.2019
 * @brief Defines the interface of an impedance analyser. Abstract class, device specific functions must be implemented 
 * by derived class
 */

#include "DataP.h"

#include <vector>
#include <memory>
#include <tinyxml2.h>

class ImpAnalyser {
public:
	typedef std::shared_ptr<ImpAnalyser> ImpAnalyser_ptr;
	
	enum WIRE_MODE {TWO_WIRE=2, THREE_WIRE=3, FOUR_WIRE=4};
	enum ANALYSER_DEVICE {ANALYSER_NOVOCONTROL, ANALYSER_HP4294A, ANALYSER_EMPICO, ANALYSER_DUMMY};
	
	ImpAnalyser();
	
	/**
	 * @brief getter for the start frequecy of the meausrement
	 * @return start frequency of the meausrement in Hz
	 */
	int getStartFrequency() const;
	
	/**
	 * @brief getter for the stop frequecy of the meausrement
	 * @return stop frequency of the meausrement in Hz
	 */
	int getStopFrequency() const;
	
	/**
	 * @brief getter for the voltage of the meausrement
	 * @return voltage of the meausrement in V
	 */
	double getVoltage() const;
	
	/**
	 * @brief getter for the number of points that are meausred
	 * @return number of points that will be measured
	 */
	int getPoints() const;
	
	/**
	 * @brief get the value of how many measurements are averaged for each frequency
	 * @return number of measurements that are done for each frequency
	 */
	int getPointAverage() const;
	
	
	/**
	 * @brief method to get the name of the impedance analyser (specified by derived class)
	 * @return name of the impedance analyser
	 */
	virtual std::string getType() const = 0;
	
	
	/**
	 * @brief get the type of the analyser
	 * @return the type of the analyser
	 */
	virtual ANALYSER_DEVICE getAnalyserType() const = 0;
	
	
	/**
	 * @brief set the start frequency of the measurement
	 * @param staFreq start frequncy in Hz
	 *
	 * if staFreq is smaller than the return value of getMinFreq(), the start frequency will be set to
	 * the return value of getMinFreq().
	 * if staFreq is greater than the return value of getMaxFreq(), the start frequency will be set to
	 * the retun value of getMaxFreq().
	 */
	void setStartFrequency(int staFreq);
	
	/**
	 * @brief set the stop frequency of the measurement
	 * @param stoFreq stop frequncy in Hz
	 *
	 * if stoFreq is smaller than the return value of getMinFreq(), the stop frequency will be set to
	 * the return value of getMinFreq().
	 * if stoFreq is greater than the return value of getMaxFreq(), the stop frequency will be set to
	 * the retun value of getMaxFreq().
	 */
	void setStopFrequency(int stoFreq);
	
	/**
	 * @brief set the voltage which will be used by the impedance analyser during the measurement
	 * @param volt voltage in V
	 *
	 * if volt is smaller than the return value of getMinVolt(), the voltage will be set to
	 * the return value of getMinVolt().
	 * if volt is greater than the return value of getMaxVolt(), the voltage will be set to
	 * the retun value of getMaxVolt().
	 */
	void setVoltage(double volt);
	
	/**
	 * @brief set the voltage which will be used by the impedance analyser during the measurement
	 * @param volt voltage in V
	 *
	 * if volt is smaller than the return value of getMinVolt(), the voltage will be set to
	 * the return value of getMinVolt().
	 * if volt is greater than the return value of getMaxVolt(), the voltage will be set to
	 * the retun value of getMaxVolt().
	 */
	void setPoints(int points);
	
	/**
	 * @brief set the number of measurement which are averaged and done for each frequency
	 * @param pointAv number of meausrements for each frequency
	 
	 */
	void setPointAverage(int pointAv);
	
	/**
	 * @brief getter for the wire mode variable
	 * @return the value of the wire mode variable
	 */
	WIRE_MODE getWireMode();
	
	/**
	 * @brief set the wire mode variable, value will be set to the spectrometer in applyParams / execute function
	 * @param w wire mode
	 */
	void setWireMode(WIRE_MODE w);
	
	/**
	 * @brief the method triggers the impedance analyser to measure an impedance spectrum and requests the meausred data
	 * @param running used by Task during the execute function to stop the execution of a recipe
	 * @return the spectrum which was captured by the impedance analyser
	 * @see Task
	 */
	virtual std::vector<DataP::DataP_ptr> measureSpectrum(bool* running = new bool(true)) = 0;
	
	/**
	 * @brief used to save the params in a xml document
	 * @param doc pointer to the document where the impedance analyser will be safed
	 * @param extern elements determins whether links to other files are allowed
	 * @return the created element containing the params
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) = 0;
	
	/**
	 * @brief saves a spectrum a csv file
	 * @param spectrum the spectrum which should be saved
	 * @param path defines where the file should be stored
	 * @param position used for the position if spectrum is part of a transient spectrum, -1 otherwise
	 * @param timediff used for the timedifference to the first spectrum if spectrum is part of a transient spectrum, -1 otherwise
	 */
	static void save_spectrumCSV(const std::vector<DataP::DataP_ptr> &spectrum, std::string path, int position = -1, double timediff = -1);
	
	
	/**
	 * @brief creates a string containing the basic parameters
	 * @return string containing the basic parameters
	 */
	virtual std::string to_string() const;
	
	
	/**
	 * @brief get the mininal frequency the analyser is able to measure
	 * @return mininal frequency the analyser is able to measure
	 */
	virtual int getMinFreq() const = 0;
	
	/**
	 * @brief get the maximal frequency the analyser is able to measure
	 * @return maximal frequency the analyser is able to measure
	 */
	virtual int getMaxFreq() const = 0;
	
	/**
	 * @brief get the minimal number of points the analyser is able to measure
	 * @return minimal number of points the analyser is able to measure
	 */
	virtual int getMinPoints() const = 0;
	
	/**
	 * @brief get the maximal number of points the analyser is able to measure
	 * @return maximal number of points the analyser is able to measure
	 */
	virtual int getMaxPoints() const = 0;
	
	/**
	 * @brief get the minimal voltage the analyser can use for the measurement
	 * @return minimal voltage the analyser can use for the measurement
	 */
	virtual double getMinVolt() const = 0;
	
	/**
	 * @brief get the maximal voltage the analyser can use for the measurement
	 * @return maximal voltage the analyser can use for the measurement
	 */
	virtual double getMaxVolt() const = 0;
	
	/**
	 * @brief get the maximal number of measurements the analyser average to one meausrement
	 * @return maximal number of measurements the analyser average to one meausrement
	 */
	virtual int getMaxPointAverage() const = 0;
	
	/**
	 * @brief get if the used analyser is internal or external (used to correctly switch relais)
	 * @return true, if the analyser is internal
	 */
	virtual bool getInternal() const = 0;
	
protected:
	double voltage;
	int startFrequency;
	int stopFrequency;
	int points;
	int pointAverage;
	WIRE_MODE wire_mode;
};
