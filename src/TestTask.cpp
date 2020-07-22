#include "TestTask.h"
#include "DataP.h"
#include "TransSpect.h"

#include <iostream>
#include <vector>
#include <math.h>

TestTask::TestTask(): Task(){
	
}
TestTask::TestTask(std::string name): TestTask(){
	Task::name = name;
}

TestTask::~TestTask(){
	
}

void TestTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	std::cout << "executing Task " << name << " ..." << std::endl;
	TransSpect::TransSpect_ptr spectrums = TransSpect::create();
	data->addTransImpedanceSpectrum(spectrums);
	
	for (int i = 0; i < 40; i++){
		
		TransSpect::Spectrum s;
		for (int j = 0; j < 800; j++){
			double x_val = j+1;
			double y_val_real = i + (j+1);
			double y_val_imag = (i + 1) * 0.5 + (j+1) * 4;
			s.push_back(std::make_shared<DataP>(x_val, y_val_real, y_val_imag));
			
		}
		spectrums->addSpectrum(s);
		sleep(1);
	}
}

std::string TestTask::getType() const{
	return "testTask";
}

tinyxml2::XMLElement* TestTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("TestTask");
	
	return xmlTaskElement;
}

std::list<Task::DEVICES> TestTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	
	return devices;
}