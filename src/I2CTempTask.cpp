#include "I2CTempTask.h"
#include "TempData.h"
#include "Addresses.h"

#include <iostream>
#include <wiringPi.h>
#include <iomanip> // std::setprecision
#include <unistd.h>

#define TIMEOUT_US 500

I2CTempTask::I2CTempTask(int slaveAddress): I2CTask(slaveAddress){
	data = nullptr;  // no data read so far
}
I2CTempTask::~I2CTempTask(){
	
}

/*
* Triggers a data read from DHT11 on atmega32, receives the measured data ad stors it in class
* variable data
*/
void I2CTempTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	usleep(2 * 1000 * 1000);
	std:: cout << "write: " << connection->writeRegister(I2C_ATMEGA32_BUFFER_TEMP_REQUEST, 0x01) << std::endl; //trigger measurement
	usleep(50 * 1000); //wait until data is ready
	
	int status;
	int timeout = 0;
	do{
		status = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_READY);
		timeout += 10;
		usleep(10);
	}while((status == 0) && (timeout < TIMEOUT_US) && (*executeNext));
	
	if (*executeNext){
		std::cout << "status: " << status << std::endl;
		int temp_data[5];
		
		temp_data[0] = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_DATA0);
		temp_data[1] = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_DATA1);
		temp_data[2] = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_DATA2);
		temp_data[3] = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_DATA3);
		temp_data[4] = connection->readRegister(I2C_ATMEGA32_BUFFER_TEMP_DATA4);
		if (status == 0x01){ // data valid
			I2CTempTask::data = std::make_shared<TempData>(temp_data);
			measurementValid = true;
			addLogEvent(Log_Event::create("measurement valid", "received valid temperature measurement - " + std::to_string(I2CTempTask::data->getTemperature()) + "°C, " + std::to_string(I2CTempTask::data->getHumidity()) + "%" , Log_Event::TYPE::LOG_INFO));
		}else{
			measurementValid = false;
			addLogEvent(Log_Event::create("measurement unvalid", "received unvalid temperature measurement", Log_Event::TYPE::LOG_ERROR));
		}
	}
}

/*
* returns the pointer to the data of the last measurement. If it returns nullptr, the last 
* measurement failed or execute has not been called
*/
TempData::TempData_ptr I2CTempTask::getTempData() const{
	return I2CTempTask::data;
}

std::string I2CTempTask::getType() const{
	return "I2CTempTask";
}

tinyxml2::XMLElement* I2CTempTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("PadTask");
	xmlTaskElement->SetAttribute("name", name.c_str());
	xmlTaskElement->SetAttribute("comment", comment.c_str());
	xmlTaskElement->SetAttribute("slaveAddress", connection->getSlaveAddress());
	
	return xmlTaskElement;
}

std::list<Task::DEVICES> I2CTempTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	Task::DEVICES d = Task::DEVICES::DEVICE_ATMEGA_GENERAL;
	devices.push_back(d);
	
	return devices;
}