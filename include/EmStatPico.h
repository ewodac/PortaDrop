#pragma once
/**
 * @file EmStatPico.h
 * 
 * @class EmStatPico
 * @author Nils Bosbach
 * @date 23.05.2019
 * @brief class to interface with the internal impedance analyser EmStat pico
 */
#include "ImpAnalyser.h"
#include "DataP.h"
#include "MeasurementPackage.h"

#include <vector>
#include <memory>
#include <list>

#define TIMEOUT_S 10

class EmStatPico: public ImpAnalyser{
public:
	typedef std::shared_ptr<EmStatPico> EmStatPico_ptr;
	typedef std::list<MeasurementPackage::MeasurementPackage_ptr> MeasurementList;
	typedef std::shared_ptr<MeasurementList> MeasurementList_ptr;
	
	
	EmStatPico();
	virtual ~EmStatPico();
	
	
	/**
	 * @brief applys the params, triggers the meausrements of the points in logarithmic linear contribution and stores the captured data
	 * in a vector
	 * @param running needs to be true to keep the meausrement running - used to stop the execution when set to false
	 * @return the captured spectrum
	 */
	virtual std::vector<DataP::DataP_ptr> measureSpectrum(bool* running = new bool(true)) override;
	
	/**
	 * @brief get the type of Impedance analyser
	 * @return "EmStatPico"
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief used to save the params in a xml document
	 * @param doc pointer to the document where the impedance analyser will be safed
	 * @param extern elements determins whether links to other files are allowed
	 * @return the created element containing the params
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get the type of the analyser
	 * @return the type of the analyser
	 */
	virtual ImpAnalyser::ANALYSER_DEVICE getAnalyserType() const override;
	
	/**
	 * @brief creates an EmStatPico object from the data specified in a xml file
	 * @param task_elemmnt the part of a xml document which contain the information of the Novocontrol
	 */
	static EmStatPico_ptr loadEmStatPico(tinyxml2::XMLElement* task_element);
	
	
	/**
	 * @brief get basic params as string
	 * @return  basic params as string
	 */
	virtual std::string to_string() const override;
	
	virtual int getMinFreq() const override;
	virtual int getMaxFreq() const override;
	virtual int getMinPoints() const override;
	virtual int getMaxPoints() const override;
	virtual int getMaxPointAverage() const override;
	virtual double getMinVolt() const override;
	virtual double getMaxVolt() const override;
	virtual bool getInternal() const override;
	
private:
	static std::string formatNumber(double i);
	
	void startMeasurement(int fd, double f_min, double f_max, double current_min = 10e-9, double current_max = 10e-3);
	MeasurementList_ptr receiveMeasurements(int fd);
	bool waitForSerialData(int fd); // false -> timeout reached
	static int getNextCurrentRange(int current_range);
	static int getPrevCurrentRange(int current_range);
};
