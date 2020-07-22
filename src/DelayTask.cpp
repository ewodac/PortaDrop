#include "DelayTask.h"
#include "FSHelper.h"

#include <iostream>
#include <unistd.h>
#include <chrono>

DelayTask::DelayTask(unsigned int delayTimeS, unsigned int delayTimeMs): Task(), delay_time_s(delayTimeS), delay_time_ms(delayTimeMs){
	std::string name = formatTime();
	
	name += " delay";
	setName(name);
}

DelayTask::DelayTask_ptr DelayTask::create(unsigned int delayTimeS, unsigned int delayTimeMs){
	return std::make_shared<DelayTask>(delayTimeS, delayTimeMs);
}

DelayTask::~DelayTask(){
	
}

void DelayTask::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
//	if (*executeNext){
//		addLogEvent(Log_Event::create("delaying", "delaying for " + std::to_string(delay_time_us) + "us", Log_Event::TYPE::LOG_INFO));
//		usleep(delay_time_us);
//	}
	const std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
	double elapsed_secs = 0;
	
	addLogEvent(Log_Event::create("delaying", "delaying for " + formatTime(), Log_Event::TYPE::LOG_INFO));
	
	while (elapsed_secs < delay_time_s && (*executeNext)){
		const std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
		const std::chrono::duration<double> elapsed_secs_duration = current_time - start_time;
		elapsed_secs = elapsed_secs_duration.count();
		usleep(100);
	}
	usleep(delay_time_ms * 1000);
}

std::string DelayTask::getType() const{
	return "DelayTask";
}

tinyxml2::XMLElement* DelayTask::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("DelayTask");
	xmlTaskElement->SetAttribute("delay_time_s", std::to_string(delay_time_s).c_str());
	
	return xmlTaskElement;
}

DelayTask::DelayTask_ptr DelayTask::loadDelayTask(tinyxml2::XMLElement* task_element){
	unsigned int delay_time_s = 10;
	
	if (task_element->FindAttribute("delay_time_s")){
		delay_time_s = task_element->FindAttribute("delay_time_s")->IntValue();
	}
	
	return std::make_shared<DelayTask>(delay_time_s);
}

std::list<Task::DEVICES> DelayTask::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	return devices;
}

std::string DelayTask::formatTime() const{
	std::string time = "";
	if (delay_time_s > 0){
		time += FSHelper::formatTime(delay_time_s);
	}
	if (delay_time_ms > 0){
		if (delay_time_s > 0){
			time += " ";
		}
		time += std::to_string(delay_time_ms) + "ms";
	}
	
	return time;
}