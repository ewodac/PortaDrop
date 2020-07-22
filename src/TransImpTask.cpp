#include "TransImpTask.h"
#include "TransSpect.h"
#include "FSHelper.h"
#include "Novocontrol.h"
#include "HP4294A.h"

#include <chrono>

TransImpTask::TransImpTask(ImpAnalyser::ImpAnalyser_ptr analyser, unsigned int termination, TERMINATION_MODE t): Task(), analyser(analyser), termination(termination), termMode(t){
	setName(to_string());
}

void TransImpTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) {
	relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, true); //Imp measurement
	relais->writeRelais();
	addLogEvent(Log_Event::create("switched relais", "switched relais to imp measurement", Log_Event::LOG_INFO));
	usleep(51*1000);
	
	switch(analyser->getWireMode()){
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
	addLogEvent(Log_Event::create("switched relais", "switched relais to configure " + std::to_string(analyser->getWireMode()) + "-wire mode", Log_Event::LOG_INFO));
	
	if (analyser->getInternal()){ // internal imp analyser
//		relais->setRelais(Relais::RELAIS::R_STEUER_BOOST_IN, true); //internal measurement
		relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, false); //internal measurement
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to internal imp measurement", Log_Event::LOG_INFO));
	}else{
//		relais->setRelais(Relais::RELAIS::R_STEUER_BOOST_IN, false); //external measurement
		relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, true); //external measurement
		relais->writeRelais();
		addLogEvent(Log_Event::create("switched relais", "switched relais to external imp measurement", Log_Event::LOG_INFO));
	}
	
	
	TransSpect::TransSpect_ptr spectrums = TransSpect::create();
	data->addTransImpedanceSpectrum(spectrums);
	
	status_leds->impMeasRunning(true);
	status_leds->write_reg_val();
	
	
	if (termMode == TERMINATION_MODE::TERM_CNT){
		for (int i = 0; (i < termination) && (*executeNext); i++){
			spectrums->addSpectrum(analyser->measureSpectrum(executeNext));
			spectrums->progress = ((double) (i+1)) / (double (termination));
			addLogEvent(Log_Event::create("spectrum captured", "transient measurement - captured spectrum " + std::to_string(i+1) + " of " + std::to_string(termination), Log_Event::TYPE::LOG_INFO));
		}
	}else if (termMode == TERMINATION_MODE::TERM_TIME){
		const std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
		double elapsed_secs = 0;
		int i = 1;
		
		while (elapsed_secs < termination && (*executeNext)){
			spectrums->addSpectrum(analyser->measureSpectrum(executeNext));
			
			const std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
			const std::chrono::duration<double> elapsed_secs_duration = current_time - start_time;
			elapsed_secs = elapsed_secs_duration.count();
			addLogEvent(Log_Event::create("spectrum captured", "transient measurement - captured spectrum " + std::to_string(i++) + "  -  " + FSHelper::formatTime(elapsed_secs) + " elapsed of " + FSHelper::formatTime(termination), Log_Event::TYPE::LOG_INFO));
			
			spectrums->progress = (elapsed_secs) / (double (termination));
		}
	}
	spectrums->progress = 1;
	
	status_leds->impMeasRunning(false);
	status_leds->write_reg_val();
	
	relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, false); //Imp measurement
	relais->writeRelais();
	addLogEvent(Log_Event::create("switched relais", "switched relais to ewod mode", Log_Event::LOG_INFO));
}

std::string TransImpTask::getType() const{
	return "TransImpTask - " + analyser->getType();
}

tinyxml2::XMLElement* TransImpTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("TransImpTask");
	
	if(termMode == TERMINATION_MODE::TERM_CNT){
		xmlTaskElement->SetAttribute("measurements_term_cnt", std::to_string(termination).c_str());
	}else if(termMode == TERMINATION_MODE::TERM_TIME){
		xmlTaskElement->SetAttribute("measurements_term_time", std::to_string(termination).c_str());
	}
	
	xmlTaskElement->InsertEndChild(analyser->toXMLElement(doc, externElements));
	
	return xmlTaskElement;
}
TransImpTask::TransImpTask_ptr TransImpTask::loadTransImpTask(tinyxml2::XMLElement* task_element){
	TransImpTask_ptr p;
	tinyxml2::XMLNode* elem = task_element->FirstChild();
	
	if (elem != nullptr){
		tinyxml2::XMLElement* childElement = dynamic_cast<tinyxml2::XMLElement*>(elem);
		if (childElement != nullptr){
			if (std::string(childElement->Name()).compare("Novocontrol") == 0){
				p = std::make_shared<TransImpTask>(Novocontrol::loadNovocontrol(childElement));
			}else if (std::string(childElement->Name()).compare("HP4294A") == 0){
				p = std::make_shared<TransImpTask>(HP4294A::loadHP4294A(childElement));
			}else{ // no imp analyser specified
				///@todo exception?
			}
			if (task_element->FindAttribute("measurements_term_cnt")){
				p->setTermination(task_element->FindAttribute("measurements_term_cnt")->IntValue(), TERMINATION_MODE::TERM_CNT);
			}
			if (task_element->FindAttribute("measurements_term_time")){
				p->setTermination(task_element->FindAttribute("measurements_term_time")->IntValue(), TERMINATION_MODE::TERM_TIME);
			}
		}
	}
	return p;
}

void TransImpTask::setTermination(unsigned int value, TERMINATION_MODE mode){
	termination = value;
	termMode = mode;
	setName(to_string());
}
const unsigned int TransImpTask::getTerminationNumber() const{
	return termination;
}
const TransImpTask::TERMINATION_MODE TransImpTask::getTerminationMode() const{
	return termMode;
}

std::string TransImpTask::to_string() const{
	std::string retString = "";
	switch (termMode){
		case TERMINATION_MODE::TERM_CNT:
			retString += std::to_string(termination) + " spectrums";
			break;
			
		case TERMINATION_MODE::TERM_TIME:
			retString += FSHelper::formatTime(termination);
			break;
	}
	
	if (analyser != nullptr){
		retString += " -- " + analyser->to_string();
	}
	
	return retString;
}

std::list<Task::DEVICES> TransImpTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	
	Task::DEVICES d;
	switch (analyser->getAnalyserType()){
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