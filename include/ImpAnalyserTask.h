#pragma once
/**
 * @file ImpAnalyserTask.h
 * 
 * @class ImpAnalyserTask
 * @author Nils Bosbach
 * @date 04.06.2019
 * @brief requests an impedance spectrum and stores it in the ExperimentData Object during execute method
 */
#include "Task.h"
#include "ImpAnalyser.h"

#include <memory>

class ImpAnalyserTask: public Task{
public: 
	typedef std::shared_ptr<ImpAnalyserTask> ImpAnalyserTask_ptr;
	
	/**
	 * @brief initializes the oject
	 * @param analyser the impedance analyser which should be used to meausre the spectrum
	 */
	ImpAnalyserTask(ImpAnalyser::ImpAnalyser_ptr analyser);
	
	/** 
	 * @brief virtual destructor
	 */
	virtual ~ImpAnalyserTask();
	
	/**
	 * @brief requests a spectrum from the impedance analyser and stores it in the ExperimentData object
	 * @param data object where the captured data is stored
	 * @param executeNext needs to be true to execute a task - used to stop the execution
	 */
	void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief used to get the type of the Task
	 * @return "ImpAnalyserTask - <return value of impAnalyser.getType()>"
	 */
	std::string getType() const override;
	
	/**
	 * @brief used to save the params in a xml document
	 * @param doc pointer to the document where the impedance analyser will be safed
	 * @param extern elements determins whether links to other files are allowed
	 * @return the created element containing the params
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief creates an ImpAnalyserTask from the data specified in a xml file
	 * @param task_elemmnt the part of a xml document which contain the information of the ImpAnalyserTask
	 */
	static ImpAnalyserTask_ptr loadImpAnalyserTask(tinyxml2::XMLElement* task_element);
	
	
private:
	ImpAnalyser::ImpAnalyser_ptr impAnalyser;
	
	std::string getParams() const;
};
