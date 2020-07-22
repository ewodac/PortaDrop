#pragma once
/**
 * @file I2CFreqTask.h
 * 
 * @class I2CFreqTask
 * @author Nils Bosbach
 * @date 10.04.2019
 * @brief This class can be used to change the frequency of the attiiny45 frequency generator via i2c. The value will be transmitted during the
 * execute function.
 */
#include "I2CTask.h"
#include "Uc_Connection.h"

#include <string>
#include <pthread.h>
#include <memory>


class I2CFreqTask: public I2CTask{

public:
	
	///smart pointer to a I2CFreqTask object
	typedef std::shared_ptr<I2CFreqTask> I2CFreqTask_ptr;
	
	/**
	 * @brief constructor, opens the i2c connection and sets the internal freq variable
	 * @param freq frequency which will be transmitted during the execute function
	 */
	I2CFreqTask(unsigned int freq = 1000);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~I2CFreqTask();
	
	/**
	 * @brief creates a new I2CFreqTask and returns a  smart pointer to the object
	 * @param freq frequency which will be transmitted during the execute function
	 * @return smart pointer to the created object
	 */
	static I2CFreqTask_ptr create(unsigned int freq = 1000);
	
	/**
	 * @brief sets the relias (if nessecarry) and transmits the new frequency. If the freq is set to 0, the relais will be switched to disable the frequency generator. No frequency will be transmitted in that case
	 * @param data object which can be used to store captured data. Not used in this task_element
	 * @param executeNext varialbe which is used to stop the execution
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get the type of the task, 'I2CFreqTask'
	 * @return 'I2CFreqTask'
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief convert the params to a xml element which can be integrated in a xml file
	 * @param doc pointer to the document, which will contain the task
	 * @param externElements determines if the xml file can contain links to other files
	 * @return the created xml element
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief create a I2CFreqTask from data defined in a xml file and return a smart pointer to the created object
	 * @param task_element part of the xml file which contains the params of the I2CFreqTask
	 * @return smart pointer to the created object
	 */
	static I2CFreqTask_ptr loadFreqTask(tinyxml2::XMLElement* task_element);
	
	/**
	 * @brief sets the internal frequency variable, the value will not be transmittet until execute is called
	 * @param f frequency [Hz], which should be transmittet during execute - 0 turns off the generator
	 */
	void setFrequency(unsigned int f);
	
	/**
	 * @brief returns the value of the internal frequency variable (which does not have to be the current value of the attiny45)
	 * @return current internal frequency value
	 */
	unsigned int getFrequency() const;
	
	/**
	 * @brief check whether a attiny45 frequency generator is connected via i2c
	 * @return true if a frequency generator is connected
	 */
	bool isConnected();
	
	/**
	 * @brief read the current frequency from the frequency generator
	 * @return the current frequency [Hz]
	 */
	int readFreq();
	
	/**
	 * @brief get the frequency as string
	 * @return formatted frequency
	 */
	std::string to_string() const;
	
private:
	unsigned int freq;
};
