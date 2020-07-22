#pragma once
/**
 * @file Novocontrol.h
 * 
 * @class Novocontrol
 * @author Nils Bosbach
 * @date 27.05.2019
 * @brief class to interface with a Novocontrol impedance analyser via gpib. Derives from class ImpAnalyser and implements the virtual methods
 */
#include "ImpAnalyser.h"
#include "DataP.h"
#include "GpibConnection.h"

#include <vector>

#define MIN_FREQ 0.000003
#define MAX_FREQ 40000000
#define MIN_VOLT 0.001
#define MAX_VOLT 5
#define MIN_POINTS 1
#define MAX_POINTS 1500
#define MAX_POINT_AVERAGE 256


class Novocontrol: public ImpAnalyser{
public:
	typedef std::shared_ptr<Novocontrol> Novocontrol_ptr;
	
	
	/**
	 * @brief opens a connection to the novocontrol impedance analyser with the specified gpib slave address
	 * @param slaveAddress the gpib address of the impedance analyser
	 */
	Novocontrol(int slaveAddress);
	virtual ~Novocontrol();
	
	/**
	 * @brief applys the params, triggers the meausrements of the points in logarithmic linear contribution and stores the captured data
	 * in a vector
	 * @param running needs to be true to keep the meausrement running - used to stop the execution when set to false
	 * @return the captured spectrum
	 */
	virtual std::vector<DataP::DataP_ptr> measureSpectrum(bool* running = new bool(true)) override;
	
	
	/**
	 * @brief used to save the params in a xml document
	 * @param doc pointer to the document where the impedance analyser will be safed
	 * @param extern elements determins whether links to other files are allowed
	 * @return the created element containing the params
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get the type of Impedance analyser
	 * @return "Novocontrol"
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief get the type of the analyser
	 * @return the type of the analyser
	 */
	virtual ImpAnalyser::ANALYSER_DEVICE getAnalyserType() const override;
	
	/**
	 * @brief creates an Novocontrol object from the data specified in a xml file
	 * @param task_elemmnt the part of a xml document which contain the information of the Novocontrol
	 */
	static Novocontrol_ptr loadNovocontrol(tinyxml2::XMLElement* task_element);
	
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
	GpibConnection connection;
	
	inline void applyParams();
	inline void sendOKCommand(std::string command);
	inline void applyAcVoltageAmplitude();
	inline void applyWireMode();
	DataP::DataP_ptr measureFreq(double freq);
};
