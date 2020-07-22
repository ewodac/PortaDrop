#include "Logbook.h"

#include <iostream>
#include <fstream>

Logbook::Logbook(){
	onLogEventAdded = nullptr;
	Logbook::add_event(Log_Event::create("log started", "", Log_Event::TYPE::LOG_INFO));
}

Logbook::~Logbook(){
	
}
Logbook::Logbook_ptr Logbook::create(){
	return std::make_shared<Logbook>();
}

int Logbook::save_log_file(std::string path){
	std::ofstream file;
	file.open(path);
	
	file << "***Header***\n";
	
	file << "_________________________________________________________________\n"; //divider
	
	Logbook::eventsMtx.lock();
	for(std::list<Log_Event::Log_Event_ptr>::const_iterator it = Logbook::events.cbegin(); it != Logbook::events.cend(); it++){
		file << (*(*it));
		file << "\n";
	}
	Logbook::eventsMtx.unlock();
	
	file.close();
	return 0;
}

void Logbook::add_event(Log_Event::Log_Event_ptr e){
	Logbook::eventsMtx.lock();
	Logbook::events.insert(Logbook::events.end(), e);
	Logbook::eventsMtx.unlock();
//	std::cout << "Log event added - " << e->getName() << std::endl;
	if (tempLogbook != nullptr){
		tempLogbook->add_event(e);
	}
	if (onLogEventAdded != nullptr){
		onLogEventAdded(e);
	}
}
void Logbook::set_onLogEventAdded(void (*callback)(Log_Event::Log_Event_ptr)){
	onLogEventAdded = callback;
}
void Logbook::set_temp_Logbook(Logbook::Logbook_ptr l){
	tempLogbook = l;
}
