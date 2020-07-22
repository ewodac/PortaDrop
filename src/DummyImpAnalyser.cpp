#include "DummyImpAnalyser.h"
#include "FSHelper.h"

#include <iostream>
#include <algorithm>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

DummyImpAnalyser::DummyImpAnalyser(): ImpAnalyser(){
	voltage = 0.05;
	setStartFrequency(100);
	setStopFrequency(4000000);
	setPoints(100);
	setPointAverage(1);
}
DummyImpAnalyser::~DummyImpAnalyser(){
	
}


DataP::DataP_ptr DummyImpAnalyser::measureFreq(double freq){
	return std::make_shared<DataP>(freq, rand() % ((int) freq), rand() % ((int) freq / 7));
}

std::vector<DataP::DataP_ptr> DummyImpAnalyser::measureSpectrum(bool *running){
	
	std::vector<DataP::DataP_ptr> data;
	double startFrequency_log = std::log10(startFrequency);
	double stopFrequency_log = std::log10(stopFrequency);
	double range_log = stopFrequency_log - startFrequency_log;
	double step_log = (points > 1 ? range_log / (points-1): 0);
	
	
	for (int i = 0; (i < points) && (*running); i++){
		double freq = std::pow(10, startFrequency_log + i * step_log);
		
		//measure 'pointAverage' times and calculate the average
		DataP::DataP_ptr p = measureFreq(freq);
		double x = p->getX();
		double y_real = p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
		double y_imag = p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
		
		for (int i = 1; i < pointAverage; i++){
			p = measureFreq(freq);
			y_real += p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
			y_imag += p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
		}
		y_real /= pointAverage;
		y_imag /= pointAverage;
		data.push_back(DataP::create(x, y_real, y_imag));
	}
	usleep(500 * 1000);
	return data;
}

tinyxml2::XMLElement* DummyImpAnalyser::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("DummyImpAnalyser");
	
	xmlTaskElement->SetAttribute("startFrequency", std::to_string(getStartFrequency()).c_str());
	xmlTaskElement->SetAttribute("stopFrequency", std::to_string(getStopFrequency()).c_str());
	xmlTaskElement->SetAttribute("voltage", std::to_string(getVoltage()).c_str());
	xmlTaskElement->SetAttribute("points", std::to_string(getPoints()).c_str());
	xmlTaskElement->SetAttribute("pointAverage", std::to_string(getPointAverage()).c_str());
	
	return xmlTaskElement;
}
DummyImpAnalyser::DummyImpAnalyser_ptr DummyImpAnalyser::loadDummyImpAnalyser(tinyxml2::XMLElement* task_element){
	DummyImpAnalyser_ptr t;
	
	t = std::make_shared<DummyImpAnalyser>();
	
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
	
	return t;
}

int DummyImpAnalyser::getMinFreq() const{
	return MIN_FREQ;
}
int DummyImpAnalyser::getMaxFreq() const {
	return MAX_FREQ;
}
int DummyImpAnalyser::getMinPoints() const{
	return MIN_POINTS;
}
int DummyImpAnalyser::getMaxPoints() const{
	return MAX_POINTS;
}
int DummyImpAnalyser::getMaxPointAverage() const{
	return MAX_POINT_AVERAGE;
}
double DummyImpAnalyser::getMinVolt() const{
	return MIN_VOLT;
}
double DummyImpAnalyser::getMaxVolt() const{
	return MAX_VOLT;
}
std::string DummyImpAnalyser::getType() const{
	return "DummyImpAnalyser";
}

ImpAnalyser::ANALYSER_DEVICE DummyImpAnalyser::getAnalyserType() const{
	return ImpAnalyser::ANALYSER_DEVICE::ANALYSER_DUMMY;
}


bool DummyImpAnalyser::getInternal() const{
	return true;
}