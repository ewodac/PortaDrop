#include "Novocontrol.h"
#include "FSHelper.h"

#include <iostream>
#include <algorithm>
#include <math.h>

Novocontrol::Novocontrol(int slaveAddress): connection(slaveAddress){
	voltage = 0.05;
	setStartFrequency(100);
	setStopFrequency(4000000);
	setPoints(100);
	setPointAverage(1);
}
Novocontrol::~Novocontrol(){
	
}

void Novocontrol::sendOKCommand(std::string command){
	connection.send(command);
	if (connection.read().compare("OK") != 0){ // failed
		throw std::runtime_error(std::string("error setting novocontrol param - gpib: ").append(command));
	}
}

void Novocontrol::applyWireMode(){
	std::string command = "FRS=" + std::to_string(wire_mode);
	sendOKCommand(command);
	
	command = "MODE=IMP";
	sendOKCommand(command);
}
void Novocontrol::applyAcVoltageAmplitude(){
	/* ACV = %1f1 Set AC-Voltage [Vrms]
	 * Sets the ac generator voltage amplitude
	 * Answer: OK
	 * Range:
	 * 0V..3V for freq <= 1e7Hz
	 * 0V..1V for freq > 1e7Hz
	 */
	std::string voltage_string = std::to_string(voltage);
	std::replace(voltage_string.begin(), voltage_string.end(), ',', '.'); // replace , with .
	
	std::string command = "ACV=" + voltage_string;
	sendOKCommand(command);
}

void Novocontrol::applyParams(){
	applyWireMode();
	
	/*
	 * SRE			Set Service Request Mask Register
	 * MTM			Set Measurement Integration Time [s] -  0 for the shortest available integration time
	 * IAC			Set V1, V2, AC / DC Coupling 1st param: 0(ac coupling off (dc coupling)), 1(ac coupling on) 2nd param: 1(input channel 1) 2 (input channel2)
	 * ZLLCOR		Enable Low Loss Correction 0(off) 1(on)
	 * ZSLCAL		Enable Low Impedance Load Short Calibration 0(off) 1(on)
	 * ZREFMODE?		Retruns Reference Mode for impedance measurements
	 */
	std::string command = "SRE=65; MTM=0; IAC= 1 1; ZLLCOR=1; ZSLCAL=1; ZREFMODE?)";
	sendOKCommand(command);
	
	
	applyAcVoltageAmplitude();
	
}

DataP::DataP_ptr Novocontrol::measureFreq(double freq){
	/* GFR=%1 Set Frequency
	 * %f1 Frequency [Hz]
	 * Answer OK
	 * Range: 3e-6 - 3e5(Alpha AL)
	 * Range: 3e-6 - 3e6(Alpha AK)
	 * Range: 3e-6 - 2e7(Alpha AN)
	 * Range: 3e-6 - 4e7(Alpha AT)
	 */
	sendOKCommand("GFR=" + std::to_string(freq));
	 
	/* MST Trigger Measurement
	 * Answer OK
	 */
	sendOKCommand("MST");
	
	/* ZTSTAT? Return Impedance Measurement Task State
	 * Returns the state of the las or actual impedance measurement or calibration task
	 * Answer: ZTSTAT=%i1 %i2
	 * %i1: Identifier of the task in progress or last terminated task
	 * 0: No task was started until now
	 * 1: Impedance Measurement
	 * ...
	 * 
	 * %2: State of task in progress or last terminated task
	 * 0: Not activated
	 * 1: stopped by user break (MBK)
	 * 2: stopped by error
	 * 3: Successfully terminated
	 * 4: Paused
	 * 5: Running
	 * ...
	 */
	 
	//wait until measurement finished
	connection.send("ZTSTAT?");
	std::string state = connection.read();
	while (state.compare("ZTSTAT=1 5") == 0){
		connection.send("ZTSTAT?");
		state = connection.read();
	}
	if (state.compare("ZTSTAT=1 3") != 0){ // not completed successfully
		throw new std::runtime_error("error during measurement - received: " + state);
	}
	
	/* ZRE? Return Measured Impedance
	 * Returns the contents of the Alpha impedance measurement result buffer
	 * Answer: ZRE=%f1 %f2 %f3 %i4 %i5
	 * %f1: Real part of serial sample impedance [ohm]
	 * %f2: Imaginary part of serial sample impedance [ohm]
	 * %f3: Frequency of measured point [Hz]
	 * %i4: Result status
	 * 	0: invalid (buffer empty)
	 * 	1: measurement still in progress
	 * 	2: valid
	 * 	3: Voltage V1 for sample measurement out of range
	 * 	4: Current out of range
	 * 	5: Voltage V1 for reference measurement out of range
	 * 	6: Analyzer signal source disconnected within measurement
	 * %i5: Reference measurement status
	 * 	0: reference measurement disabled
	 * 	1: reference measurement enabled
	 */
	connection.send("ZRE?");
	std::string imp_Meausrement = connection.read(); //e.g. ZRE=6.714035e+02 -1.029636e+02 4.999991e+02 2 0
	imp_Meausrement = imp_Meausrement.substr(4); // cut ZRE=
	
	std::string imp_Meausrement_real = imp_Meausrement.substr(0, imp_Meausrement.find_first_of(" "));
	imp_Meausrement = imp_Meausrement.substr(imp_Meausrement.find_first_of(" ") + 1);
	
	std::string imp_Meausrement_imag = imp_Meausrement.substr(0, imp_Meausrement.find_first_of(" "));
	imp_Meausrement = imp_Meausrement.substr(imp_Meausrement.find_first_of(" ") + 1);
	
	std::string imp_Meausrement_freq = imp_Meausrement.substr(0, imp_Meausrement.find_first_of(" "));
	imp_Meausrement = imp_Meausrement.substr(imp_Meausrement.find_first_of(" ") + 1);
	
	std::string imp_Meausrement_res = imp_Meausrement.substr(0, imp_Meausrement.find_first_of(" "));
	imp_Meausrement = imp_Meausrement.substr(imp_Meausrement.find_first_of(" ") + 1);
	
	std::string imp_Meausrement_ref = imp_Meausrement;
	
	double imp_Meausrement_real_double = FSHelper::sciToDouble(imp_Meausrement_real);
	double imp_Meausrement_imag_double = FSHelper::sciToDouble(imp_Meausrement_imag);
	double imp_Meausrement_freq_double = FSHelper::sciToDouble(imp_Meausrement_freq);
	int imp_Meausrement_res_int = std::stoi(imp_Meausrement_res);
	int imp_Meausrement_ref_int =  std::stoi(imp_Meausrement_ref);
	
	// check if measurement is valid
	switch(imp_Meausrement_res_int){
	case 0: // invalid (buffer empty)
		throw std::runtime_error("measurement error - invalid (buffer empty)");
		break;
	case 1: // measurement still in progress
		throw std::runtime_error("measurement error - measurement still in progress");
		break;
	case 2: // valid
		break;
	case 3: // Voltage V1 for sample measurement out of range
		throw std::runtime_error("measurement error - Voltage V1 for sample measurement out of range");
		break;
	case 4: // Current out of range
		throw std::runtime_error("measurement error - Current out of range");
		break;
	case 5: // Voltage V1 for reference measurement out of range
		throw std::runtime_error("measurement error - Voltage V1 for reference measurement out of range");
		break;
	case 6: // Analyzer signal source disconnected within measurement
		throw std::runtime_error("measurement error - Analyzer signal source disconnected within measurement");
		break;
	}
//	std::cout << imp_Meausrement_real_double << std::endl  << imp_Meausrement_imag_double << std::endl << imp_Meausrement_freq_double << std::endl<< imp_Meausrement_res_int << std::endl << imp_Meausrement_ref_int << std::endl;
	return std::make_shared<DataP>(imp_Meausrement_freq_double, imp_Meausrement_real_double, imp_Meausrement_imag_double);
}

std::vector<DataP::DataP_ptr> Novocontrol::measureSpectrum(bool *running){
	applyParams();
	
	std::vector<DataP::DataP_ptr> data;
	double startFrequency_log = std::log10(startFrequency);
	double stopFrequency_log = std::log10(stopFrequency);
	double range_log = stopFrequency_log - startFrequency_log;
	double step_log = (points > 1 ? range_log / (points-1): 0);
	
	applyParams();
	
	for (int i = 0; (i < points) && (*running); i++){ // for each frequency
		double freq = std::pow(10, startFrequency_log + i * step_log);
		
		//measure 'pointAverage' times and calculate the average
		DataP::DataP_ptr p = measureFreq(freq);
		double x = p->getX();
		double y_real = p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
		double y_imag = p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
		
		for (int i = 1; i < pointAverage; i++){ // measure #pointAverage -1 times and calculate the average
			p = measureFreq(freq);
			y_real += p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
			y_imag += p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
		}
		y_real /= pointAverage;
		y_imag /= pointAverage;
		data.push_back(DataP::create(x, y_real, y_imag));
	}
	return data;
}

tinyxml2::XMLElement* Novocontrol::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("Novocontrol");
	
	xmlTaskElement->SetAttribute("gpibAddress", std::to_string(connection.getSlaveAddress()).c_str());
	xmlTaskElement->SetAttribute("startFrequency", std::to_string(getStartFrequency()).c_str());
	xmlTaskElement->SetAttribute("stopFrequency", std::to_string(getStopFrequency()).c_str());
	xmlTaskElement->SetAttribute("voltage", std::to_string(getVoltage()).c_str());
	xmlTaskElement->SetAttribute("points", std::to_string(getPoints()).c_str());
	xmlTaskElement->SetAttribute("pointAverage", std::to_string(getPointAverage()).c_str());
	xmlTaskElement->SetAttribute("wireMode", std::to_string(getWireMode()).c_str());
	
	return xmlTaskElement;
}
Novocontrol::Novocontrol_ptr Novocontrol::loadNovocontrol(tinyxml2::XMLElement* task_element){
	Novocontrol_ptr t;
	
	if (task_element->FindAttribute("gpibAddress")){
		t = std::make_shared<Novocontrol>(task_element->FindAttribute("gpibAddress")->IntValue());
	}
	
	if (task_element->FindAttribute("startFrequency") && t != nullptr){
		t->setStartFrequency(task_element->FindAttribute("startFrequency")->IntValue());
	}
	
	if (task_element->FindAttribute("stopFrequency") && t != nullptr){
		t->setStopFrequency(task_element->FindAttribute("stopFrequency")->IntValue());
	}
	
	if (task_element->FindAttribute("voltage") && t != nullptr){
		t->setVoltage(task_element->FindAttribute("voltage")->DoubleValue());
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

std::string Novocontrol::to_string() const{
	std::string return_string = ImpAnalyser::to_string() + ", ";
	
	return_string += std::to_string(wire_mode) + "W";
	
	return return_string;
}

int Novocontrol::getMinFreq() const{
	return MIN_FREQ;
}
int Novocontrol::getMaxFreq() const {
	return MAX_FREQ;
}
int Novocontrol::getMinPoints() const{
	return MIN_POINTS;
}
int Novocontrol::getMaxPoints() const{
	return MAX_POINTS;
}
int Novocontrol::getMaxPointAverage() const{
	return MAX_POINT_AVERAGE;
}
double Novocontrol::getMinVolt() const{
	return MIN_VOLT;
}
double Novocontrol::getMaxVolt() const{
	return MAX_VOLT;
}
std::string Novocontrol::getType() const{
	return "Novocontrol";
}

ImpAnalyser::ANALYSER_DEVICE Novocontrol::getAnalyserType() const{
	return ImpAnalyser::ANALYSER_DEVICE::ANALYSER_NOVOCONTROL;
}

bool Novocontrol::getInternal() const{
	return false;
}