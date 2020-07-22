#pragma once
/**
 * @file MeasurementPackage.h
 * 
 * @class MeasurementPackage
 * @author Nils Bosbach
 * @date 31.08.2019
 * @see MeasurementValue
 * @see MeasurementError
 * @brief this class represents one measurement package received by the EmStat pico potentiostat. The measurement values (e.g. frequency, voltage, ...) are stored in a list of MeasurmentValue.
 */
#include "MeasurementValue.h"

#include <string>

class MeasurementPackage{
public:
	/// smart pointer to a MeasurementPackage
	typedef std::shared_ptr<MeasurementPackage> MeasurementPackage_ptr;
	
	/// list of MeasurementValues
	typedef std::list<MeasurementValue::MeasurementValue_ptr> MeasurementList;
	
	/// pointer to a list of MeasurementValues
	typedef std::shared_ptr<MeasurementList> MeasurementList_ptr;
	
	
	/**
	 * @brief initialises variables and parses the string of a received package
	 * @param line received string of a package
	 */
	MeasurementPackage(std::string line);
	
	/**
	 * @brief created a new MeasurementPackage and returns a pointer to that object
	 * @param line received string of a package
	 */
	static MeasurementPackage_ptr create(std::string line);
	
	/**
	 * @brief get the captured values of the package
	 * @return list constatining the captured values of the package
	 */
	MeasurementList_ptr getMeasurements() const;
	
	/**
	 * @brief format the captured values as string
	 * @return "<MeasurementValue1.to_string()> - ... - <MeasurementValuen.to_string()>"
	 */
	std::string to_string() const;
	
protected:
	
	/**
	 * @brief read a string formatted package and saved the data in the class variables
	 * @param line string formatted package
	 */
	void parseLine(std::string line);
	
	
	MeasurementList_ptr measurements;
};
