#include "ImpAnalyserTask.h"
#include "Novocontrol.h"
#include "HP4294A.h"
#include "EmStatPico.h"

#include <iostream>

ImpAnalyserTask::ImpAnalyserTask(ImpAnalyser::ImpAnalyser_ptr analyser): Task(), impAnalyser(analyser){
	setName(impAnalyser->to_string());
}
ImpAnalyserTask::~ImpAnalyserTask(){
	
}

void ImpAnalyserTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	try{
		relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
		//relais->writeRelais();
		//usleep(51*1000);
		relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, true); //Imp measurement
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to imp measurement", Log_Event::LOG_INFO));
		
		usleep(51*1000);
		
		if (impAnalyser->getInternal()){ // internal imp analyser
			relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, false); //internal measurement
			relais->writeRelais();
			addLogEvent(Log_Event::create("switched relais", "switched relais to internal imp measurement", Log_Event::LOG_INFO));
		}else{
			relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, true); //external measurement
			relais->writeRelais();
			addLogEvent(Log_Event::create("switched relais", "switched relais to external imp measurement", Log_Event::LOG_INFO));
		}
		

		
		switch(impAnalyser->getWireMode()){
		case ImpAnalyser::WIRE_MODE::TWO_WIRE:
			relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, true);
			relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, true);
			break;
			
		case ImpAnalyser::WIRE_MODE::THREE_WIRE:
			relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, false);
			relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, true);
			break;
			
		default:
			relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, false);
			relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, false);
			break;
		}
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to configure " + std::to_string(impAnalyser->getWireMode()) + "-wire mode", Log_Event::LOG_INFO));
		
		
		status_leds->impMeasRunning(true);
		status_leds->write_reg_val();
		
		addLogEvent(Log_Event::create("imp. measurement start", impAnalyser->getType() + " triggered to measure - " + getParams(), Log_Event::LOG_INFO));
		data->addImpedanceSpectrum(impAnalyser->measureSpectrum(executeNext));
		addLogEvent(Log_Event::create("imp. measurement done", impAnalyser->getType() + " is done - " + getParams(), Log_Event::LOG_INFO));
		
		//relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
		relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, true); //external measurement
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to external imp measurement", Log_Event::LOG_INFO));
		
		usleep(51*1000);
		
		relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, false); //Imp measurement
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to ewod mode", Log_Event::LOG_INFO));
		
		status_leds->impMeasRunning(false);
		status_leds->write_reg_val();
	}catch(std::runtime_error r){
		addLogEvent(Log_Event::create(impAnalyser->getType() + " error", r.what(), Log_Event::TYPE::LOG_ERROR));
		throw; // rethrow the exception
	}
}
std::string ImpAnalyserTask::getType() const {
	return "ImpAnalyserTask - " + impAnalyser->getType();
}
tinyxml2::XMLElement* ImpAnalyserTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("ImpAnalyserTask");
	
	xmlTaskElement->InsertEndChild(impAnalyser->toXMLElement(doc, externElements));
	
	return xmlTaskElement;
}

ImpAnalyserTask::ImpAnalyserTask_ptr ImpAnalyserTask::loadImpAnalyserTask(tinyxml2::XMLElement* task_element){
	ImpAnalyserTask_ptr p;
	
	tinyxml2::XMLNode* elem = task_element->FirstChild();
	
	if (elem != nullptr){
		tinyxml2::XMLElement* childElement = dynamic_cast<tinyxml2::XMLElement*>(elem);
		if (childElement != nullptr){
			if (std::string(childElement->Name()).compare("Novocontrol") == 0){
				p = std::make_shared<ImpAnalyserTask>(Novocontrol::loadNovocontrol(childElement));
			}else if (std::string(childElement->Name()).compare("HP4294A") == 0){
				p = std::make_shared<ImpAnalyserTask>(HP4294A::loadHP4294A(childElement));
			}else if (std::string(childElement->Name()).compare("EmStatPico") == 0){
				p = std::make_shared<ImpAnalyserTask>(EmStatPico::loadEmStatPico(childElement));
			}
			
		}
	}
	return p;
}

std::string ImpAnalyserTask::getParams() const{
	std::string params = "";
	params.append(std::to_string(impAnalyser->getStartFrequency())).append("Hz-").append(std::to_string(impAnalyser->getStopFrequency())).append("Hz");
	params.append(" - ").append(std::to_string(impAnalyser->getVoltage())).append("V");
	params.append(" - ").append(std::to_string(impAnalyser->getPoints())).append("pts");
	
	return params;
}

std::list<Task::DEVICES> ImpAnalyserTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	
	Task::DEVICES d;
	switch (impAnalyser->getAnalyserType()){
		case ImpAnalyser::ANALYSER_DEVICE::ANALYSER_EMPICO:
			d = Task::DEVICES::DEVICE_EMPICO;
			devices.push_back(d);
			break;
			
		case ImpAnalyser::ANALYSER_DEVICE::ANALYSER_HP4294A:
			d = Task::DEVICES::DEVICE_HP4294A;
			devices.push_back(d);
			break;
			
		case ImpAnalyser::ANALYSER_DEVICE::ANALYSER_NOVOCONTROL:
			d = Task::DEVICES::DEVICE_NOVOCONTROL;
			devices.push_back(d);
			break;
	}
	
	return devices;
}
