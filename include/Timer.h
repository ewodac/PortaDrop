#pragma once

/**
 * @file Timer.h
 * 
 * @class Timer
 * @author Nils Bosbach
 * @date 30.08.2019
 * @brief class which can be used to run a specified method in regular interval in the context in which the object has been created (if the class has been created in the main context, the method will be run in the main context, too. This is necessary, if the method changed gui objects)
 */
 
#include <gtkmm.h>
#include <thread>
#include <functional>
 
class Timer{
public:
	
	/**
	 * @brief constructor, init the interval value
	 */
	Timer(int interval = 1000);
	
	/**
	 * @brief destructor
	 */
	virtual ~Timer();
	
	/**
	 * @brief start the regular execution of the specified mehtod
	 * @see stop
	 * @see connect
	 */
	void start();
	
	/**
	 * @brief stop the regular execution of the specified mehtod
	 * @see start
	 */
	void stop();
	
	/**
	 * @brief set the method which should be executed regularly
	 * @param slot method which should be executed regularly
	 */
	void connect(sigc::slot<void>&& slot);
	
	/**
	 * @brief set the method which should be executed regularly
	 * @param slot method which should be executed regularly
	 */
	void connect(const sigc::slot<void>& slot);
	
	/**
	 * @brief get the interval of the regular execution
	 * @return interval of the regular execution
	 */
	unsigned int getInterval() const;
	
	/**
	 * @brief set the interval of the regular execution
	 * @param interval interval of the regular execution
	 */
	void setInterval(unsigned int interval);
	
private:
	bool running;
	std::shared_ptr<Glib::Dispatcher> dispatcher;
	std::thread timer_thread;
	unsigned int interval;
	
	void start_thread(std::shared_ptr<Glib::Dispatcher> dispatcher, bool* running);
	
};
