#include "MeasurementValue.h"

#include <sstream>
#include <iostream>
#include <stdexcept>

MeasurementValue::MeasurementValue(std::string line){
	parse(line);
}
MeasurementValue::MeasurementValue_ptr MeasurementValue::create(std::string line){
	return std::make_shared<MeasurementValue>(line);
}

double MeasurementValue::getPrefix(char p){
	switch(p){
		case 'a': //atto
			return 1e-18;
			
		case 'f': //femto
			return 1e-15;
			
		case 'p': //pico
			return 1e-12;
			
		case 'n': // nano
			return 1e-9;
			
		case 'u': //micro
			return 1e-6;
		
		case 'm': //milli
			return 1e-3;
			
		case ' ': // none
			return 1;
			
		case 'k': //kilo
			return 1e3;
			
		case 'M': //Mega
			return 1e6;
			
		case 'G': //Giga
			return 1e9;
			
		case 'T': //Tera
			return 1e12;
			
		case 'P': //Peta
			return 1e15;
			
		case 'E': //Exa
			return 1e18;
			
		default:
			throw std::invalid_argument("no valid prefix: " + p);
	}
	return 1;
}

unsigned int MeasurementValue::hexStrToInt(std::string s){
	unsigned int i;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> i;
	return i;
}
MeasurementValue::MEASUREMENT_TYPE MeasurementValue::getType() const{
	return mtype;
}
double MeasurementValue::getValue() const{
	return value;
}
std::string MeasurementValue::typeToString(MEASUREMENT_TYPE m){
	switch(m){
		case TYPE_ERROR:
			return "error";
			
		case TYPE_VT_UNKNOWN:
			return "unknown";
			
		case TYPE_VT_POTENTIAL_RE:
			return "potential RE";
			
		case TYPE_VT_POTENTIAL_CE:
			return "potential CE";
			
		case TYPE_VT_POTENTIAL_WE:
			return "potential WE";
			
		case TYPE_VT_POTENTIAL_AUX1_IN:
			return "potential aux1 in";
			
		case TYPE_VT_POTENTIAL_AUX2_IN:
			return "potential aux2 in";
			
		case TYPE_VT_CURRENT_WE:
			return "current WE";
			
		case TYPE_VT_PHASE:
			return "phase";
			
		case TYPE_VT_IMP:
			return "imp";
			
		case TYPE_VT_ZREAL:
			return "Z real";
			
		case TYPE_VT_ZIMAG:
			return "Z imag";
			
		case TYPE_VT_CELL_POTENTIAL:
			return "cell potential";
			
		case TYPE_VT_CELL_CURRENT:
			return "cell current";
			
		case TYPE_VT_CELL_FREQUENCY:
			return "cell frequency";
			
		case TYPE_VT_CELL_AMPLITUDE:
			return "cell amplitude";
	}
	return "none";
}

std::string MeasurementValue::statusToString(META_STATUS s){
	switch(s){
		case STATUS_OK:
			return "OK";
			
		case STATUS_OVERLOAD:
			return "OVERLOAD";
			
		case STATUS_UNDERLOAD:
			return "UNDERLOAD";
			
		case STATUS_OVERLOAD_WARNING:
			return "OVERLOAD_WARNING";
			
		case STATUS_NO_STATUS:
			return "NO_STATUS";
	}
	return "";
}
std::string MeasurementValue::to_string() const{
	std::ostringstream streamObj;
	streamObj << value;
	std::string status_str = ((status == META_STATUS::STATUS_NO_STATUS) ? "" : "(" + statusToString(status) + ")");
	std::string range_str = ((range != -1) ? "" : "(" + currentRangeToStr(range) + ")");
	
	return typeToString(mtype) + ": " + streamObj.str() + status_str + range_str;
}

/*
 * line: ttHHHHHHHp, MV..V, MV..V
 *       0123456789
 * 
 * tt: var type
 * HHHHHHH: data value in hex with 0x8000000 offset
 * p: 
 */
void MeasurementValue::parse(std::string line){
	if (line.find(VT_UNKNOWN) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_UNKNOWN;
	
	}else if (line.find(VT_POTENTIAL_RE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_POTENTIAL_RE;
	
	}else if (line.find(VT_POTENTIAL_CE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_POTENTIAL_CE;
	
	}else if (line.find(VT_POTENTIAL_WE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_POTENTIAL_WE;
	
	}else if (line.find(VT_POTENTIAL_AUX1_IN) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_POTENTIAL_AUX1_IN;
	
	}else if (line.find(VT_POTENTIAL_AUX2_IN) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_POTENTIAL_AUX2_IN;
	
	}else if (line.find(VT_CURRENT_WE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_CURRENT_WE;
	
	}else if (line.find(VT_PHASE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_PHASE;
	
	}else if (line.find(VT_IMP) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_IMP;
	
	}else if (line.find(VT_ZREAL) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_ZREAL;
	
	}else if (line.find(VT_ZIMAG) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_ZIMAG;
	
	}else if (line.find(VT_CELL_POTENTIAL) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_CELL_POTENTIAL;
	
	}else if (line.find(VT_CELL_CURRENT) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_CELL_CURRENT;
	
	}else if (line.find(VT_CELL_FREQUENCY) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_CELL_FREQUENCY;
	
	}else if (line.find(VT_CELL_AMPLITUDE) == 0){
		mtype = MEASUREMENT_TYPE::TYPE_VT_CELL_AMPLITUDE;
	
	}
	
	if (mtype != MEASUREMENT_TYPE::TYPE_ERROR){
		std::string hexValueWO = line.substr(2, 7); // hex value with 0x8000000 offset
		char prefix = line.at(9);
		int valueWP = MeasurementValue::hexStrToInt(hexValueWO) - 0x8000000;  // value without prefix
		value = valueWP * getPrefix(prefix);
		
		unsigned int posLastComma = line.find(',', 0);
		while(posLastComma != std::string::npos){
			std::string partString = line.substr(posLastComma + 1, line.find(',', posLastComma + 1) - (posLastComma + 1));
			posLastComma = line.find(',', posLastComma + 1);
			
			switch (partString.at(0)){
				case '1':{ //MetaData Mode
					switch (partString.at(1)){
						case '0':{
							status = MeasurementValue::META_STATUS::STATUS_OK;
							break;
						}
						
						case '2':{
							status = MeasurementValue::META_STATUS::STATUS_OVERLOAD;
							break;
						}
						
						case '4':{
							status = MeasurementValue::META_STATUS::STATUS_UNDERLOAD;
							break;
						}
						
						case '8':{
							status = MeasurementValue::META_STATUS::STATUS_OVERLOAD_WARNING;
							break;
						}
					}
					break;
				}
				case '2':{ //Current Range
					MeasurementValue::range = MeasurementValue::hexStrToInt(partString.substr(1));
					break;
				}
			}
		}
	}else{
		throw std::invalid_argument("no valid variable type");
	}
}

MeasurementValue::META_STATUS MeasurementValue::getStatus() const{
	return status;
}
std::string MeasurementValue::currentRangeToStr(int cr){
	std::string currentRangeStr;
	
	switch (cr){
		case 0:
			currentRangeStr = "100nA";
			break;
		case 1:
			currentRangeStr = "2uA";
			break;
		case 2:
			currentRangeStr = "4uA";
			break;
		case 3:
			currentRangeStr = "8uA";
			break;
		case 4:
			currentRangeStr = "16uA";
			break;
		case 5:
			currentRangeStr = "32uA";
			break;
		case 6:
			currentRangeStr = "63uA";
			break;
		case 7:
			currentRangeStr = "125uA";
			break;
		case 8:
			currentRangeStr = "250uA";
			break;
		case 9:
			currentRangeStr = "500uA";
			break;
		case 10:
			currentRangeStr = "1mA";
			break;
		case 11:
			currentRangeStr = "15mA";
			break;
		case 128:
			currentRangeStr = "100nA (High speed)";
			break;
		case 129:
			currentRangeStr = "1uA (High speed)";
			break;
		case 130:
			currentRangeStr = "6uA (High speed)";
			break;
		case 131:
			currentRangeStr = "13uA (High speed)";
			break;
		case 132:
			currentRangeStr = "25uA (High speed)";
			break;
		case 133:
			currentRangeStr = "50uA (High speed)";
			break;
		case 134:
			currentRangeStr = "100uA (High speed)";
			break;
		case 135:
			currentRangeStr = "200uA (High speed)";
			break;
		case 136:
			currentRangeStr = "1mA (High speed)";
			break;
		case 137:
			currentRangeStr = "5mA (High speed)";
			break;
		default:
			currentRangeStr = "";
	}
	return currentRangeStr;
}

int MeasurementValue::getCurrentRange() const{
	return range;
}
