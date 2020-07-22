#pragma once
/**
 * @file Log_Event.h
 * 
 * @class Log_Event
 * @author Nils Bosbach
 * @date 15.04.2019
 * @brief This class represents one event (error, info, default)
 * @see Logbook
 */
#include <string>
#include <ctime>
#include <memory>


class Log_Event{
public:
	/// types of the event
	enum TYPE {LOG_INFO, LOG_ERROR, LOG_DEFAULT};
	
	/// smart pointer
	typedef std::shared_ptr<Log_Event> Log_Event_ptr;
	
	/**
	 * @brief
	 * @param name name of the event
	 * @param description description of the evenet
	 * @param t type of the event
	 */
	Log_Event(std::string name = "", std::string description = "", TYPE t = TYPE::LOG_DEFAULT);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~Log_Event();
	
	/**
	 * @brief create a new Log_Event object and return a smart pointer to the object
	 * @param name name of the event
	 * @param description description of the evenet
	 * @param t type of the event
	 * @return smart pointer to the created object
	 */
	static Log_Event_ptr create(std::string name = "", std::string description = "", TYPE t = TYPE::LOG_DEFAULT);
	
	/**
	 * @brief get the time of the creation of the object
	 * @return time of the creation of the object
	 */
	std::time_t getTime() const;
	
	/**
	 * @brief get the time of the creation of the object as string
	 * @return time of the creation of the object as string
	 */
	std::string getTime_toString() const;
	
	/**
	 * @brief get the name of the event
	 * @return name of the event
	 */
	std::string getName() const;
	
	/**
	 * @brief get the description of the event
	 * @return description of the event
	 */
	std::string getDescription() const;
	
	/**
	 * @brief get the type of the event
	 * @return type of the event
	 */
	TYPE getType() const;
	
	/**
	 * @brief get the type of the event as string
	 * @return type of the event as string
	 */
	std::string getType_toString() const;
	
	/**
	 * @brief get type, time, name and description formatted as string
	 * @return formatted string containing type, time, name and description
	 */
	std::string toString() const;
	
private:
	std::string name;
	std::string description;
	std::time_t time;
	TYPE log_type;
	
	static std::string fillStringToLength(std::string s, unsigned int length);
};

std::ostream& operator<<(std::ostream& stream, const Log_Event& event);
