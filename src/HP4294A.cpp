#include "HP4294A.h"
#include "FSHelper.h"

#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <math.h>
#include <time.h>
#include <algorithm>

HP4294A::HP4294A(int slaveAddress): connection(slaveAddress){
	startFrequency = 100;
	stopFrequency = 110000000;
	voltage = 0.05;
	points = 200;
	bw = 2;
	pointAverage = 1;
}
HP4294A::~HP4294A(){
	
}

void HP4294A::applyParams(){
	applyStopFrequency();
	applyStartFrequency();
	connection.send("SWPP FREQ; POWMOD VOLT; FORM4; SWPT LOG");
	applyVoltage();
	applyBW();
	applyPointAverage();
	applyPoints();
}
void HP4294A::applyStopFrequency(){
	connection.send("STOP " + std::to_string(stopFrequency) + "hz");
}
void HP4294A::applyStartFrequency(){
	connection.send("STAR " + std::to_string(startFrequency) + "hz");
}
void HP4294A::applyVoltage(){
	std::string voltage_string = std::to_string(voltage);
	std::replace(voltage_string.begin(), voltage_string.end(), ',', '.'); // replace , with .
	
	connection.send("POWE " + voltage_string);
}
void HP4294A::applyBW(){
	connection.send("BWFACT " + std::to_string(bw));
}
void HP4294A::applyPointAverage(){
	connection.send("PAVERFACT " + std::to_string(pointAverage));
}
void HP4294A::applyPoints(){
	connection.send("POIN " + std::to_string(points));
}

void HP4294A::triggerMeasurement(){
	connection.send("SPLD ON; MEAS IMPH; TRAC B; FMT LINY; AUTO; TRAC A; FMT LOGY; AUTO; TRGS INT; SING"); 
}
void HP4294A::waitUntilMeasurementFinished(){
	bool running = true;
	
	connection.send("*OPC?");
	while (running){
		usleep(1000*100);
		std::string response = connection.read();
		try{
			int number = std::atoi(response.c_str());
			if (number == 1){
				running = false;
			}
		}catch (std::invalid_argument const &e){
			std::cout << "nan" << std::endl;
		}
	}
}

std::vector<DataP::DataP_ptr> HP4294A::measureSpectrum(bool *running){
	applyParams();
	triggerMeasurement();
	waitUntilMeasurementFinished();
	connection.send("MEAS IMPH; TRAC B; FMT LINY; TRAC A; FMT LOGY; AUTO");
	
	std::vector<DataP::DataP_ptr> data;
	std::string sweep_val = "";
	std::string measurement_val = "";
	std::string measurement_val_real = "";
	std::string measurement_val_im = "";
	for (int i = 1; (i <= points) && (*running); i++){
		connection.send("OUTPSWPRMP? " + std::to_string(i));
		sweep_val = connection.read();
		sweep_val = sweep_val.substr(0, sweep_val.find_first_of("\n")); // cut off the \n<^END>
		
		connection.send("OUTPDATAP? " + std::to_string(i));
		measurement_val = connection.read();
		measurement_val = measurement_val.substr(0, measurement_val.find_first_of("\n")); // cut off the \n<^END>
		
		measurement_val_real = measurement_val.substr(0, measurement_val.find(','));
		measurement_val_im = measurement_val.substr(measurement_val.find(',') + 1, measurement_val.size()-1);
		
		double measurement_val_im_d = FSHelper::sciToDouble(measurement_val_im);
		double measurement_val_real_d = FSHelper::sciToDouble(measurement_val_real);
		double sweep_val_d = FSHelper::sciToDouble(sweep_val);
		
		data.push_back(std::make_shared<DataP>(sweep_val_d, measurement_val_real_d, measurement_val_im_d));
	}
	return data;
}


tinyxml2::XMLElement* HP4294A::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("HP4294A");
	
	xmlTaskElement->SetAttribute("gpibAddress", std::to_string(connection.getSlaveAddress()).c_str());
	xmlTaskElement->SetAttribute("startFrequency", std::to_string(getStartFrequency()).c_str());
	xmlTaskElement->SetAttribute("stopFrequency", std::to_string(getStopFrequency()).c_str());
	xmlTaskElement->SetAttribute("voltage", std::to_string(getVoltage()).c_str());
	xmlTaskElement->SetAttribute("points", std::to_string(getPoints()).c_str());
	xmlTaskElement->SetAttribute("bw", std::to_string(getBw()).c_str());
	xmlTaskElement->SetAttribute("pointAverage", std::to_string(getPointAverage()).c_str());
	xmlTaskElement->SetAttribute("wireMode", std::to_string(getWireMode()).c_str());
	
	return xmlTaskElement;
}
HP4294A::HP4294A_ptr HP4294A::loadHP4294A(tinyxml2::XMLElement* task_element){
	HP4294A_ptr t;
	
	if (task_element->FindAttribute("gpibAddress")){
		t = std::make_shared<HP4294A>(task_element->FindAttribute("gpibAddress")->IntValue());
	}
	
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
	
	if (task_element->FindAttribute("bw") && t != nullptr){
		t->setBw(task_element->FindAttribute("bw")->IntValue());
	}
	
	if (task_element->FindAttribute("pointAverage") && t != nullptr){
		t->setPointAverage(task_element->FindAttribute("pointAverage")->IntValue());
	}
	
	if (task_element->FindAttribute("wireMode") && t != nullptr){
		t->setWireMode(static_cast<WIRE_MODE>(task_element->FindAttribute("wireMode")->IntValue()));
	}
	
	return t;
}

//double* HP4294A::getFrequencies(){
//	send("OUTPSWPRM?");
//	std::string frequencies = "";
//	
//	do{
//		frequencies.append(read());
//		std::cout << std::endl << std::endl << frequencies << std::endl;
//	}while(frequencies.find('\n') == std::string::npos);
//	
//	return nullptr;
//}

unsigned short HP4294A::getBw() const{
	return bw;
}

void HP4294A::setBw(unsigned short bw){ //range 1-5
	if(bw < 1){
		bw = 1;
	}
	if(bw > 5){
		bw = 5;
	}
	HP4294A::bw = bw;
}

int HP4294A::getMinFreq() const{
	return MIN_FREQ;
}
int HP4294A::getMaxFreq() const {
	return MAX_FREQ;
}
int HP4294A::getMinPoints() const{
	return MIN_POINTS;
}
int HP4294A::getMaxPoints() const{
	return MAX_POINTS;
}
int HP4294A::getMaxPointAverage() const{
	return MAX_POINT_AVERAGE;
}
double HP4294A::getMinVolt() const{
	return MIN_VOLT;
}
double HP4294A::getMaxVolt() const{
	return MAX_VOLT;
}
std::string HP4294A::getType() const{
	return "HP4294A";
}

std::string HP4294A::to_string() const{
	std::string return_string = ImpAnalyser::to_string();
	
	return return_string;
}

ImpAnalyser::ANALYSER_DEVICE HP4294A::getAnalyserType() const{
	return ImpAnalyser::ANALYSER_DEVICE::ANALYSER_HP4294A;
}

bool HP4294A::getInternal() const{
	return false;
}