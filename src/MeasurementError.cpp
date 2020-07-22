#include "MeasurementError.h"

#include <sstream>
#include <stdexcept>

MeasurementError::MeasurementError(std::string s){
	parse(s);
}
MeasurementError::MeasurementError_ptr MeasurementError::create(std::string s){
	return std::make_shared<MeasurementError>(s);
}
void MeasurementError::parse(std::string s){
	if (s.at(0) == '!'){
		std::string error_code_str = s.substr(1, 4);
		int error_code = hexStrToInt(error_code_str);
		if (s.find(',') != std::string::npos){ // format: !XXXX: Line L, Col C
			std::string L_str = s.substr(s.find("Line") + 5, s.find(',') - (s.find("Line") + 5));
			std::string C_str = s.substr(s.find("Col") + 4, s.size() - s.find("Col") + 4);
			line = std::stoi(L_str);
			column = std::stoi(C_str);
		}else{// format: !XXXX: Line L
			std::string L_str = s.substr(s.find("Line") + 5, s.size() - (s.find("Line") + 5));
			line = std::stoi(L_str);
			column = -1;
		}
		std::string line_str = s.substr(s.find("Line") + 5, s.size());
		
		e = intToError(error_code);
		
	}else{
		throw std::invalid_argument("no error string: '" + s +"'");
	}
}
unsigned int MeasurementError::hexStrToInt(std::string s){
	unsigned int i;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> i;
	return i;
}
std::string MeasurementError::errorToStr(ERROR e){
	switch(e){
		case ERROR::ERROR_STATUS_ERR:
			return "An unspecified error has occurred";
			
		case ERROR::ERROR_STATUS_INVALID_VT:
			return "An invalid Value Type has been used";
			
		case ERROR::ERROR_STATUS_UNKNOWN_CMD:
			return "The command was not recognized";
			
		case ERROR::ERROR_STATUS_REG_UNKNOWN:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_REG_READ_ONLY:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_WRONG_COMM_MODE:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_BAD_ARG:
			return "An argument has an unexpected value";
			
		case ERROR::ERROR_STATUS_CMD_BUFF_OVERFLOW:
			return "Command exceeds maximum length";
			
		case ERROR::ERROR_STATUS_CMD_TIMEOUT:
			return "The command has timed out";
			
		case ERROR::ERROR_STATUS_REF_ARG_OUT_OF_RANGE:
			return "A var has a wrong identifier";
			
		case ERROR::ERROR_STATUS_OUT_OF_VAR_MEM:
			return "Cannot reserve the memory needed for this var";
			
		case ERROR::ERROR_STATUS_NO_SCRIPT_LOADED:
			return "Cannot run a script without loading one first";
			
		case ERROR_STATUS_INVALID_TIME:
			return "The given (or calculated) time value is invalid for this command";
			
		case ERROR::ERROR_STATUS_OVERFLOW:
			return "An overflow has occurred while averaging a measured value";
			
		case ERROR::ERROR_STATUS_INVALID_POTENTIAL:
			return "The given potential is not valid";
			
		case ERROR::ERROR_STATUS_INVALID_BITVAL:
			return "A variable has become either “NaN” or “inf”";
			
		case ERROR::ERROR_STATUS_INVALID_FREQUENCY:
			return "The input frequency is invalid";
			
		case ERROR::ERROR_STATUS_INVALID_AMPLITUDE:
			return "The input amplitude is invalid";
			
		case ERROR::ERROR_STATUS_NVM_ADDR_OUT_OF_RANGE:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_OCP_CELL_ON_NOT_ALLOWED:
			return "Cannot perform OCP measurement when cell on";
			
		case ERROR::ERROR_STATUS_INVALID_CRC:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_FLASH_ERROR:
			return "An error has occurred while reading / writing flash";
			
		case ERROR::ERROR_STATUS_INVALID_FLASH_ADDR:
			return "An error has occurred while reading / writing flash";
			
		case ERROR::ERROR_STATUS_SETTINGS_CORRUPT:
			return "The device settings have been corrupted";
			
		case ERROR::ERROR_STATUS_AUTH_ERR:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_CALIBRATION_INVALID:
			return "Not applicable for MethodSCRIPT";
			
		case ERROR::ERROR_STATUS_NOT_SUPPORTED:
			return "This command or part of this command is not supported by the current device";
			
		case ERROR::ERROR_STATUS_NEGATIVE_ESTEP:
			return "Step Potential cannot be negative for this technique";
			
		case ERROR::ERROR_STATUS_NEGATIVE_EPULSE:
			return "Pulse Potential cannot be negative for this technique";
			
		case ERROR::ERROR_STATUS_NEGATIVE_EAMP:
			return "Amplitude cannot be negative for this technique";
			
		case ERROR::ERROR_STATUS_TECH_NOT_LICENCED:
			return "Product is not licenced for this technique";
			
		case ERROR::ERROR_STATUS_MULTIPLE_HS:
			return "Cannot have more than one high speed and/or max range mode enabled (EmStat Pico)";
			
		case ERROR::ERROR_STATUS_UNKNOWN_PGS_MODE:
			return "The specified PGStat mode is not supported";
			
		case ERROR::ERROR_STATUS_CHANNEL_NOT_POLY_WE:
			return "Channel set to be used as Poly WE is not configured as Poly WE";
			
		case ERROR::ERROR_STATUS_INVALID_FOR_PGSTAT_MODE:
			return "Command is invalid for the selected PGStat mode";
			
		case ERROR::ERROR_STATUS_TOO_MANY_EXTRA_VARS:
			return "The maximum number of vars to measure has been exceeded";
			
		case ERROR::ERROR_STATUS_UNKNOWN_PAD_MODE:
			return "The specified PAD mode is unknown";
			
		case ERROR::ERROR_STATUS_FILE_ERR:
			return "An error has occurred during a file operation";
			
		case ERROR::ERROR_STATUS_FILE_EXISTS:
			return "Cannot open file, a file with this name already exists";
			
		case ERROR::ERROR_STATUS_SCRIPT_SYNTAX_ERR:
			return "The script contains a syntax error";
			
		case ERROR::ERROR_STATUS_SCRIPT_UNKNOWN_CMD:
			return "The script command is unknown";
			
		case ERROR::ERROR_STATUS_SCRIPT_BAD_ARG:
			return "An argument was invalid for this command";
			
		case ERROR::ERROR_STATUS_SCRIPT_ARG_OUT_OF_RANGE:
			return "An argument was out of range";
			
		case ERROR::ERROR_STATUS_SCRIPT_UNEXPECTED_CHAR:
			return "An unexpected character was encountered";
			
		case ERROR::ERROR_STATUS_SCRIPT_OUT_OF_CMD_MEM:
			return "The script is too large for the internal script memory";
			
		case ERROR::ERROR_STATUS_SCRIPT_UNKNOWN_VAR_TYPE:
			return "The variable type specified is unknown";
			
		case ERROR::ERROR_STATUS_SCRIPT_VAR_UNDEFINED:
			return "The variable has not been declared";
			
		case ERROR::ERROR_STATUS_SCRIPT_INVALID_OPT_ARG:
			return "This optional argument is not valid for this command";
			
		case ERROR::ERROR_STATUS_SCRIPT_INVALID_VERSION:
			return "The stored script is generated for an older firmware version and cannot be run";
			
		case ERROR::ERROR_STATUS_FATAL_ERROR:
			return "A fatal error has occurred, the device must be reset";
			
		case ERROR_UNKNOWN:
			return "The error is unknown";
			
		default:
			return "";
	}
	return "";
}
MeasurementError::ERROR MeasurementError::intToError(int error){
	switch(error){
		case 0x0001:
			return ERROR::ERROR_STATUS_ERR;
			
		case 0x0002:
			return ERROR::ERROR_STATUS_INVALID_VT;
			
		case 0x0003:
			return ERROR::ERROR_STATUS_UNKNOWN_CMD;
			
		case 0x0004:
			return ERROR::ERROR_STATUS_REG_UNKNOWN;
			
		case 0x0005:
			return ERROR::ERROR_STATUS_REG_READ_ONLY;
			
		case 0x0006:
			return ERROR::ERROR_STATUS_WRONG_COMM_MODE;
			
		case 0x0007:
			return ERROR::ERROR_STATUS_BAD_ARG;
			
		case 0x0008:
			return ERROR::ERROR_STATUS_CMD_BUFF_OVERFLOW;
			
		case 0x0009:
			return ERROR::ERROR_STATUS_CMD_TIMEOUT;
			
		case 0x000A:
			return ERROR::ERROR_STATUS_REF_ARG_OUT_OF_RANGE;
			
		case 0x000B:
			return ERROR::ERROR_STATUS_OUT_OF_VAR_MEM;
			
		case 0x000C:
			return ERROR::ERROR_STATUS_NO_SCRIPT_LOADED;
			
		case 0x000D:
			return ERROR::ERROR_STATUS_INVALID_TIME;
			
		case 0x000E:
			return ERROR::ERROR_STATUS_OVERFLOW;
			
		case 0x000F:
			return ERROR::ERROR_STATUS_INVALID_POTENTIAL;
			
		case 0x0010:
			return ERROR::ERROR_STATUS_INVALID_BITVAL;
			
		case 0x0011:
			return ERROR::ERROR_STATUS_INVALID_FREQUENCY;
			
		case 0x0012:
			return ERROR::ERROR_STATUS_INVALID_AMPLITUDE;
			
		case 0x0013:
			return ERROR::ERROR_STATUS_NVM_ADDR_OUT_OF_RANGE;
			
		case 0x0014:
			return ERROR::ERROR_STATUS_OCP_CELL_ON_NOT_ALLOWED;
			
		case 0x0015:
			return ERROR::ERROR_STATUS_INVALID_CRC;
			
		case 0x0016:
			return ERROR::ERROR_STATUS_FLASH_ERROR;
			
		case 0x0017:
			return ERROR::ERROR_STATUS_INVALID_FLASH_ADDR;
			
		case 0x0018:
			return ERROR::ERROR_STATUS_SETTINGS_CORRUPT;
			
		case 0x0019:
			return ERROR::ERROR_STATUS_AUTH_ERR;
			
		case 0x001A:
			return ERROR::ERROR_STATUS_CALIBRATION_INVALID;
			
		case 0x001B:
			return ERROR::ERROR_STATUS_NOT_SUPPORTED;
			
		case 0x001C:
			return ERROR::ERROR_STATUS_NEGATIVE_ESTEP;
			
		case 0x001D:
			return ERROR::ERROR_STATUS_NEGATIVE_EPULSE;
			
		case 0x001E:
			return ERROR::ERROR_STATUS_NEGATIVE_EAMP;
			
		case 0x001F:
			return ERROR::ERROR_STATUS_TECH_NOT_LICENCED;
			
		case 0x0020:
			return ERROR::ERROR_STATUS_MULTIPLE_HS;
			
		case 0x0021:
			return ERROR::ERROR_STATUS_UNKNOWN_PGS_MODE;
			
		case 0x0022:
			return ERROR::ERROR_STATUS_CHANNEL_NOT_POLY_WE;
			
		case 0x0023:
			return ERROR::ERROR_STATUS_INVALID_FOR_PGSTAT_MODE;
			
		case 0x0024:
			return ERROR::ERROR_STATUS_TOO_MANY_EXTRA_VARS;
			
		case 0x0025:
			return ERROR::ERROR_STATUS_UNKNOWN_PAD_MODE;
			
		case 0x0026:
			return ERROR::ERROR_STATUS_FILE_ERR;
			
		case 0x0027:
			return ERROR::ERROR_STATUS_FILE_EXISTS;
			
		case 0x4000:
			return ERROR::ERROR_STATUS_SCRIPT_SYNTAX_ERR;
			
		case 0x4001:
			return ERROR::ERROR_STATUS_SCRIPT_UNKNOWN_CMD;
			
		case 0x4002:
			return ERROR::ERROR_STATUS_SCRIPT_BAD_ARG;
			
		case 0x4003:
			return ERROR::ERROR_STATUS_SCRIPT_ARG_OUT_OF_RANGE;
			
		case 0x4004:
			return ERROR::ERROR_STATUS_SCRIPT_UNEXPECTED_CHAR;
			
		case 0x4005:
			return ERROR::ERROR_STATUS_SCRIPT_OUT_OF_CMD_MEM;
			
		case 0x4006:
			return ERROR::ERROR_STATUS_SCRIPT_UNKNOWN_VAR_TYPE;
			
		case 0x4007:
			return ERROR::ERROR_STATUS_SCRIPT_VAR_UNDEFINED;
			
		case 0x4008:
			return ERROR::ERROR_STATUS_SCRIPT_INVALID_OPT_ARG;
			
		case 0x4009:
			return ERROR::ERROR_STATUS_SCRIPT_INVALID_VERSION;
			
		case 0x7FFF:
			return ERROR::ERROR_STATUS_FATAL_ERROR;
			
		default:
			return ERROR_UNKNOWN;
	}
	return ERROR_UNKNOWN;
}

MeasurementError::ERROR MeasurementError::getError() const{
	return e;
}
int MeasurementError::getLine() const{
	return line;
}
int MeasurementError::getCol() const{
	return column;
}
std::string MeasurementError::getDescr() const{
	return errorToStr(e);
}