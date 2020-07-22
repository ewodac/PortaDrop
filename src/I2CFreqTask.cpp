#include "I2CFreqTask.h"
#include "Addresses.h"

#include <wiringPi.h>
#include <iostream>

I2CFreqTask::I2CFreqTask(unsigned int freq): I2CTask(I2C_ATTINY45_FREQ_SLAVE_ADDRESS){
	setFrequency(freq);
}

I2CFreqTask::~I2CFreqTask(){
	
}

I2CFreqTask::I2CFreqTask_ptr I2CFreqTask::create(unsigned int freq){
	return std::make_shared<I2CFreqTask>(freq);
}

void I2CFreqTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	if (isConnected()){
		if (freq == 0){
			if (relais->uc_is_connected()){
				relais->readRelais();
				if (relais->getRelais(Relais::RELAIS::R_STEUER_AC)){
					relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
					relais->writeRelais();
					addLogEvent(Log_Event::create("switch relais", "switched relais to disable AC output", Log_Event::TYPE::LOG_INFO));
				}
			}else{
				addLogEvent(Log_Event::create("switch relais", "can't send i2c command to the relais uc", Log_Event::TYPE::LOG_ERROR));
				throw std::runtime_error("microcontroller is not connected - cannot switch relais to disable frequency generator"); 
			}
		}else{
			addLogEvent(Log_Event::create("set frequency", "f=" + std::to_string(freq) + "Hz", Log_Event::TYPE::LOG_INFO));
			
	//		std::cout << "freq: " << freq << "Hz" << std::endl;
			
			char freq_0 =  (0x000000FF & freq);
			char freq_1 = ((0x0000FF00 & freq) >> 8);
			char freq_2 = ((0x00FF0000 & freq) >> 16);
			char freq_3 = ((0xFF000000 & freq) >> 24);
			
	//		std::cout << std::bitset<8>(freq_3) << " - " << std::bitset<8>(freq_2) << " - " << std::bitset<8>(freq_1) << " - " << std::bitset<8>(freq_0) << std::endl;
			
			bool error = false;
			if (!error) error = (connection->writeRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ0, freq_0) != false);
			if (!error) error = (connection->writeRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ1, freq_1) != false);
			if (!error) error = (connection->writeRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ2, freq_2) != false);
			if (!error) error = (connection->writeRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ3, freq_3) != false);
			
			if (error){
				addLogEvent(Log_Event::create("set frequency", "can't send i2c command to frequency generator", Log_Event::TYPE::LOG_ERROR));
			}else{
				if (relais->uc_is_connected()){
					relais->readRelais();
					if (!relais->getRelais(Relais::RELAIS::R_STEUER_AC)){
						relais->setRelais(Relais::RELAIS::R_STEUER_AC, true);
						relais->writeRelais();
						addLogEvent(Log_Event::create("switch relais", "switched relais to enable AC output", Log_Event::TYPE::LOG_INFO));
					}
				}
			}
		}
	}else{
		throw std::runtime_error("microcontroller is not connected - cannot set the frequency");
	}
	executed = true;
}

void I2CFreqTask::setFrequency(unsigned int f){
	I2CFreqTask::freq = f;
	setName(to_string());
}

// returns the value of the class variable freq
unsigned int I2CFreqTask::getFrequency() const{
	return I2CFreqTask::freq;
}

std::string I2CFreqTask::getType() const{
	return "I2CFreqTask";
}

tinyxml2::XMLElement* I2CFreqTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("FreqTask");
	xmlTaskElement->SetAttribute("name", name.c_str());
	xmlTaskElement->SetAttribute("comment", comment.c_str());
	xmlTaskElement->SetAttribute("freq", freq);
	
	return xmlTaskElement;
}
I2CFreqTask::I2CFreqTask_ptr I2CFreqTask::loadFreqTask(tinyxml2::XMLElement* task_element){
	I2CFreqTask_ptr p;
	if (task_element->FindAttribute("freq") != nullptr){
			p = create(task_element->FindAttribute("freq")->IntValue());
	}
	return p;
}

bool I2CFreqTask::isConnected(){
	return (connection->readRegister(I2C_ATTINY45_FREQ_BUFFER_ID) == I2C_ATTINY45_FREQ_SLAVE_ADDRESS);
}

int I2CFreqTask::readFreq(){
	int freq0 = connection->readRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ0);
	int freq1 = connection->readRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ1);
	int freq2 = connection->readRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ2);
	int freq3 = connection->readRegister(I2C_ATTINY45_FREQ_BUFFER_FREQ3);
	int freq = freq0 + (freq1 << 8) + (freq2 << 16) + (freq3 << 24);
	
	return freq;
}

std::string I2CFreqTask::to_string() const{
	return (freq == 0 ? "freq off" : "f = " + std::to_string(freq).append(" Hz"));
}

std::list<Task::DEVICES> I2CFreqTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	Task::DEVICES d = Task::DEVICES::DEVICE_ATTINY_FREQ;
	devices.push_back(d);
	
	d = Task::DEVICES::DEVICE_ATMEGA_GENERAL;
	devices.push_back(d);
	
	return devices;
}