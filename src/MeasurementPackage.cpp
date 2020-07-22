#include "MeasurementPackage.h"

#include <iostream>

MeasurementPackage::MeasurementPackage(std::string line){
	measurements = std::make_shared<MeasurementList>();
	parseLine(line);
}
MeasurementPackage::MeasurementPackage_ptr MeasurementPackage::create(std::string line){
	return std::make_shared<MeasurementPackage>(line);
}

MeasurementPackage::MeasurementList_ptr MeasurementPackage::getMeasurements() const{
	return measurements;
}

std::string MeasurementPackage::to_string() const{
	std::string returnString = "";
	for(MeasurementList::iterator it = measurements->begin(); it != measurements->end(); it++){
		MeasurementValue::MeasurementValue_ptr p = *it;
		returnString += " - " + p->to_string();
	}
	
	if (!returnString.empty()){
		returnString = returnString.substr(3);
	}
	return returnString;
}

void MeasurementPackage::parseLine(std::string line){
	if (line.at(0) == 'P'){ // Package data
		
		unsigned int posLastCSemicolon = 0; // skip first character
		
		while(posLastCSemicolon != std::string::npos){
			std::string partString = line.substr(posLastCSemicolon + 1, line.find(';', posLastCSemicolon + 1) - (posLastCSemicolon + 1));
			posLastCSemicolon = line.find(';', posLastCSemicolon + 1);
			
			measurements->push_back(MeasurementValue::create(partString));
		}
	}else{ //line is no package data
		throw std::invalid_argument("no package data");
	}
}
