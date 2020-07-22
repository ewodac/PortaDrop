#include "Logbook.h"
#include "GUI.h"

#include <wiringPi.h>
#include <iostream>
#include <string>

GUI *gui;

void onLogEventAdded(Log_Event::Log_Event_ptr event){
	gui->addLogEvent(event);
}

int main (int argc, char **argv){
	
	wiringPiSetup(); //init wiring pi
	Logbook::Logbook_ptr logfile = Logbook::create();
	gui = new GUI(logfile);
	
	logfile->set_onLogEventAdded(&onLogEventAdded);
	
	Task::setLogFile(logfile);
	
	
	gui->run();
	delete gui;
	return 0;
}
