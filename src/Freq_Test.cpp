#include "Freq_Test.h"
#include "I2CFreqTask.h"
#include "FSHelper.h"

#include <string>
#include <unistd.h>
#include <iostream>
#include <fstream>

Freq_Test::Freq_Test(): conn(8){
	
}
Freq_Test::Freq_Test_ptr Freq_Test::create(){
	return std::make_shared<Freq_Test>();
}

void Freq_Test::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	I2CFreqTask::I2CFreqTask_ptr freqgen = I2CFreqTask::create(2);
	double f_min = 2;
	double f_max = 100000;
	int points = 200;
	int pointAverage = 4;
	
	
	double startFrequency_log = std::log10(f_min);
	double stopFrequency_log = std::log10(f_max);
	double range_log = stopFrequency_log - startFrequency_log;
	double step_log = (points > 1 ? range_log / (points-1): 0);
	
	conn.send("MEASUREMENT:IMMED:TYPE FREQUENCY");
	conn.send("MEASUREMENT:IMMED:SOURCE1 CH1");
	std::ofstream file;
	file.open("/home/pi/data.txt");
	
	for (int i = 0; (i < points) && (*executeNext); i++){ // for each frequency
		int freq = std::pow(10, startFrequency_log + i * step_log);
		freqgen->setFrequency(freq);
		freqgen->execute(0, executeNext);
		usleep(1000000);
		
		double measured = 0;
		
		for (int i = 0; i < pointAverage; i++){ // measure #pointAverage times and calculate the average
			conn.send("MEASUREMENT:IMMED:VALUE?");
			std::string value = conn.read();
			measured += FSHelper::sciToDouble(value);
			usleep(5000);
		}
		measured /= pointAverage;
		std::cout << "freq: " << measured << std::endl;
		file << std::to_string(freq) << ";" << std::to_string(measured) << std::endl;
	}
	file.close();
}
std::string Freq_Test::getType() const{
	return "Freq Test";
}
tinyxml2::XMLElement* Freq_Test::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("FreqTest");
//	xmlTaskElement->SetAttribute("name", name.c_str());
	
	return xmlTaskElement;
}
std::list<Task::DEVICES> Freq_Test::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	Task::DEVICES d = Task::DEVICES::DEVICE_ATTINY_FREQ;
	devices.push_back(d);
	
	return devices;
}