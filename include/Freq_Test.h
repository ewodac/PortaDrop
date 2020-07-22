#pragma once
/**
 * @file Freq_Test.h
 * 
 * @class Freq_Test
 * @author Nils Bosbach
 * @date 15.08.2019
 * @brief used to test the frequency generator - sends a frequency to the attiny45 freq generator and requests a freq measurement from a gpib oszilloscope. The captured data is stored in a file on the disk
 */
#include "Task.h"
#include "GpibConnection.h"

#include <memory>

class Freq_Test: public Task{
	
public:
	typedef std::shared_ptr<Freq_Test> Freq_Test_ptr;
	Freq_Test();
	static Freq_Test_ptr create();
	
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	virtual std::string getType() const override;
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
protected:
	GpibConnection conn;
};
