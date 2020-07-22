#include "Relais.h"
#include "Addresses.h"

#include <iostream>
#include <stdexcept>
#include <unistd.h>

Relais::Relais(){
	lastWrite = getCurrentTime();
	relais_L = 0x00;
	relais_H = 0x00;
	conn = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
}
Relais::~Relais(){
	relais_L = 0x00;
	relais_H = 0x00;
	writeRelais();
}
Relais::Relais_ptr Relais::create(){
	return std::make_shared<Relais>();
}

void Relais::readRelais(){
	delay_to_last_write();
	if (uc_is_connected()){
		relais_L = conn->readRegister(I2C_ATMEGA32_BUFFER_RELAISL);
		relais_H = conn->readRegister(I2C_ATMEGA32_BUFFER_RELAISH);
		std::cout << "relais_L: " << ((int) relais_L) << "\trelais_H: " << ((int) relais_H) << std::endl;
		lastWrite = getCurrentTime();
	}else{
		throw std::runtime_error("relais cannot be read - i2c slave " + std::to_string(I2C_ATMEGA32_SLAVE_ADDRESS) + " is not connected"); 
	}
}
void Relais::writeRelais(){
	delay_to_last_write();
	if (uc_is_connected()){
		conn->writeRegister(I2C_ATMEGA32_BUFFER_RELAISL, relais_L);
		usleep(10);
		conn->writeRegister(I2C_ATMEGA32_BUFFER_RELAISH, relais_H);
		lastWrite = getCurrentTime();
	}else{
		throw std::runtime_error("relais cannot be written - i2c slave " + std::to_string(I2C_ATMEGA32_SLAVE_ADDRESS) + " is not connected"); 
	}
}
void Relais::setRelais(Relais::RELAIS r, bool value){
	if (r < 8){
		if (value){
			relais_L |= ((char)1) << r;
		}else{
			relais_L &= ~(((char)1) << r);
		}
	}else{
		if (value){
			relais_H |= ((char)1) << (r-8);
		}else{
			relais_H &= ~(((char)1) << (r-8));
		}
	}
	
}
bool Relais::getRelais(RELAIS r) const{
	if (r < 8){
		return relais_L & (((char)1) << r);
	}else{
		return relais_H & (((char)1) << (r-8));
	}
	
}
bool Relais::uc_is_connected(){
	return (conn->readRegister(I2C_ATMEGA32_BUFFER_ID) == I2C_ATMEGA32_SLAVE_ADDRESS);
}
Relais::TimePoint Relais::getCurrentTime() const{
	return std::chrono::system_clock::now();
}
void Relais::delay_to_last_write() const{
	std::chrono::duration<double> elapsed_seconds;
	
	do{
		elapsed_seconds = getCurrentTime() - lastWrite;
	}while (elapsed_seconds.count() < 0.01);
}
