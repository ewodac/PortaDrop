#include "Task.h"

#include <iostream>


// static variables
int Task::max_id = 0;
std::mutex Task::idMtx;
Logbook::Logbook_ptr Task::logfile;
StatusLed::StatusLed_ptr Task::status_leds;
Relais::Relais_ptr Task::relais;

Task::Task(){
	Task::idMtx.lock(); //lock the mutex to synchronize access on max_id
	id = Task::max_id++; // get unique id 
	Task::idMtx.unlock(); //unlock the mutex
}
Task::Task(Logbook::Logbook_ptr l): Task(){
	logfile = l;
}
Task::Task(const Task& t){
	id = t.getID();
	name = t.getName();
	comment = t.getComment();
}

Task::~Task(){
//	std::cout << "deleted Task - " << getName() << std::endl;
}

const int Task::getID() const{
	return id;
}

void Task::setComment(std::string comment){
	Task::comment = comment;
}

std::string Task::getComment() const{
	return Task::comment;
}

void Task::setName(std::string name){
	Task::name = name;
}

std::string Task::getName() const{
	return Task::name;
}

void Task::addLogEvent(Log_Event::Log_Event_ptr l){
	if (logfile != nullptr){
		logfile->add_event(l);
	}
}
void Task::setLogFile(Logbook::Logbook_ptr l){
	logfile = l;
}