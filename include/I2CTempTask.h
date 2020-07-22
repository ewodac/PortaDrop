#pragma once 
/**
 * @file I2CTempTask.h
 * 
 * @class I2CTask
 * @author Nils Bosbach
 * @date 10.04.2019
 * @brief This class implemnets the protocol to read the DHT11 data from atmega32 via I2C. The method execute requests the data and stores it in the class variable data, which can be read via getTempData
 */
#include "I2CTask.h"
#include "Uc_Connection.h"
#include "TempData.h"

#include <string>
#include <pthread.h>


class I2CTempTask: public I2CTask{

public:
	
	/**
	 * @brief creates a Uc_Connection object to communicate with the atmega32 via i2c
	 * @param slaveAddress the i2c slave address of the atmega32
	 */
	I2CTempTask(int slaveAddress);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~I2CTempTask();
	
	/**
	 * @brief triggers the atmega32 to request a measurement from the DHT11 and request the mesaurement. It can be accessed via the getTempData function
	 * @param data object where captured data can be stored - not used in this I2CTempTask
	 * @param executeNext used to stop a running experiment if value is false
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get the kind of task as string
	 * @return 'I2CTempTask'
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief get last last captured measurement
	 * @return the last temperature / humidity meausurement which has been caputured during the execute function
	 */
	TempData::TempData_ptr getTempData() const; // return the last measurement ( execute has to be call previously)
	
	/**
	 * @brief saves the task as part of a xml file
	 * @param doc the xml document which should contain the task
	 * @param externElements if true, links to other files are allowed
	 * @return the part of the xml file
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
private:
	TempData::TempData_ptr data; // pointer to the last measurement
	bool measurementValid;
};
