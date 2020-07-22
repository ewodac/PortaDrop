#include "TempData.h"

#include <stdio.h>
#include <iostream>
#include <iomanip>

/* Contructor
* needs a int[5] array as paramter which contains the raw data of a DHT11 sensor
*/
TempData::TempData(int* measurement){
	/* measurement[0]  integer part of humidity
	 * measurement[1]  decimal part of humidity
	 * measurement[2]  integer part of temperature
	 * measurement[3]  decimal part of temperature
	 * measurement[4]  parity byte
	 * */
	if (measurement[0] + measurement[1] + measurement[2] + measurement[3] == measurement[4]){ //parity check
		TempData::valid = true;
		
		double temp = measurement[3]; // temp = decimal part of temperature
		while (temp > 1.0){ // shift decimal part behind the colon
			temp = temp / 10.0;
		}
		temp += measurement[2]; // add integer part
		
		double hum = measurement[1]; // hum = decimal part of humidity
		while (hum > 0){ // shift decimal part behind the colon
			hum = hum / 10.0;
		}
		hum += measurement[0]; // add integer part
		
		TempData::humidity = hum;
		TempData::temperature = temp;
		
	}else{ //error
		TempData::valid = false;
	}
}

TempData::TempData(double temperature, UNIT u, double humidity){
	TempData::valid = true;
	switch (u){
		case TempData::UNIT::CELSIUS:
			TempData::temperature = temperature;
			break;
		case TempData::UNIT::FAHRENHEIT:
			TempData::temperature = TempData::fToC(temperature);
			break;
	}
	TempData::humidity = humidity;
}
TempData::~TempData(){
	
}

/*
* returns the temperature
*/
const double TempData::getTemperature(UNIT u) const{
	switch (u){
		case TempData::UNIT::FAHRENHEIT:
			return TempData::cToF(TempData::temperature);
		case TempData::UNIT::CELSIUS:
			return TempData::temperature;
	}
	return -1;
}
const double TempData::getHumidity() const{
	return TempData::humidity;
}
/*
 * returns if the data is valid (cheksum test)
 */
const bool TempData::isValid() const{
	return TempData::valid;
}
/*
* converts from celsius to farenheit
*/
double TempData::cToF(double tempC){
	return ((tempC * 9 / 5) + 32);
}
/*
* converts from farenheit to celsius
*/
double TempData::fToC(double tempF){
	return ((tempF - 32) * 5 / 9);
}
