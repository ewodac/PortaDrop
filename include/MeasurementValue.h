#pragma once

/**
 * @file MeasurementValue.h
 * 
 * @class MeasurementValue
 * @author Nils Bosbach
 * @date 31.08.2019
 * @see MeasurementPackage
 * @see MeasurementError
 * @brief this class represents one measuremnt value of a MeasurementPackage received by the EmStat pico potentiostat. A part of the received string can be parsed and the measurement value, type and error are stored in the object
 */
#include <memory>
#include <string>
#include <list>


#define VT_UNKNOWN "aa"
#define VT_POTENTIAL_RE "ab"
#define VT_POTENTIAL_CE "ac"
#define VT_POTENTIAL_WE "ad"
#define VT_POTENTIAL_AUX1_IN "as"
#define VT_POTENTIAL_AUX2_IN "at"
#define VT_CURRENT_WE "ba"
#define VT_PHASE "cp"
#define VT_IMP "ci"
#define VT_ZREAL "cc"
#define VT_ZIMAG "cd"
#define VT_CELL_POTENTIAL "da"
#define VT_CELL_CURRENT "db"
#define VT_CELL_FREQUENCY "dc"
#define VT_CELL_AMPLITUDE "dd"

class MeasurementValue{
public:
	typedef std::shared_ptr<MeasurementValue> MeasurementValue_ptr;
	
	/**
	 * STATUS_OK:				measurement valid
	 * STATUS_OVERLOAD:			>95% of max ADC value
	 * STATUS_UNDERLOAD:		<2% of max ADC value
	 * STATUS_OVERLOAD_WARNING:	>80% of max ADC value
	 */
	enum META_STATUS {STATUS_OK, STATUS_OVERLOAD, STATUS_UNDERLOAD, STATUS_OVERLOAD_WARNING, STATUS_NO_STATUS};
	
	///Type of the measurement
	enum MEASUREMENT_TYPE {TYPE_ERROR, TYPE_VT_UNKNOWN, TYPE_VT_POTENTIAL_RE, TYPE_VT_POTENTIAL_CE, TYPE_VT_POTENTIAL_WE, TYPE_VT_POTENTIAL_AUX1_IN, TYPE_VT_POTENTIAL_AUX2_IN, TYPE_VT_CURRENT_WE, TYPE_VT_PHASE, TYPE_VT_IMP, TYPE_VT_ZREAL, TYPE_VT_ZIMAG, TYPE_VT_CELL_POTENTIAL, TYPE_VT_CELL_CURRENT, TYPE_VT_CELL_FREQUENCY, TYPE_VT_CELL_AMPLITUDE };
	
	/**
	 * @brief inits variables
	 * @param line received string to parse
	 * 
	 * line: ttHHHHHHHp, MV..V, MV..V
	 *
	 * tt: var type
	 * HHHHHHH: data value in hex with 0x8000000 offset
	 * p: prefix
	 */
	MeasurementValue(std::string line);
	
	/**
	 * @brief creates a new Measurement object and returnes a pointer to the object
	 */
	static MeasurementValue_ptr create(std::string line);
	
	/**
	 * @brief get the type of the measurement 
	 * @return the type of this measurement (voltage, freq, ...)
	 */
	MEASUREMENT_TYPE getType() const;
	
	/**
	 * @brief get the value of the measurement
	 * @return value of the measuremnt
	 * @see getType
	 */
	double getValue() const;
	
	/**
	 * @brief get the status of the measurement to see if the value is valid
	 * @return status of the meausrement
	 */
	META_STATUS getStatus() const;
	
	/**
	 * @brief variables formatted as string
	 * @return <type>: <value>(<status>)(<range>)
	 */
	std::string to_string() const;
	
	/**
	 * @brief get the used current range of the measurement
	 * @return the current range, -1 if measurement has no current range
	 */
	int getCurrentRange() const;
	
	/**
	 * @brief convert a status to a string
	 * @param m the status to convert
	 * @return the converted string
	 */
	static std::string statusToString(META_STATUS m);
	
	/**
	 * @brief convert a current range to a string
	 * @param cr the current range to convert
	 * @return the converted string
	 */
	static std::string currentRangeToStr(int cr);
	
protected:
	
	/**
	 * @brief turn a prefix (e.g. 'm' for milli) into the corresponding factor (1e-3 for 'm')
	 * @param param p the prefix as char
	 * @return prefix as factor
	 */
	static double getPrefix(char p);
	
	/**
	 * @brief convert a string hex number to a int (e.g. "1A" -> 26)
	 * @param s string hex number
	 * @return corresponding int
	 */
	static unsigned int hexStrToInt(std::string s);
	
	/**
	 * @brief convert a measurement type to string (e.g TYPE_VT_POTENTIAL_CE -> "potential CE")
	 * @param m measurement type
	 * @return converted string
	 */
	static std::string typeToString(MEASUREMENT_TYPE m);
	
	/**
	 * @brief read a string formatted measurement and saved the data in the class variables
	 * @param line string formatted measurement
	 */
	virtual void parse(std::string line);
	
	
	META_STATUS status = META_STATUS::STATUS_NO_STATUS;
	MEASUREMENT_TYPE mtype = MEASUREMENT_TYPE::TYPE_ERROR;
	double value;
	int range = -1;
};
