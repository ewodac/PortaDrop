#pragma once
/**
 * @file StatusLed.h
 * 
 * @class StatusLed
 * @author Nils Bosbach
 * @date 04.08.2019
 * @brief used to control the status LED, which are connected to a shift register via the atmega32
 */
#include "Uc_Connection.h"

#include <memory>
#include <chrono>
#include <bitset>



class StatusLed{
public:
	/// smart pointer to a StatusLed object
	typedef std::shared_ptr<StatusLed> StatusLed_ptr;
	
	/**
	 * @brief constructor - opens the i2c connenction to the atmega32
	 */
	StatusLed();
	
	/**
	 * @brief create a StatusLed object
	 * @return smart pointer to the created object
	 */
	static StatusLed_ptr create();
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~StatusLed();
	
	/**
	 * @brief sets the running LED in the internal variable. The value can be transfered to the atmega32 by calling the write_reg_val function
	 * @param bool status of the LED
	 */
	void running(bool on);
	
	/**
	 * @brief sets the impedance meausurement running LED in the internal variable. The value can be transfered to the atmega32 by calling the write_reg_val function
	 * @param bool status of the LED
	 */
	void impMeasRunning(bool on);
	
	/**
	 * @brief sets the recipe running LED in the internal variable. The value can be transfered to the atmega32 by calling the write_reg_val function
	 * @param bool status of the LED
	 */
	void recipeRunning(bool on);
	
	/**
	 * @brief sets all LEDs in the internal variable. The value can be transfered to the atmega32 by calling the write_reg_val function
	 * @param bool status of all LEDs
	 */
	void all(bool on);
	
	/**
	 * @brief sets the Pad LED in the internal variable. The value can be transfered to the atmega32 by calling the write_reg_val function
	 * @param bool status of the Pad LED
	 */
	void pad(bool on);
	
	
	/**
	 * @brief reads the current status of the LEDs from the atmega32 and stores it in the internal variable
	 */
	void read_reg_val();
	
	/**
	 * @brief writes the value of the internal variable to the atmega32
	 */
	void write_reg_val();
		
private:
	typedef std::chrono::system_clock::time_point TimePoint;
	Uc_Connection::Uc_Connection_ptr uc_connection;
	TimePoint lastWrite;
	std::bitset<16> reg_val;
	
	
	inline void setLED(bool on, int led_id);
	inline TimePoint getCurrentTime() const;
	inline void delay_to_last_write() const;
	
};
