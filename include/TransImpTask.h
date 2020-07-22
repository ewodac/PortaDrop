#pragma once
/**
 * @file TransImpTask.h
 * 
 * @class TransImpTask
 * @author Nils Bosbach
 * @date 2019-06-03
 * @brief Task which caputures a transient impedance spectrum, which consists of multiple impedance spectrums
 */
#include "Task.h"
#include "ImpAnalyser.h"

#include <memory>
#include <string>

class TransImpTask: public Task{
public:
	
	/// smart poitner to a TransImpTask object
	typedef std::shared_ptr<TransImpTask> TransImpTask_ptr;
	
	
	/// kinds of possible termination modes -> stop after a certain time (in [s]) or after a certain number of caputured spectrums
	enum TERMINATION_MODE {TERM_TIME, TERM_CNT};
	
	/**
	 * @brief constructor, initializes the internal variables
	 * @param analyser the impedance analyser which should be used to caputure the spectrum
	 * @param termination the termination value (time or number of spectrums) - the kind of value is determined by the termination mode
	 * @param t termination mode - determines if the termination value is a time or the number of spectrums
	 */
	TransImpTask(ImpAnalyser::ImpAnalyser_ptr analyser, unsigned int termination = 20, TERMINATION_MODE t = TERMINATION_MODE::TERM_CNT);
	
	/**
	 * @brief caputures the spectrums and stores them in the ExperimentData object
	 * @param data used to store caputured data - spectrums are stored here
	 * @param executeNext used to stop the task during execution
	 */
	void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get the type of the task as string
	 * @return 'TransImpTask'
	 */
	std::string getType() const override;
	
	/**
	 * @brief saves the task as part of a xml file
	 * @param doc the xml document which should contain the task
	 * @param externElements if true, links to other files are allowed
	 * @return the part of the xml file
	 */
	tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false);
	
	/**
	 * @brief create a TransImpTask from data defined in a xml file and return a smart pointer to the created object
	 * @param task_element part of the xml file which contains the params of the TransImpTask
	 * @return smart pointer to the created object
	 */
	static TransImpTask_ptr loadTransImpTask(tinyxml2::XMLElement* task_element);
	
	/**
	 * @brief set the termination value
	 * @param value the termination value (time or number of spectrums) - the kind of value is determined by the termination mode
	 * @param mode termination mode - determines if the termination value is a time or the number of spectrums
	 */
	void setTermination(unsigned int value, TERMINATION_MODE mode);
	
	/**
	 * @brief get the termination number
	 * @return termination number - if the number represents a time or the number of spectrums can be figured out by getTerminationMode
	 */
	const unsigned int getTerminationNumber() const;
	
	/**
	 * @brief get the termination mode
	 * @return the termination mode
	 */
	const TERMINATION_MODE getTerminationMode() const;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief get parameters as string
	 * @return string containing the parameters
	 */
	std::string to_string() const;
	
private:
	ImpAnalyser::ImpAnalyser_ptr analyser;
	unsigned int termination; // seconds or measurement number (specified at termMode)
	TERMINATION_MODE termMode;
};
