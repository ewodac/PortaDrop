#pragma once
/**
 * @file MeasurementError.h
 * 
 * @class MeasurementError
 * @author Nils Bosbach
 * @date 31.08.2019
 * @see MeasurementPackage
 * @see MeasurementValue
 * @brief this class represents an error which can occur during the communication with a EmStat pico potentiostat
 */
#include <memory>
#include <string>

class MeasurementError{
public:
	
	/// smart pointer to a MeasurementError object
	typedef std::shared_ptr<MeasurementError> MeasurementError_ptr;
	
	/// possible errors
	enum ERROR {ERROR_STATUS_ERR, ERROR_STATUS_INVALID_VT, ERROR_STATUS_UNKNOWN_CMD, ERROR_STATUS_REG_UNKNOWN, ERROR_STATUS_REG_READ_ONLY, ERROR_STATUS_WRONG_COMM_MODE, ERROR_STATUS_BAD_ARG, ERROR_STATUS_CMD_BUFF_OVERFLOW, ERROR_STATUS_CMD_TIMEOUT, ERROR_STATUS_REF_ARG_OUT_OF_RANGE, ERROR_STATUS_OUT_OF_VAR_MEM, ERROR_STATUS_NO_SCRIPT_LOADED, ERROR_STATUS_INVALID_TIME, ERROR_STATUS_OVERFLOW, ERROR_STATUS_INVALID_POTENTIAL, ERROR_STATUS_INVALID_BITVAL, ERROR_STATUS_INVALID_FREQUENCY, ERROR_STATUS_INVALID_AMPLITUDE, ERROR_STATUS_NVM_ADDR_OUT_OF_RANGE, ERROR_STATUS_OCP_CELL_ON_NOT_ALLOWED, ERROR_STATUS_INVALID_CRC, ERROR_STATUS_FLASH_ERROR, ERROR_STATUS_INVALID_FLASH_ADDR, ERROR_STATUS_SETTINGS_CORRUPT, ERROR_STATUS_AUTH_ERR, ERROR_STATUS_CALIBRATION_INVALID, ERROR_STATUS_NOT_SUPPORTED, ERROR_STATUS_NEGATIVE_ESTEP, ERROR_STATUS_NEGATIVE_EPULSE, ERROR_STATUS_NEGATIVE_EAMP, ERROR_STATUS_TECH_NOT_LICENCED, ERROR_STATUS_MULTIPLE_HS, ERROR_STATUS_UNKNOWN_PGS_MODE, ERROR_STATUS_CHANNEL_NOT_POLY_WE, ERROR_STATUS_INVALID_FOR_PGSTAT_MODE, ERROR_STATUS_TOO_MANY_EXTRA_VARS, ERROR_STATUS_UNKNOWN_PAD_MODE, ERROR_STATUS_FILE_ERR, ERROR_STATUS_FILE_EXISTS, ERROR_STATUS_SCRIPT_SYNTAX_ERR, ERROR_STATUS_SCRIPT_UNKNOWN_CMD, ERROR_STATUS_SCRIPT_BAD_ARG, ERROR_STATUS_SCRIPT_ARG_OUT_OF_RANGE, ERROR_STATUS_SCRIPT_UNEXPECTED_CHAR, ERROR_STATUS_SCRIPT_OUT_OF_CMD_MEM, ERROR_STATUS_SCRIPT_UNKNOWN_VAR_TYPE, ERROR_STATUS_SCRIPT_VAR_UNDEFINED, ERROR_STATUS_SCRIPT_INVALID_OPT_ARG, ERROR_STATUS_SCRIPT_INVALID_VERSION, ERROR_STATUS_FATAL_ERROR, ERROR_UNKNOWN};
	
	/**
	 * @brief initializes the class variables and parses a received error
	 * @param s the received error (needs to start with '!');
	 */
	MeasurementError(std::string s);
	
	/**
	 * @brief creates a new MeasurementError object and returns a smart pointer to the object
	 * @param s the received error (needs to start with '!');
	 * @return pointer to the created object
	 */
	static MeasurementError_ptr create(std::string s);
	
	/**
	 * @brief get the error
	 * @return the error 
	 */
	ERROR getError() const;
	
	/**
	 * @brief get the line of the Method script where the error occured
	 * @return line of the Method script where the error is
	 */
	int getLine() const;
	
	/**
	 * @brief if the error is a sythax error, the method will return the column where the error occured. If the error is a runtime error, the function will return -1
	 * @return -1 if synthax error; error column if error is a runtime errror
	 */
	int getCol() const;
	
	/**
	 * @brief get the description of the error as string
	 * @return string description of the error
	 */
	std::string getDescr() const;
	
	
protected:
	/**
	 * @brief transforms a error code into a object of the error code enum
	 * @return the transformed enum object
	 */
	static ERROR intToError(int error);
	
	/**
	 * @brief get the description of an error as string
	 * @return error description
	 */
	static std::string errorToStr(ERROR e);
	
	/**
	 * @brief convert a string hex number to a int (e.g. "1A" -> 26)
	 * @param s string hex number
	 * @return corresponding int
	 */
	static unsigned int hexStrToInt(std::string s);
	
	/**
	 * @brief read a string formatted error and saved the data in the class variables
	 * @param line string formatted error
	 */
	void parse(std::string s);
	
	int line = -1;
	int column = -1;
	ERROR e = ERROR::ERROR_UNKNOWN;
};
