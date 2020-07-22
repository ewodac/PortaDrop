#include "EmStatPico.h"
#include "MeasurementValue.h"
#include "MeasurementError.h"

#include <wiringSerial.h>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <string>

EmStatPico::EmStatPico(){
	startFrequency = 100;
	stopFrequency = 220000;
	voltage = 0.05;
	points = 200;
	pointAverage = 1;
}
EmStatPico::~EmStatPico(){
	
}
bool EmStatPico::waitForSerialData(int fd){
	double timeoutCnt = 0.0;
	while (!serialDataAvail(fd) && timeoutCnt < TIMEOUT_S){
		usleep(20e3);
		timeoutCnt += 20e-3;
	}
	return timeoutCnt < TIMEOUT_S;
}
void EmStatPico::startMeasurement(int fd, double f_min, double f_max, double current_min, double current_max){
	serialPrintf(fd, "e\n"); // start of the method script
	serialPrintf(fd, "var h\n");
	serialPrintf(fd, "var r\n");
	serialPrintf(fd, "var j\n");
	serialPrintf(fd, "set_pgstat_chan 0\n"); //Select channel 0
	serialPrintf(fd, "set_pgstat_mode 3\n"); //High speed mode is required for EIS
//	serialPrintf(fd, "set_cr 1m\n"); //Set current range for currents of up to 1 mA
	
	std::string cmd_currentRange = "set_autoranging " + formatNumber(current_min) + " " + formatNumber(current_max) + " \n";
	serialPrintf(fd, cmd_currentRange.c_str());
	
	serialPrintf(fd, "cell_on\n"); //Cell must be on to do measurements
	
	std::string cmd =  "meas_loop_eis h r j " + formatNumber(getVoltage()) + " " + formatNumber(f_min) + " " + formatNumber(f_max) + " " + formatNumber(getPoints()) + " 0\n";
//	std::cout << cmd << std::endl;
	
	serialPrintf(fd, cmd.c_str()); //Run actual EIS measurement
	serialPrintf(fd, "pck_start\n"); //Send measurement package containing frequency, Z-real and Z-imaginary
	serialPrintf(fd, "pck_add h\n");
	serialPrintf(fd, "pck_add r\n");
	serialPrintf(fd, "pck_add j\n");
	serialPrintf(fd, "pck_end\n");
	serialPrintf(fd, "endloop\n");
	serialPrintf(fd, "on_finished:\n"); //urn cell off when finished or aborted
	serialPrintf(fd, "cell_off\n");
	serialPrintf(fd, "\n"); // end of the method script
}
EmStatPico::MeasurementList_ptr EmStatPico::receiveMeasurements(int fd){
	std::string line = "";
	bool receiving = true;
	MeasurementList_ptr packages = std::make_shared<MeasurementList>();
	
	if (waitForSerialData(fd)){ // timeout not reached
		while (receiving && serialDataAvail(fd)) {
			char c = serialGetchar(fd);
			if (c == '\n'){
				if (line.empty()){ // end of return 
					receiving = false;
				}else{ // line is not empty
					std::cout << "pico: " << line << std::endl;
					switch(line.at(0)){
						case 'P':{ // package
							packages->push_back(MeasurementPackage::create(line));
							break;
						}
							
						case '!':{ // error
							MeasurementError::MeasurementError_ptr e = MeasurementError::create(line);
							throw std::runtime_error(e->getDescr());
							break;
						}
						
						default :
							std::cout << "received unhandled line: " << line << std::endl;
					}
					line = "";
				}
			}else{
				line += c;
			}
			if (receiving){
				waitForSerialData(fd);
			}
		}
	}else { // timeout reached
		throw std::runtime_error("timeout reached - no response from emstat pico");
	}
	return packages;
}
std::vector<DataP::DataP_ptr> EmStatPico::measureSpectrum(bool* running){
	int fd;
	
	if ((fd = serialOpen ("/dev/serial0", 230400)) < 0){
		throw std::runtime_error("Unable to open serial device: %s\n");
	}
	
	startMeasurement(fd, getStartFrequency(), getStopFrequency());
	MeasurementList_ptr measurements = receiveMeasurements(fd);
	std::vector<DataP::DataP_ptr> spectrum;
	
	//for all measurement packages
	for(MeasurementList::iterator it = measurements->begin(); it != measurements->end(); it++){
		MeasurementPackage::MeasurementPackage_ptr package = *it;
		MeasurementPackage::MeasurementList_ptr measuredValues = package->getMeasurements();
		double x, y_real, y_imag;
		bool x_found = false, y_real_found = false, y_imag_found = false;
		bool data_valid = true;
		int currentRange = -1;
		MeasurementValue::META_STATUS status = MeasurementValue::META_STATUS::STATUS_NO_STATUS;
		
		// for all captured values of the measurement package
		for (MeasurementPackage::MeasurementList::iterator it_measurement = measuredValues->begin(); it_measurement != measuredValues->end(); it_measurement++){
			MeasurementValue::MeasurementValue_ptr value = *it_measurement;
			if(value->getCurrentRange() != -1){
				currentRange = value->getCurrentRange();
			}
			
			if (value->getStatus() == MeasurementValue::META_STATUS::STATUS_OK || value->getStatus() == MeasurementValue::META_STATUS::STATUS_NO_STATUS || value->getStatus() == MeasurementValue::META_STATUS::STATUS_OVERLOAD_WARNING){
				switch (value->getType()){
				case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_ZREAL:
					y_real_found = true;
					y_real = value->getValue();
					break;
					
				case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_ZIMAG:
					y_imag_found = true;
					y_imag = value->getValue();
					break;
					
				case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_CELL_FREQUENCY:
					x_found = true;
					x = value->getValue();
					break;
				}
			}else{ //Status not okay
				status = value->getStatus();
				data_valid = false;
				std::cout << "received measurement with status " << MeasurementValue::statusToString(value->getStatus()) << std::endl;
			}
		}
		//STATUS_OK, STATUS_OVERLOAD, STATUS_UNDERLOAD, STATUS_OVERLOAD_WARNING, STATUS_NO_STATUS
		
		// try to remeasure
//		if (!data_valid){
//			bool error = false;
//			bool direction_upwards;
//			
//			switch (status){
//				case MeasurementValue::META_STATUS::STATUS_OVERLOAD_WARNING:
//				case MeasurementValue::META_STATUS::STATUS_OVERLOAD:
//					direction_upwards = false;
//					break;
//					
//				case MeasurementValue::META_STATUS::STATUS_UNDERLOAD:
//					direction_upwards = true;
//					break;
//					
//				default:
//					error = true;
//			}
//			
//			
//			if (!error && x_found && currentRange != -1){ // no error, valid current ragne and freq information in last measurement (x_found == true)
//				bool nextCrAvailable = true;
//				
//				while (!data_valid && nextCrAvailable){
//					int cr_nextMeasurement;
//					
//					if (direction_upwards){
//						cr_nextMeasurement = getNextCurrentRange(currentRange);
//					}else{
//						cr_nextMeasurement = getPrevCurrentRange(currentRange);
//					}
//					
//					if ( cr_nextMeasurement != -1){
//						std::string cr_nextMeasurement_str = formatNumber(cr_nextMeasurement);
//						startMeasurement(fd, x, x, cr_nextMeasurement, cr_nextMeasurement);
//						MeasurementList_ptr measurements = receiveMeasurements(fd);
//						
//						if (measurements->size() == 1){
//							MeasurementPackage::MeasurementPackage_ptr package = *(measurements->begin());
//							MeasurementPackage::MeasurementList_ptr measuredValues = package->getMeasurements();
//							data_valid = true;
//							
//							for (MeasurementPackage::MeasurementList::iterator it_measurement = measuredValues->begin(); it_measurement != measuredValues->end(); it_measurement++){
//								MeasurementValue::MeasurementValue_ptr value = *it_measurement;
//								if (value->getStatus() == MeasurementValue::META_STATUS::STATUS_OK || value->getStatus() == MeasurementValue::META_STATUS::STATUS_NO_STATUS ){
//									switch (value->getType()){
//									case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_ZREAL:
//										y_real_found = true;
//										y_real = value->getValue();
//										break;
//										
//									case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_ZIMAG:
//										y_imag_found = true;
//										y_imag = value->getValue();
//										break;
//										
//									case MeasurementValue::MEASUREMENT_TYPE::TYPE_VT_CELL_FREQUENCY:
//										x_found = true;
//										x = value->getValue();
//										break;
//									}
//								}else{ //Status not okay
//									status = value->getStatus();
//									data_valid = false;
//									std::cout << "received measurement with status " << MeasurementValue::statusToString(value->getStatus()) << std::endl;
//									std::cout << "old cr: " << MeasurementValue::currentRangeToStr(currentRange) << "\t new cr: " << MeasurementValue::currentRangeToStr(value->getCurrentRange()) << std::endl; 
//								}
//							}
//							
//						}else{ // received more than one measurement
//							
//						}
//					}else {
//						nextCrAvailable = false;
//					}
//				}
//			}
//		}
		
		if (data_valid){
			if (x_found){ // got freq value
				if (y_real_found){ // got no z_real value
					if (y_imag_found){ // got no z_imag value
						spectrum.push_back(DataP::create(x, y_real, y_imag));
					}else{ // got no z_imag value
						throw std::runtime_error("got no z_imag value");
					}
				}else{ // got no z_real value
					throw std::runtime_error("got no z_real value");
				}
			}else{ // got no freq value
				throw std::runtime_error("got no freq value");
			}
		}else{
			std::cout << "skipped invalid measurement" << std::endl;
		}
		
	}
	
	
	
	serialClose(fd);
	
//	for(std::list<MeasurementPackage::MeasurementPackage_ptr>::iterator it = packages.begin(); it != packages.end(); it++){
//		MeasurementPackage::MeasurementPackage_ptr m = *it;
//		std::cout << m->to_string() << std::endl;
//	}
	
	return spectrum;
}
std::string EmStatPico::getType() const{
	return "EmStat pico";
}
tinyxml2::XMLElement* EmStatPico::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("EmStatPico");
	
	xmlTaskElement->SetAttribute("startFrequency", std::to_string(getStartFrequency()).c_str());
	xmlTaskElement->SetAttribute("stopFrequency", std::to_string(getStopFrequency()).c_str());
	xmlTaskElement->SetAttribute("voltage", std::to_string(getVoltage()).c_str());
	xmlTaskElement->SetAttribute("points", std::to_string(getPoints()).c_str());
	xmlTaskElement->SetAttribute("pointAverage", std::to_string(getPointAverage()).c_str());
	xmlTaskElement->SetAttribute("wireMode", std::to_string(getWireMode()).c_str());
	
	return xmlTaskElement;
}

ImpAnalyser::ANALYSER_DEVICE EmStatPico::getAnalyserType() const{
	return ImpAnalyser::ANALYSER_DEVICE::ANALYSER_EMPICO;
}
EmStatPico::EmStatPico_ptr EmStatPico::loadEmStatPico(tinyxml2::XMLElement* task_element){
	EmStatPico_ptr t = std::make_shared<EmStatPico>();
	
	if (task_element->FindAttribute("startFrequency") && t != nullptr){
		t->setStartFrequency(task_element->FindAttribute("startFrequency")->IntValue());
	}
	
	if (task_element->FindAttribute("stopFrequency") && t != nullptr){
		t->setStopFrequency(task_element->FindAttribute("stopFrequency")->IntValue());
	}
	
	if (task_element->FindAttribute("voltage") && t != nullptr){
		t->setVoltage(task_element->FindAttribute("voltage")->IntValue());
	}
	
	if (task_element->FindAttribute("points") && t != nullptr){
		t->setPoints(task_element->FindAttribute("points")->IntValue());
	}
	
	if (task_element->FindAttribute("pointAverage") && t != nullptr){
		t->setPointAverage(task_element->FindAttribute("pointAverage")->IntValue());
	}
	
	if (task_element->FindAttribute("wireMode") && t != nullptr){
		t->setWireMode(static_cast<WIRE_MODE>(task_element->FindAttribute("wireMode")->IntValue()));
	}
	
	return t;
}
std::string EmStatPico::to_string() const{
	return "";
}
int EmStatPico::getMinFreq() const{
	return 1;
}
int EmStatPico::getMaxFreq() const{
	return 200000;
}
int EmStatPico::getMinPoints() const{
	return 1;
}
int EmStatPico::getMaxPoints() const{
	return 500;
}
int EmStatPico::getMaxPointAverage() const{
	return 10;
}
double EmStatPico::getMinVolt() const{
	return 1e-6; //?
}
double EmStatPico::getMaxVolt() const{
	return 200e-3; //?
}
std::string EmStatPico::formatNumber(double i){
	int sign = (i < 0 ? -1 : 1);
	int exp = 0;
	i = i * sign; // i is positive
	
	if (i > 999){
		while (i > 999){
			i /= 1e3;
			exp += 3;
		}
	}else if (i < 1){
		while (i < 1){
			i *= 1000;
			exp -= 3;
		}
	}
	int i_int = (int) i;
	char exp_char = ' ';
	switch (exp){
		case -18:
			exp_char = 'a';
			break;
			
		case -15:
			exp_char = 'f';
			break;
			
		case -12:
			exp_char = 'p';
			break;
			
		case -9:
			exp_char = 'n';
			break;
			
		case -6:
			exp_char = 'u';
			break;
			
		case -3:
			exp_char = 'm';
			break;
			
		case 0:
			exp_char = ' ';
			break;
			
		case 3:
			exp_char = 'k';
			break;
			
		case 6:
			exp_char = 'M';
			break;
			
		case 9:
			exp_char = 'G';
			break;
			
		case 12:
			exp_char = 'T';
			break;
			
		case 15:
			exp_char = 'P';
			break;
			
		case 18:
			exp_char = 'E';
			break;
	}
	return (sign < 0 ? "-" : "") + std::to_string(i_int) + (exp_char != ' ' ? std::string(1, exp_char) : "");
}
bool EmStatPico::getInternal() const{
	return true;
}
int EmStatPico::getNextCurrentRange(int cr){
	switch (cr){
		case 0: // 100nA
			return 2e-6;
			
		case 1: // 2uA
			return 4e-6;
			
		case 2: // 4uA
			return 8e-6;
			
		case 3: // 8uA
			return 16e-6;
			
		case 4: // 16uA
			return 32e-6;
			
		case 5: // 32uA
			return 63e-6;
			
		case 6: // 63uA
			return 125e-6;
			
		case 7: // 125uA
			return 250e-6;
			
		case 8: // 250uA
			return 500e-6;
			
		case 9: // 500uA
			return 1e-3;
			
		case 10: // 1mA
			return 15e-3;
			
		case 11: // 15mA
			return -1;
			
		case 128: // 100nA (High speed)
			return 1e-6;
			
		case 129: // 1uA (High speed)
			return 6e-6;
			
		case 130: // 6uA (High speed)
			return 13e-6;
			
		case 131: // 13uA (High speed)
			return 25e-6;
			
		case 132: // 25uA (High speed)
			return 50e-6;
			
		case 133: // 50uA (High speed)
			return 100e-6;
			
		case 134: // 100uA (High speed)
			return 200e-6;
			
		case 135: // 200uA (High speed)
			return 1e-3;
			
		case 136: // 1mA (High speed)
			return 5e-3;
			
		case 137: // 5mA (High speed)
			return -1; 
			
		default:
			return -1;
	}
	return -1;
}
int EmStatPico::getPrevCurrentRange(int cr){
	switch (cr){
		case 0: // 100nA
			return -1;
			
		case 1: // 2uA
			return 100e-6;
			
		case 2: // 4uA
			return 2e-6;
			
		case 3: // 8uA
			return 4e-6;
			
		case 4: // 16uA
			return 8e-6;
			
		case 5: // 32uA
			return 16e-6;
			
		case 6: // 63uA
			return 32e-6;
			
		case 7: // 125uA
			return 63e-6;
			
		case 8: // 250uA
			return 125e-6;
			
		case 9: // 500uA
			return 250e-6;
			
		case 10: // 1mA
			return 500e-6;
			
		case 11: // 15mA
			return 1e-3;
			
		case 128: // 100nA (High speed)
			return -1;
			
		case 129: // 1uA (High speed)
			return 100e-9;
			
		case 130: // 6uA (High speed)
			return 1e-6;
			
		case 131: // 13uA (High speed)
			return 6e-6;
			
		case 132: // 25uA (High speed)
			return 13-6;
			
		case 133: // 50uA (High speed)
			return 25e-6;
			
		case 134: // 100uA (High speed)
			return 50e-6;
			
		case 135: // 200uA (High speed)
			return 100e-6;
			
		case 136: // 1mA (High speed)
			return 200e-6;
			
		case 137: // 5mA (High speed)
			return 1e-3; 
			
		default:
			return -1;
	}
	return -1;
}
