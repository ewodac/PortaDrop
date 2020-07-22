#include "PadTask.h"
#include "Addresses.h"

#include <string>
#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <algorithm> //std::find
#include <math.h>

//static variables
std::mutex PadTask::executeMtx;
bool PadTask::executing = false;

PadTask::PadTask(std::vector<int> pads, int duration_ms): Task(){
	PadTask::pads = pads;
	PadTask::duration_ms = duration_ms;
	
	//set up name
	std::string name = "power pad";
	if (pads.size() > 1) name += "s";
	name += " ";
	for(std::vector<int>::const_iterator cit = pads.cbegin(); cit != pads.cend(); cit++){
		name += std::to_string(*cit);
		name += ", ";
	}
	name = name.substr(0, name.length() - 2); // remove last ', '
	name += " for ";
	name += std::to_string(duration_ms);
	name += "ms";
	PadTask::name = name;
	
	//set outputs
	pinMode(GPIO_ADC_0, OUTPUT);
	pinMode(GPIO_ADC_1, OUTPUT);
	pinMode(GPIO_ADC_2, OUTPUT);
	pinMode(GPIO_ADC_3, OUTPUT);
	
	pinMode(GPIO_EN_1, OUTPUT);
	pinMode(GPIO_EN_2, OUTPUT);
	pinMode(GPIO_EN_3, OUTPUT);
	pinMode(GPIO_EN_4, OUTPUT);
	pinMode(GPIO_EN_5, OUTPUT);
	
	pinMode(GPIO_CS_1, OUTPUT);
	pinMode(GPIO_CS_2, OUTPUT);

}

PadTask::~PadTask(){
	
}
PadTask::PadTask_ptr PadTask::create(std::vector<int> pads, int duration_ms){
	return std::make_shared<PadTask>(pads, duration_ms);
}

void PadTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	executeMtx.lock();
	if (executing == false){ // not executing at the moment
		executing = true;
		executeMtx.unlock();
		
		relais->setRelais(Relais::RELAIS::R_STEUER_AC, true);
		relais->writeRelais();
		
		Task::status_leds->pad(true);
		Task::status_leds->write_reg_val();
		std::string loginfo = "pads ";
		for (std::vector<int>::iterator it = pads.begin(); it != pads.end(); it++){
				//std::cout << "power on pad " << *it << std::endl;
				setPadHigh(*it);
				loginfo += std::to_string(*it);
				loginfo += ", ";
		}
		loginfo = loginfo.substr(0, loginfo.length()-2);
		loginfo += " for " + std::to_string(duration_ms) + "ms";
		addLogEvent(Log_Event::create("pad on", loginfo, Log_Event::TYPE::LOG_INFO));
		
		delay(duration_ms);
		
		setPadsLow();
		Task::status_leds->pad(false);
		Task::status_leds->write_reg_val();
		addLogEvent(Log_Event::create("pads off", "setting all pads to low", Log_Event::TYPE::LOG_INFO));
//		loginfo = "pads ";
//		for (std::vector<int>::reverse_iterator rit = pads.rbegin(); rit != pads.rend(); rit++){
//			//std::cout << "power off pad " << *rit << std::endl;
//			loginfo += *rit;
//			loginfo += ", ";
//		}
//		addLogEvent(Log_Event::create("pad off", loginfo, Log_Event::TYPE::LOG_INFO));
		executeMtx.lock();
		executing = false;
		executeMtx.unlock();
	}else{ //error, executing at the moment
		executeMtx.unlock();
		addLogEvent(Log_Event::create("already executing", "already powering pads", Log_Event::TYPE::LOG_ERROR));
	}
}

std::string PadTask::getType() const{
	return "PadTask";
}

PadTask::PadTask_ptr PadTask::getPadTaskFromString(std::string text){
	/**
	 * text format:
	 * <time in ms>;<pad 1>;<pad 2>; ...
	 */
	PadTask_ptr p;
	
	if (!text.empty() && text.find(';') != std::string::npos){ // check format
		std::vector<int> pads; // will include all pads which should be turned on
		double time; // time the pads will be powered
		std::string substring = text.substr(0, text.find(';')); // text from the beginning of the string to the first semi colon
		
		text = text.substr(text.find(';')+1, text.length()-1); // remove the substring from text
		time = atof(substring.c_str()); // get the time (first param)
		
		while (!text.empty()){
			if (text.find(';') != std::string::npos){ //not last pad
				substring = text.substr(0, text.find(';'));
				text = text.substr(text.find(';')+1, text.length()-1);
			}else{ //last pad
				substring = text;
				text = "";
			}
			int pad = atoi(substring.c_str());
			if (std::find(pads.begin(), pads.end(), pad) == pads.end()){ // pad has not been added yet
				pads.push_back(pad);
			}
		}
		p = std::make_shared<PadTask>(pads, time); //create PadTask
	}else{ //format error
		///@todo handle format error
	}
	return p;
}


tinyxml2::XMLElement* PadTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("PadTask");
	xmlTaskElement->SetAttribute("name", name.c_str());
	xmlTaskElement->SetAttribute("comment", comment.c_str());
	xmlTaskElement->SetAttribute("duration_ms", duration_ms);
	
	for (std::vector<int>::const_iterator cit = pads.cbegin(); cit != pads.cend(); cit++){
		tinyxml2::XMLElement* xmlTaskElement_pad = doc->NewElement("Pad");
		xmlTaskElement_pad->SetAttribute("padNo", *cit);
		xmlTaskElement->InsertEndChild(xmlTaskElement_pad);
	}
	
	return xmlTaskElement;
}


void PadTask::setPadHigh(unsigned int pad){
	/*
	 * 1 circuit borad: 4 uc x 12 pads * 1 uc x 11 pads = 59 pads
	 * first pad is pad 1 !!
	 * first pcb is pcb 1 !!
	 * first uc is uc 1 !!
	 * 
	 */
	//std::cout << "powering pad " << pad << std::endl;
	if (pad <= 118 && pad > 0){ //valid pad
		
		int pcb_no; // 1, 2
		int uc_no;  // 1, .., 5
		int pad_no; // 1, .., 59
		
		pcb_no = std::ceil(((double)pad) / 59.0); // 1, 2
		uc_no = std::ceil((((double)pad) - double((pcb_no - 1) * 59.0))/ 12.0); // 1, .., 5
		pad_no = pad - ((pcb_no - 1)*59 + (uc_no - 1) * 12); // 1, .. , 12
		
		
		
		std::cout << "powering pad " << pad_no << " of uc " << uc_no << " on pcb " << pcb_no << std::endl; 
		
		bool ADC_0 = (((pad_no >> 0) & 1) == 1);
		bool ADC_1 = (((pad_no >> 1) & 1) == 1);
		bool ADC_2 = (((pad_no >> 2) & 1) == 1);
		bool ADC_3 = (((pad_no >> 3) & 1) == 1);
		
		bool EN_1 = (uc_no == 1);
		bool EN_2 = (uc_no == 2);
		bool EN_3 = (uc_no == 3);
		bool EN_4 = (uc_no == 4);
		bool EN_5 = (uc_no == 5);
		
		bool CS_1 = (pcb_no == 1);
		bool CS_2 = (pcb_no == 2);
		
		digitalWrite(GPIO_ADC_0, ADC_0);
		digitalWrite(GPIO_ADC_1, ADC_1);
		digitalWrite(GPIO_ADC_2, ADC_2);
		digitalWrite(GPIO_ADC_3, ADC_3);
		digitalWrite(GPIO_CS_1, CS_1);
		digitalWrite(GPIO_CS_2, CS_2);
		
		delayMicroseconds(20);
		digitalWrite(GPIO_EN_1, EN_1);
		digitalWrite(GPIO_EN_2, EN_2);
		digitalWrite(GPIO_EN_3, EN_3);
		digitalWrite(GPIO_EN_4, EN_4);
		digitalWrite(GPIO_EN_5, EN_5);
		
		
//		std::cout << "Address set" << std::endl;
		delayMicroseconds(20);
//		delayMicroseconds(4e6);
//		delay(20000);
		
		digitalWrite(GPIO_EN_1, LOW);
		digitalWrite(GPIO_EN_2, LOW);
		digitalWrite(GPIO_EN_3, LOW);
		digitalWrite(GPIO_EN_4, LOW);
		digitalWrite(GPIO_EN_5, LOW);
		digitalWrite(GPIO_CS_1, LOW);
		digitalWrite(GPIO_CS_2, LOW);
		
//		std::cout << "EN LOW" << std::endl;
	}
}

void PadTask::setPadsLow(){
	digitalWrite(GPIO_ADC_0, LOW);
	digitalWrite(GPIO_ADC_1, LOW);
	digitalWrite(GPIO_ADC_2, LOW);
	digitalWrite(GPIO_ADC_3, LOW);
	
	delayMicroseconds(20);
	digitalWrite(GPIO_EN_1, HIGH);
	digitalWrite(GPIO_EN_2, HIGH);
	digitalWrite(GPIO_EN_3, HIGH);
	digitalWrite(GPIO_EN_4, HIGH);
	digitalWrite(GPIO_EN_5, HIGH);
	
	digitalWrite(GPIO_CS_1, HIGH);
	digitalWrite(GPIO_CS_2, HIGH);
	
	
//	std::cout << "Pad clear" << std::endl;
	delayMicroseconds(30);
//	delay(20000);
	
	digitalWrite(GPIO_EN_1, LOW);
	digitalWrite(GPIO_EN_2, LOW);
	digitalWrite(GPIO_EN_3, LOW);
	digitalWrite(GPIO_EN_4, LOW);
	digitalWrite(GPIO_EN_5, LOW);
	
	digitalWrite(GPIO_CS_1, LOW);
	digitalWrite(GPIO_CS_2, LOW);
//	std::cout << "EN low" << std::endl;
}

PadTask::PadTask_ptr PadTask::loadPadTask(tinyxml2::XMLElement* task_element){
	std::vector<int> pads;
	int duration_ms; 
	tinyxml2::XMLElement *pad = task_element->FirstChildElement("Pad");
	
	if (task_element->FindAttribute("duration_ms")){
		duration_ms = task_element->FindAttribute("duration_ms")->IntValue();
	}
	
	//add all pads
	while (pad != nullptr){
		if (pad->FindAttribute("padNo")){
			pads.push_back(pad->FindAttribute("padNo")->IntValue());
		}
		pad = pad->NextSiblingElement("Pad");
	}
	
	return std::make_shared<PadTask>(pads, duration_ms);
}

std::list<Task::DEVICES> PadTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	
	return devices;
}
