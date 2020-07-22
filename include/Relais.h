#pragma once

/**
 * @file Relais.h
 * 
 * @class Relais
 * @author Nils Bosbach
 * @date 31.07.2019
 * @brief class to control the relais connected to the atmega32 via I2C. The methods getRelais and setRelais only change the internal variable, the configuration
 * is transmitted to the uc during the writeRelais method.
 */

#include "Uc_Connection.h"

#include <memory>



class Relais{
public:
	typedef std::shared_ptr<Relais> Relais_ptr;
	
	enum RELAIS {R_STEUER_BOOST_IN = 0, R_STEUER_HV_EXT = 1, R_STEUER_SAFETY = 2, R_STEUER_AC = 3, R_STEUER_AC_2 = 8, R_STEUER_IMP_EWOD = 9, R_STEUER_WE_P_WE_S = 10, R_STEUER_RE_P_CE = 11, R_STEUER_EXT_PICO = 12};
	
	/**
	 * @brief Constructor, opens the i2c connection and initializes the internal relais variable with 0x00
	 */
	Relais();
	
	/**
	 * @brief Destructor, writes all registers to their initial values
	 */
	virtual ~Relais();
	
	/**
	 * @brief creates a new Relais object
	 * @return a smart pointer to the created relais object
	 */
	static Relais_ptr create();
	
	/**
	 * @brief read the current relais configuration from the uc and store it in the interial varible
	 * @see getRelais
	 */
	void readRelais();
	
	/**
	 * @brief sends the current state of the internal relais configuration to the uc
	 * @see setRelais
	 */
	void writeRelais();
	
	/**
	 * @brief Set the value of one relais in the internal relais variable. The values can be transmitted to the uc by using the writeRelais method
	 * @param r the relais which value should be set
	 * @param value the value of the realis
	 */
	void setRelais(RELAIS r, bool value);
	
	/**
	 * @brief Read the current configuration of the relais from the internal variable (this configuration does not necessarily be the current configuration
	 * of the relais). The current configuration can be read from the uc by the readRelais fuction.
	 * @param r the relais which value should be returned
	 * @return the state of the relais in the internal variable
	 */
	bool getRelais(RELAIS r) const;
	
	/**
	 * @brief check, if the atmega32 can be accessed via i2c
	 * @return true if uc can be accessed
	 */
	bool uc_is_connected();
	
private:
	typedef std::chrono::system_clock::time_point TimePoint;
	
	TimePoint lastWrite;
	char relais_L;
	char relais_H;
	Uc_Connection::Uc_Connection_ptr conn;
	
	inline TimePoint getCurrentTime() const;
	inline void delay_to_last_write() const;
};
