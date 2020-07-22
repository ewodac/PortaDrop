#pragma once
/**
 * @file TestTask
 * 
 * @class TestTask
 * @author Nils Bosbach
 * @date 04.09.2019
 * @brief Task which can be used to test a custom code in the execute function
 */
#include "Task.h"

#include <string>
#include <pthread.h>

class TestTask: public Task {
public:
	TestTask();
	TestTask(std::string name);
	~TestTask();
	void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	virtual std::string getType() const override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;

protected:
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;

};

