#pragma once
/**
 * @file DelayTask.h
 * 
 * @class DelayTask
 * @author Nils Bosbach
 * @date 19.06.2019
 * @brief Implements a taks which blocks for a certain time during execute function
 */
#include "Task.h"

#include <string>
#include <pthread.h>
#include <memory>

class DelayTask: public Task {
public:
	
	/// smart pointer to a DelayTask object
	typedef std::shared_ptr<DelayTask> DelayTask_ptr;
	
	/**
	 * @brief constructor, sets the internal delay_time_s variable
	 * @param delayTimeS delay time in seconds
	 * @param delayTimeMs additionoal delay time in milli seconds
	 */
	DelayTask(unsigned int delayTimeS, unsigned int delayTimeMs = 0);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~DelayTask();
	
	/**
	 * @brief create a DelayTask object
	 * @param delayTimeS delay time in seconds
	 * @param delayTimeMs delay time in milli seconds
	 * @return smart pointer to the created object
	 */
	static DelayTask_ptr create(unsigned int delayTimeS, unsigned int delayTimeMs = 0);
	
	/**
	 * @brief does nothing for the defined time
	 * @param data object to store captured data - not used in this task
	 * @param executeNext deterimes, if the task should be carried out or stopped
	 */
	void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get the type of task as string
	 * @return 'DelayTask'
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief create a DelayTask from a part of a xml file
	 * @param task_element part of a xml file
	 */
	static DelayTask_ptr loadDelayTask(tinyxml2::XMLElement* task_element);
	

protected:
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	unsigned int delay_time_s;
	unsigned int delay_time_ms;
	
	std::string formatTime() const;
};

