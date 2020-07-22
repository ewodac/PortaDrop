#include "StatusLed.h"
#include "Addresses.h"

#include <iostream>

StatusLed::StatusLed(): uc_connection(Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS)){
	lastWrite = getCurrentTime();
	all(false);
	running(true);
	write_reg_val();
}
StatusLed::StatusLed_ptr StatusLed::create(){
	return std::make_shared<StatusLed>();
}
StatusLed::~StatusLed(){
	running(false);
	write_reg_val();
}

void StatusLed::running(bool on){
	setLED(on, LED_RUNNING);
}
void StatusLed::impMeasRunning(bool on){
	setLED(on, LED_IMP_MEAS_RUNNING);
}
void StatusLed::recipeRunning(bool on){
	setLED(on, LED_RECIPE_RUNNING);
}
void StatusLed::all(bool on){
	reg_val = (on ? 0xFFFF : 0x0000);
}
void StatusLed::pad(bool on){
	setLED(on, LED_PAD);
}

void StatusLed::setLED(bool on, int led_id){
	if (on){
		reg_val.set(led_id);
	}else{
		reg_val.reset(led_id);
	}
}
void StatusLed::read_reg_val(){
	reg_val = uc_connection->readRegister(I2C_ATMEGA32_BUFFER_LEDH);
	reg_val <<= 8;
	reg_val &= (uc_connection->readRegister(I2C_ATMEGA32_BUFFER_LEDH) & 0xFF);
}
void StatusLed::write_reg_val(){
	delay_to_last_write();
	std::bitset<16> bitmask(0x00FF);
	std::bitset<8> low_byte((reg_val & bitmask).to_ulong());
	std::bitset<8> high_byte(((reg_val >> 8) & bitmask).to_ulong());
	
	std::cout << "H: " << high_byte.to_ulong() << "\tL: " << low_byte.to_ulong() << std::endl;
	uc_connection->writeRegister(I2C_ATMEGA32_BUFFER_LEDL, low_byte.to_ulong());
	uc_connection->writeRegister(I2C_ATMEGA32_BUFFER_LEDH, high_byte.to_ulong());
	lastWrite = getCurrentTime();
}

StatusLed::TimePoint StatusLed::getCurrentTime() const{
	return std::chrono::system_clock::now();
}
void StatusLed::delay_to_last_write() const{
	std::chrono::duration<double> elapsed_seconds;
	
	do{
		elapsed_seconds = getCurrentTime() - lastWrite;
	}while (elapsed_seconds.count() < 0.01);
}
