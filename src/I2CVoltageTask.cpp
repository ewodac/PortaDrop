#include "I2CVoltageTask.h"
#include "FSHelper.h"
#include "Addresses.h"

#include <wiringPi.h>
#include <iostream>
#include <unistd.h>

#define WAIT_FOR_VOLTAGE_TIMEOUT 40

DialogExtVolt* I2CVoltageTask::dialogExtVolt= nullptr;

I2CVoltageTask::I2CVoltageTask(unsigned int voltage, VOLTAGE_MODE m): I2CTask(I2C_ATTINY45_VOLT_SLAVE_ADDRESS), mode(m){
	setVoltage(voltage);
	waitForVoltage = true;
	dutyCycle = 0.5;
}

I2CVoltageTask::~I2CVoltageTask(){
	
}

I2CVoltageTask::I2CVoltageTask_ptr I2CVoltageTask::create(unsigned int voltage, VOLTAGE_MODE m){
	return std::make_shared<I2CVoltageTask>(voltage, m);
}

void I2CVoltageTask::setInternalVoltage(bool* executeNext){
	if (voltage > 0){ // voltage should be turned on
		if (isConnected()){
			
			if (mode == VOLTAGE_MODE::MODE_CONTROLLER){
				addLogEvent(Log_Event::create("set voltage", "v=" + std::to_string(voltage) + "V", Log_Event::TYPE::LOG_INFO));
				
				int ADCS = ((double) VOLTAGE_ATTINY45_R2) / (((double) VOLTAGE_ATTINY45_R1) + ((double) VOLTAGE_ATTINY45_R2)) * ((double) VOLTAGE_ATTINY45_R_CORR) * 1024.0 / 5.0 * ((double) voltage);
				
				char ADC_SL =  (0x000000FF & ADCS);
				char ADC_SH = ((0x0000FF00 & ADCS) >> 8);
				
				//std::cout << std::bitset<8>(ADC_SH) << " - " << std::bitset<8>(ADC_SL) << std::endl;
				int tries = 0;
				bool success = false;
				bool error = false;
				
				//try to set voltage
				do{
					if (!error) error = (connection->writeRegister(I2C_ATTINY45_VOLT_BUFFER_ADCL_S, ADC_SL) != false);
					if (!error) error = (connection->writeRegister(I2C_ATTINY45_VOLT_BUFFER_ADCH_S, ADC_SH) != false);
					if (!error) error = (connection->writeRegister(I2C_ATTINY45_VOLT_BUFFER_MODE, I2C_ATTINY45_VOLT_BUFFER_MODE_CONT) != false);
					
					usleep(1 * 1000);
					
					//check if controller accepted the voltage
					success = (connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCL_S) == ADC_SL && connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCH_S) == ADC_SH);
					tries++;
				}while (!success && tries < 3);
				
				if (!error && success){
					//set relais
					if (relais->uc_is_connected()){
						relais->readRelais();
						if (!relais->getRelais(Relais::RELAIS::R_STEUER_SAFETY)){
							relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, true);
							relais->setRelais(Relais::RELAIS::R_STEUER_BOOST_IN, true);
							relais->writeRelais();
							addLogEvent(Log_Event::create("relais switched", "relais switched to supply the boost converter with voltage", Log_Event::TYPE::LOG_INFO));
						}
						
						int timeout = 0;
						double current_voltage = readVoltage();
						TimePoint lastTimeWithRightVoltage = getCurrentTime();
						
						
						while(getElapsedSeconds(lastTimeWithRightVoltage) < VOLTAGE_TASK_TIME_CORRECT_VOLT && timeout < WAIT_FOR_VOLTAGE_TIMEOUT && *executeNext && waitForVoltage){
							if (current_voltage > voltage * 1.2){ // voltage is much higher than the setpoint
								
								relais->readRelais();
								if (relais->getRelais(Relais::RELAIS::R_STEUER_SAFETY)){ //if capacitor is not discharging at the moment
									addLogEvent(Log_Event::create("huge voltage difference", "current voltage (" + FSHelper::formatDouble(current_voltage) + "V) is too high - discharging capacitor" , Log_Event::TYPE::LOG_INFO));
									
									relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, false);
									relais->writeRelais(); // discharge
									
									int discharging_timeout = 0;
									//while (current_voltage > voltage * 1.2 && discharging_timeout < 60){
									while (current_voltage > voltage * 1 && discharging_timeout < 60){
										current_voltage = readVoltage();
										usleep(50 * 1000);
										discharging_timeout++;
									}
									
									relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, true);
									relais->writeRelais(); // stop discharging
								}
							}
							current_voltage = readVoltage();
							double e_rel = (current_voltage - (voltage)) / (voltage); // error
							e_rel = (e_rel > 0 ? e_rel : e_rel * -1.0); //abs
							
							usleep(500 * 1000);
							if (e_rel > 0.05){ //error bigger than 5%
								timeout++;
								lastTimeWithRightVoltage = getCurrentTime();
							}else{
								
							}
							std::cout << "e_rel: " << e_rel << "\tvoltag: " << current_voltage << "\ttimeout: " << timeout << std::endl;
							
						} 
						
						
						if (timeout >= WAIT_FOR_VOLTAGE_TIMEOUT){
							addLogEvent(Log_Event::create("timeout expired", "cannot set voltage to " + std::to_string(voltage) + "V - current voltage: " + FSHelper::formatDouble(current_voltage) +"V", Log_Event::TYPE::LOG_ERROR));
							throw std::runtime_error("timeout expired - cannot set voltage");
						}else{
							addLogEvent(Log_Event::create("voltage set", "current voltage: " + FSHelper::formatDouble(current_voltage) +"V", Log_Event::TYPE::LOG_INFO));
						}
					}else{
						addLogEvent(Log_Event::create("cannot set relais", "relais microcontroller is not connected", Log_Event::TYPE::LOG_ERROR));
						throw std::runtime_error("relais cannot be set - microcontroller is not connected"); 
					}
				}else{
					if (error){
						addLogEvent(Log_Event::create("set voltage", "can't send i2c command to voltage controller", Log_Event::TYPE::LOG_ERROR));
						throw std::runtime_error("can't send i2c command to voltage controller"); 
					}
					if(!success){
						addLogEvent(Log_Event::create("set voltage", "voltage cannot be set", Log_Event::TYPE::LOG_ERROR));
						throw std::runtime_error("voltage cannot be set"); 
					}
				}
			}else if (mode == VOLTAGE_MODE::MODE_DUTY_CYCLE){
				addLogEvent(Log_Event::create("set duty cycle", "D=" + FSHelper::formatDouble(dutyCycle*100) + "%", Log_Event::TYPE::LOG_INFO));
				
				bool error = false;
				int dutyCycle_int = 255.0 * dutyCycle;
				
				if (!error) error = (connection->writeRegister(I2C_ATTINY45_VOLT_BUFFER_MODE, I2C_ATTINY45_VOLT_BUFFER_MODE_DUTY) != false);
				if (!error) error = (connection->writeRegister(I2C_ATTINY45_VOLT_BUFFER_DUTY_CYCLE, dutyCycle_int) != false);
				
				if (!error){
					if (relais->uc_is_connected()){
						relais->readRelais();
						if (!relais->getRelais(Relais::RELAIS::R_STEUER_SAFETY)){
							relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, true);
							relais->writeRelais();
							addLogEvent(Log_Event::create("relais switched", "relais switched to supply the boost converter with voltage", Log_Event::TYPE::LOG_INFO));
						}
					}else{
						addLogEvent(Log_Event::create("cannot set relais", "relais microcontroller is not connected", Log_Event::TYPE::LOG_ERROR));
						throw std::runtime_error("relais cannot be set - microcontroller is not connected"); 
					}
				}else{
					addLogEvent(Log_Event::create("set duty cycle", "can't send i2c command to voltage controller", Log_Event::TYPE::LOG_ERROR));
					throw std::runtime_error("duty cycle cannot be set - microcontroller is not connected"); 
				}
			}
		}else{ // voltage controller is not connected
			throw std::runtime_error("voltage controller is not connected - cannot set voltage");
		}
		
	}else{ // voltage should be turned off
		if (relais->uc_is_connected()){
			relais->readRelais();
			if (relais->getRelais(Relais::RELAIS::R_STEUER_SAFETY)){
				relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, false);
				relais->writeRelais();
				addLogEvent(Log_Event::create("relais switched", "relais switched to not supply the boost converter with voltage", Log_Event::TYPE::LOG_INFO));
			}
		}else{
			throw std::runtime_error("microcontroller is not connected - cannot turn off the voltage by switching relais");
		}
	}
}
void I2CVoltageTask::setExternalVoltage(bool* executeNext){
	addLogEvent(Log_Event::create("external V source", "setpoint v=" + std::to_string(voltage) + "V", Log_Event::TYPE::LOG_INFO));
	Uc_Connection::Uc_Connection_ptr atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	
	
	if (atmega32->isConnected()){
		double ext_voltage = readExternalVoltage();
		double e_rel = (ext_voltage - ((double)voltage)) / ((double)voltage);
		e_rel = (e_rel > 0 ? e_rel : e_rel * -1.0); //abs
		
		addLogEvent(Log_Event::create("measured ext. voltage", "external voltage v=" + FSHelper::formatDouble(ext_voltage) + "V", Log_Event::TYPE::LOG_INFO));
		
		bool correctVoltage = false;
		
		if (e_rel < 0.05){ // relative error is smaller than 5 %
			correctVoltage = true;
		}else{ // relative error is bigger than 5 %
			if (dialogExtVolt != nullptr){ // got a dialogExtVolt reference
				dialogExtVolt->setSetpointVoltage(voltage);
				if (dialogExtVolt->show() == Gtk::RESPONSE_ACCEPT){
					correctVoltage = true;
				}
			}
		}
		
		if (correctVoltage){
			ext_voltage = readExternalVoltage();
			addLogEvent(Log_Event::create("external voltage", "external voltage: " + FSHelper::formatDouble(ext_voltage) + "V, (setpoint: " + std::to_string(voltage) + "V)", Log_Event::TYPE::LOG_INFO));
			
		}else{
			addLogEvent(Log_Event::create("wrong voltage", "wrong external voltage " + FSHelper::formatDouble(ext_voltage) + "V, needs to be " + std::to_string(voltage) + "V", Log_Event::TYPE::LOG_ERROR));
			throw std::runtime_error("wrong external voltage " + FSHelper::formatDouble(ext_voltage) + "V, needs to be " + std::to_string(voltage) + "V");
		}
		
	}else{ //atmega32 is not connected
		addLogEvent(Log_Event::create("atmeg32 not connected", "cannot set relais and cannot measure the external voltage", Log_Event::TYPE::LOG_ERROR));
		throw std::runtime_error("atmega32 not connected - relais cannot be set and ext voltage cannot be measured");
	}
}
void I2CVoltageTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	if (executeNext != 0 && *executeNext){
		if (relais->uc_is_connected()){
			
			//unswitch AC Relais
//			bool acRelaisSwitched;
//			relais->readRelais();
//			acRelaisSwitched = relais->getRelais(Relais::RELAIS::R_STEUER_AC);
			
//			if (acRelaisSwitched){
//				relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
//				relais->writeRelais();
//			}
			
			
			if (mode == VOLTAGE_MODE::MODE_EXTERN){ // external source
				relais->readRelais();
				if (relais->getRelais(Relais::RELAIS::R_STEUER_HV_EXT)){
					relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, false); // external source
					relais->writeRelais();
					addLogEvent(Log_Event::create("relais switched", "relais switched to connect the external HV source with the H-bridge", Log_Event::TYPE::LOG_INFO));
				}
				
				setExternalVoltage(executeNext);
			
			}else{ // internal source
				relais->readRelais();
				if (!relais->getRelais(Relais::RELAIS::R_STEUER_HV_EXT)){
					relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, true); // external source
					relais->writeRelais();
					addLogEvent(Log_Event::create("relais switched", "relais switched to connect the internal HV source with the H-bridge", Log_Event::TYPE::LOG_INFO));
				}
				
				setInternalVoltage(executeNext);
			}
			
//			if (acRelaisSwitched){
//				relais->setRelais(Relais::RELAIS::R_STEUER_AC, true);
//				relais->writeRelais();
//			}
		}else{
			addLogEvent(Log_Event::create("cannot set relais", "relais microcontroller is not connected", Log_Event::TYPE::LOG_ERROR));
			throw std::runtime_error("relais cannot be set - microcontroller is not connected"); 
		}
	}
	executed = true;
}

void I2CVoltageTask::setVoltage(unsigned int v){
	voltage = (v > MAX_OUT_VOLT_BOOST ? MAX_OUT_VOLT_BOOST : v);
	
	setName(to_string());
}


unsigned int I2CVoltageTask::getVoltage() const{
	return I2CVoltageTask::voltage;
}

std::string I2CVoltageTask::getType() const{
	return "I2CVoltageTask";
}

tinyxml2::XMLElement* I2CVoltageTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("VoltageTask");
	xmlTaskElement->SetAttribute("name", name.c_str());
	xmlTaskElement->SetAttribute("comment", comment.c_str());
	xmlTaskElement->SetAttribute("mode", mode);
	
	switch(mode){
		case VOLTAGE_MODE::MODE_CONTROLLER:
			xmlTaskElement->SetAttribute("volt", voltage);
			xmlTaskElement->SetAttribute("wait_for_voltage", waitForVoltage);
			break;
			
		case VOLTAGE_MODE::MODE_DUTY_CYCLE:
			xmlTaskElement->SetAttribute("duty_cycle", dutyCycle);
			break;
			
		case VOLTAGE_MODE::MODE_EXTERN:
			xmlTaskElement->SetAttribute("volt", voltage);
			break;
	}
	
	return xmlTaskElement;
}
I2CVoltageTask::I2CVoltageTask_ptr I2CVoltageTask::loadVoltageTask(tinyxml2::XMLElement* task_element){
	I2CVoltageTask_ptr p;
	if (task_element->FindAttribute("volt") != nullptr){
		p = create(task_element->FindAttribute("volt")->IntValue());
	}
	
	if (p != nullptr && task_element->FindAttribute("wait_for_voltage") != nullptr){
		p->setWaitForVoltage(task_element->FindAttribute("wait_for_voltage")->BoolValue());
	}
	
	if (p != nullptr && task_element->FindAttribute("mode") != nullptr){
		p->setVoltageMode(static_cast<VOLTAGE_MODE>(task_element->FindAttribute("mode")->IntValue()));
	}
	
	if (p != nullptr && task_element->FindAttribute("duty_cycle") != nullptr){
		p->setDutyCycle(task_element->FindAttribute("duty_cycle")->DoubleValue());
	}
	return p;
}

bool I2CVoltageTask::isConnected(){
	return (connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ID) == I2C_ATTINY45_VOLT_SLAVE_ADDRESS);
}

double I2CVoltageTask::readCurrentDutyCycle(){
	int OCR1A = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_OCR1A);
	int OCR1C = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_OCR1C);
	
	double dutyCycle = (((double) OCR1A) / ((double) OCR1C));
	
//	std::cout << "OCR1A: " << OCR1A << "\tOCR1C: " << OCR1C << std::endl;
	
	return dutyCycle;
}

double I2CVoltageTask::readVoltage(){
	int ADCL = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCL);
	int ADCH = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCH);
	int ADC = ADCH * 256 + ADCL;
	
	double voltage = (((double)VOLTAGE_ATTINY45_R2) + ((double) VOLTAGE_ATTINY45_R1)) / ((double) VOLTAGE_ATTINY45_R2) / ((double) VOLTAGE_ATTINY45_R_CORR) * ((double) ADC) / 1023.0 * 5.0;
//	std::cout << "ADCL: " << ADCL << "\tADCH: " << ADCH << "\ADC: " << ADC << std::endl;
	
	return voltage;
}
double I2CVoltageTask::readExternalVoltage(){
	Uc_Connection::Uc_Connection_ptr atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	int ADCL = atmega32->readRegister(I2C_ATMEGA32_BUFFER_ADCL);
	int ADCH = atmega32->readRegister(I2C_ATMEGA32_BUFFER_ADCH);
	int ADC = ADCH * 256 + ADCL;
	double ext_voltage = (((double) VOLTAGE_ATMEGA32_R1) + ((double) VOLTAGE_ATMEGA32_R2)) / ((double) VOLTAGE_ATMEGA32_R2) / ((double) VOLTAGE_ATMEGA32_R_CORR) * ((double) ADC) / 1023.0 * 5.0;
	
	return ext_voltage;
}

double I2CVoltageTask::readSetpointVoltage(){
	int ADCL = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCL_S);
	int ADCH = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_ADCH_S);
	
	int ADC = ADCH * 256 + ADCL;
	
	double voltage = (((double)VOLTAGE_ATTINY45_R2) + ((double) VOLTAGE_ATTINY45_R1)) / ((double) VOLTAGE_ATTINY45_R2) / ((double) VOLTAGE_ATTINY45_R_CORR) * ((double) ADC) / 1024.0 * 5.0;
//	std::cout << "ADCL: " << ADCL << "\tADCH: " << ADCH << "\ADC: " << ADC << std::endl;
	return voltage;
}

std::string I2CVoltageTask::to_string() const{
	std::string source = (mode == VOLTAGE_MODE::MODE_EXTERN ? "external source" : "internal source");
	return source + " - " + (voltage > 0 ? "V=" + std::to_string(voltage).append("V") : "voltage off");
}

void I2CVoltageTask::setWaitForVoltage(bool wait){
	waitForVoltage = wait;
}

bool I2CVoltageTask::getWaitForVoltage() const{
	return waitForVoltage;
}

void I2CVoltageTask::setVoltageMode(I2CVoltageTask::VOLTAGE_MODE mode){
	I2CVoltageTask::mode = mode;
}

I2CVoltageTask::VOLTAGE_MODE I2CVoltageTask::getVoltageMode() const{
	return mode;
}

void I2CVoltageTask::setDutyCycle(double d){
	if (d > 0 && d <= 1){
			dutyCycle = d;
	}else{
		std::cout << "duty cycle" << d << " out of range - needs to be in the range 0..1" << std::endl;
	}
}

double I2CVoltageTask::getDutyCycle() const{
	return dutyCycle;
}
	
I2CVoltageTask::VOLTAGE_MODE I2CVoltageTask::readCurrentMode(){
	int mode = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_MODE);
	
	switch (mode){
		case I2C_ATTINY45_VOLT_BUFFER_MODE_DUTY:
			return VOLTAGE_MODE::MODE_DUTY_CYCLE;
			
		case I2C_ATTINY45_VOLT_BUFFER_MODE_CONT:
			return VOLTAGE_MODE::MODE_CONTROLLER;
			
		default:
			return VOLTAGE_MODE::MODE_ERROR;
	}
	
	return VOLTAGE_MODE::MODE_ERROR;
}

int I2CVoltageTask::readFrequency(){
	int freq0 = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ0);
	int freq1 = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ1);
	int freq2 = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ2);
	int freq3 = connection->readRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ3);
	int freq = freq0 + (freq1 << 8) + (freq2 << 16) + (freq3 << 24);
	
	return freq;
}

std::list<Task::DEVICES> I2CVoltageTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	if (mode != VOLTAGE_MODE::MODE_EXTERN){
		Task::DEVICES d = Task::DEVICES::DEVICE_ATTINY_VOLT;
		devices.push_back(d);
	}
	
	
	Task::DEVICES d = Task::DEVICES::DEVICE_ATMEGA_GENERAL;
	devices.push_back(d);
	
	return devices;
}

I2CVoltageTask::TimePoint I2CVoltageTask::getCurrentTime(){
	return std::chrono::system_clock::now();
}

double I2CVoltageTask::getElapsedSeconds(I2CVoltageTask::TimePoint startTime){
	std::chrono::duration<double> elapsed_seconds = getCurrentTime() - startTime;
	
	return elapsed_seconds.count();
}