#include "Log_Event.h"

#include <iostream>


Log_Event::Log_Event(std::string name, std::string description, Log_Event::TYPE t){
	Log_Event::time = std::time(0); // now
	Log_Event::log_type = t;
	Log_Event::name = name;
	Log_Event::description = description;
}

Log_Event::~Log_Event(){
	
}

Log_Event::Log_Event_ptr Log_Event::create(std::string name, std::string description, TYPE t){
	return std::make_shared<Log_Event>(name, description, t);
}

std::time_t Log_Event::getTime() const{
	return Log_Event::time;
}
std::string Log_Event::getTime_toString() const{
	std::tm *ptm = std::localtime(&time);
	char buffer[9];
	
	std::strftime(buffer, 9, "%H:%M:%S", ptm);
	return buffer;
}
std::string Log_Event::getName() const{
	return Log_Event::name;
}
std::string Log_Event::getDescription() const{
	return Log_Event::description;
}
Log_Event::TYPE Log_Event::getType() const{
	return Log_Event::log_type;
}
std::string Log_Event::getType_toString() const{
	switch(log_type){
		case Log_Event::TYPE::LOG_ERROR:
			return "error";
		case Log_Event::TYPE::LOG_INFO:
			return "info";
		default:
			return "default";
	}
	return "";
}

std::ostream& operator<<(std::ostream& stream, const Log_Event& event){
	stream << event.toString();
	return stream;
}

std::string Log_Event::toString() const{
	std::string returnString = "";
	
	returnString += fillStringToLength(getTime_toString(), 12);
	returnString += fillStringToLength(getType_toString(), 13);
	returnString += fillStringToLength(getName(), 30);
	returnString += getDescription();
	
	return returnString;
}

std::string Log_Event::fillStringToLength(std::string s, unsigned int length){
	while(s.length() < length){
		s += " ";
	}
	return s;
}
