#pragma once 
/**
 * @file TempData.h
 * 
 * @class TempData
 * @author Nils Bosbach
 * @date 10.04.2019
 * @brief stores temperature and humidity data
 * This class can store temperature and humidity pair of data. The constructor TempData(int* measurement)
 * enables initializing the Object with the raw data of a DHT11 sensor
 */
#include <memory>
 
class TempData{
public:
	typedef std::shared_ptr<TempData> TempData_ptr;
	
	enum UNIT {FAHRENHEIT, CELSIUS};
	
	/**
	 * @brief use DHT11 data
	 * @param measurement int[5] array which contains the 5 bytes received from DHT11
	 */
	TempData(int* measurement);
	
	/**
	 * @param temperature temperature
	 * @param u unit of the temperature (CELSIUS or FAHRENHEIT)
	 * @param humidity humidity [percentage]
	 */
	TempData(double temperature, UNIT u, double humidity);
	~TempData();
	
	/**
	 * @param u UNIT of the temperature (CELSIUS / FAHRENHEIT)
	 * @return the temperature as double
	 */
	const double getTemperature(UNIT u = UNIT::CELSIUS) const;
	
	/**
	 * @return stored humidity
	 */
	const double getHumidity() const;
	
	/**
	 * @brief returns if the DHT11 data was valid (checksum test)
	 * @return true if checksum test was positive
	 * only senseful if the DHT11 data constructor has been used!
	 */
	const bool isValid() const; 
	
private:
	double temperature; //temperature in Celsius
	double humidity; //humidity
	bool valid; // valid measurement (checksum test)
	static double cToF(double tempC); //converts a celsius temperature to fahrenheit
	static double fToC(double C); //converts a fahrenheit temperature to celsius
	
};
