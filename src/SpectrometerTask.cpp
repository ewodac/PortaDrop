#include "SpectrometerTask.h"
#include "FSHelper.h"

#include <stdexcept>

SpectrometerTask::SpectrometerTask(Spectrometer *s, Spectrometer::TRIGGER_MODE triggerMode, unsigned long integrationTimeMicros, unsigned int scansToAverage): Task(), spectrometer(s), triggerMode(triggerMode), integrationTimeMicros(integrationTimeMicros), scansToAverage(scansToAverage){
	std::string name = "";
	name += "int. time: " + FSHelper::formatDouble(integrationTimeMicros) + "us";
	name += " - scans: " + FSHelper::formatDouble(scansToAverage);
	setName(name);
}

SpectrometerTask::~SpectrometerTask(){
	
}

void SpectrometerTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	if (spectrometer != nullptr){
		try{
			spectrometer->setIntegrationTimeMicros(integrationTimeMicros);
			spectrometer->setTriggerMode(triggerMode);
			spectrometer->setScansToAverage(scansToAverage);
			
			addLogEvent(Log_Event::create("requesting spectrum", getSpectrometerParams(), Log_Event::TYPE::LOG_INFO));
			if (data != nullptr) data->addSpectrum(spectrometer->getFormattedSpectrum_DataPoints());
			addLogEvent(Log_Event::create("received spectrum", getSpectrometerParams(), Log_Event::TYPE::LOG_INFO));
		}catch(std::runtime_error r){
			addLogEvent(Log_Event::create("spectrometer error", r.what(), Log_Event::TYPE::LOG_ERROR));
			throw; // rethrow the exception
		}
	}else{
		addLogEvent(Log_Event::create("spectrometer error", "spectrometer is nullptr", Log_Event::TYPE::LOG_ERROR));
	}
}

tinyxml2::XMLElement* SpectrometerTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("SpectrometerTask");
	xmlTaskElement->SetAttribute("triggermode", std::to_string(triggerMode).c_str());
	xmlTaskElement->SetAttribute("integrationtime_us", std::to_string(integrationTimeMicros).c_str());
	xmlTaskElement->SetAttribute("scansToAverage", std::to_string(scansToAverage).c_str());
	
	return xmlTaskElement;
}

std::string SpectrometerTask::getType() const{
	return "SpectrometerTask";
}

std::string SpectrometerTask::getSpectrometerParams() const{
	std::string params = "";
	
	params += "integrationtime: " + std::to_string(integrationTimeMicros) + "us " + "\t";
	params += "trigger mode: " + Spectrometer::triggerModeToString(triggerMode) + "\t";
	params += "scans to average: " + std::to_string(scansToAverage);
	
	return params;
}


SpectrometerTask::SpectrometerTask_ptr SpectrometerTask::loadSpectrometerTask(tinyxml2::XMLElement* task_element, Spectrometer *s){
	Spectrometer::TRIGGER_MODE triggerMode = Spectrometer::TRIGGER_MODE::TRIGGER_MODE_NORMAL;
	unsigned long integrationtime_us = 1000;
	unsigned int scansToAverage = 1;
	
	if (task_element->FindAttribute("triggermode")){
		triggerMode = static_cast<Spectrometer::TRIGGER_MODE>(task_element->FindAttribute("triggermode")->IntValue());
	}
	
	if (task_element->FindAttribute("integrationtime_us")){
		integrationtime_us = task_element->FindAttribute("integrationtime_us")->Int64Value();
	}
	
	if (task_element->FindAttribute("scansToAverage")){
		scansToAverage = task_element->FindAttribute("scansToAverage")->IntValue();
	}
	
	return std::make_shared<SpectrometerTask>(s, triggerMode, integrationtime_us, scansToAverage);
}

std::list<Task::DEVICES> SpectrometerTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	Task::DEVICES d = Task::DEVICES::DEVICE_SPECTROMETER;
	devices.push_back(d);
	
	return devices;
}
