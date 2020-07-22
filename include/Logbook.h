#pragma once
/**
 * @file Logbook.h
 * 
 * @class Logbook
 * @author Nils Bosbach
 * @date 15.04.2019
 * @brief class which can store Log_Event objects
 */
#include "Log_Event.h"

#include <list>
#include <string>
#include <mutex>
#include <memory>

class Logbook{
public:
	
	/// smart pointer
	typedef std::shared_ptr<Logbook> Logbook_ptr;
	
	/**
	 * @brief constructor - initialize variables
	 */
	Logbook();
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~Logbook();
	
	/**
	 * @brief create a new Logbook object
	 * @return smart pointer to the created object
	 */
	static Logbook_ptr create();
	
	/**
	 * @brief add a event to the logbook
	 * @param e event which should be onLogEventAdded
	 */
	void add_event(Log_Event::Log_Event_ptr e);
	
	/**
	 * @brief saves the logbook as file
	 * @param path path where the logfile should be stored
	 */
	int save_log_file(std::string path);
	
	/**
	 * @brief set a listener, which will be called each time an event is onLogEventAdded
	 * @param callback listener
	 */
	void set_onLogEventAdded(void (*callback)(Log_Event::Log_Event_ptr));
	
	/**
	 * @brief set a temporary logbook, which will be filled with the same entries as the normal one
	 * @param l the temporary logbook
	 */
	void set_temp_Logbook(Logbook_ptr l);
	
private:
	std::list<Log_Event::Log_Event_ptr> events;
	std::mutex eventsMtx;
	Logbook_ptr tempLogbook;
	
	void (*onLogEventAdded)(Log_Event::Log_Event_ptr); //callback function
};
