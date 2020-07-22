#pragma once
/**
 * @file Task.h
 * 
 * @class Task
 * @author Nils Bosbach
 * @date 14.04.2019
 * @brief implements one job as part of a Recipe
 * @see Recipe
 */
#include "Logbook.h"
#include "ExperimentData.h"
#include "StatusLed.h"
#include "Relais.h"

#include <mutex>
#include <pthread.h>
#include <string>
#include <tinyxml2.h>
#include <memory>
#include <list>

class Task {
public:
	typedef std::shared_ptr<Task> Task_ptr;
	
	/// devices, which can connected to the raspberry pi
	enum DEVICES {DEVICE_ATTINY_VOLT, DEVICE_ATTINY_FREQ, DEVICE_ATMEGA_GENERAL, DEVICE_NOVOCONTROL, DEVICE_HP4294A, DEVICE_EMPICO, DEVICE_SPECTROMETER};
	
	/**
	 * @brief constructor
	 * 
	 * the unique id is obtained here
	 */
	Task();
	
	/**
	 * @brief constructor
	 * @param l logfile which stores errors and information of the tasks
	 * 
	 * the unique id is obtained here
	 * logfile is static -> just need to be set once
	 * 
	 * @todo remove this constructor -> logfile is static
	 */
	Task(Logbook::Logbook_ptr l);
	
	/**
	 * @brief copy constructor
	 * @param t the Task which should be copied
	 */
	Task(const Task& t);
	
	/**
	 * @brief virtual destructor
	 */
	virtual~ Task();
	
	/**
	 * @brief executes the task
	 * @param executeNext used in objects of class Recipe to stop executing a Recipe if executeNext is false
	 * 
	 * virtual void which needs to be implemented by subclass
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) = 0;
	
	/**
	 * @brief get the kind of task
	 * @return type of the task as std::string 
	 * 
	 * virtual function which needs to be implemented by subclass
	 */
	virtual std::string getType() const = 0;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<DEVICES> getNecessaryDevices() = 0;
	
	/**
	 * @brief get the unique ID of the task
	 * @return the unique ID 
	 * 
	 * ID is obtained in the constructor
	 */
	const int getID() const;
	
	/**
	 * @brief get the comment of the task
	 * @return comment of the task
	 * @see setComment(std::string comment)
	 */
	std::string getComment() const;
	
	/**
	 * @brief get the name of the task
	 * @return name of the task
	 * @see setName(std::string name)
	 */
	std::string getName() const;
	
	/**
	 * @brief set the comment of the task
	 * @param comment which should be set
	 * @see getComment()
	 */
	void setComment(std::string comment);
	
	/**
	 * @brief set the name of the task
	 * @param name which should be set
	 * @see getName()
	 */
	virtual void setName(std::string name);
	
	/**
	 * @brief set the logfile
	 * @param l logfile
	 * 
	 * logfile is static -> one logfile for all tasks!
	 */
	static void setLogFile(Logbook::Logbook_ptr l);
	
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) = 0;
	
	static StatusLed::StatusLed_ptr status_leds;
	
	static Relais::Relais_ptr relais;
	
	
protected:
	/// the unique id obtained in the constructor
	int id;
	
	/// name of the task @see setName(std::string name)
	std::string name;
	
	/// comment of the task @set setComment(std::string comment)
	std::string comment;
	
	/**
	 * @brief add a new Log_Event
	 * @param l the new event
	 * should be called by subclasses to add a log event because Log* logfile is private
	 */
	void addLogEvent(Log_Event::Log_Event_ptr l);
	
private:
	/// counts the created Task objects (ID of the next Task Object)
	static int max_id;
	
	/// used to synchronize the access on max_id
	static std::mutex idMtx;
	
	/// logfile to log errors / information of tasks @see addLogEvent(Log_Event* l)
	static Logbook::Logbook_ptr logfile;
};
