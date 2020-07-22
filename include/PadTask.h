#pragma once
/**
 * @file PadTask.h
 * 
 * @class PadTask
 * @author Nils Bosbach
 * @date 23.04.2019
 * @brief implements the task to set EWOD pads high or low for a certain time
 */
#include "Task.h"

#include <string>
#include <vector>
#include <mutex>
#include <memory>


class PadTask: public Task{
public:
	
	/// smart pointer
	typedef std::shared_ptr<PadTask> PadTask_ptr;
	
	/**
	 * @brief create a pad task
	 * @param pads list of pads which should be activated for a ceratian time
	 * @param duration_ms defines how long the pads should be activated
	 */
	PadTask(std::vector<int> pads, int duration_ms);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~PadTask();
	
	/**
	 * @brief create a PadTask object
	 * @param pads list of pads which should be activated for a ceratian time
	 * @param duration_ms defines how long the pads should be activated
	 * @return pointer to the created object
	 */
	static PadTask_ptr create(std::vector<int> pads, int duration_ms);
	
	
	/**
	 * @brief powers all specified pad for the set time
	 * @param executeNext
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get the type of the task
	 * @return 'PadTask'
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief create a PadTask from a String
	 * @param s the string which should be used to create the PadTask
	 * @return pointer of the created PadTask
	 * 
	 * format of the string: <br>
	 * <time_ms>;<pad_1>;<pad_2>;...;<pad_n> <br>
	 * -> the task will power the pads 1 .. n for time_ms milli seconds
	 */
	static PadTask_ptr getPadTaskFromString(std::string s);
	
	/**
	 * @brief create a PadTask from data defined in a xml file and return a smart pointer to the created object
	 * @param task_element part of the xml file which contains the params of the PadTask
	 * @return smart pointer to the created object
	 */
	static PadTask_ptr loadPadTask(tinyxml2::XMLElement* task_element);
	
	/**
	 * @brief saves the task as part of a xml file
	 * @param doc the xml document which should contain the task
	 * @param externElements if true, links to other files are allowed
	 * @return the part of the xml file
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
private:
	int duration_ms;
	std::vector<int> pads;
	static std::mutex executeMtx;
	static bool executing;
	
	/**
	 * @brief activates an ewod pad
	 * @param pad the pad which should be activated (range 1..116)
	 */
	void setPadHigh(unsigned int pad);
	
	/**
	 * @brief set all Pads to low level
	 */
	void setPadsLow();
};
