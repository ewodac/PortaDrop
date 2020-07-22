#pragma once
/**
 * @file SpectrometerTask.h
 * 
 * @class SpectrometerTask
 * @author Nils Bosbach
 * @date 14.05.2019
 * @brief stores the properties which should be set to a Spectrometer. The execute method requests a 
 * spectrum.
 */
 
#include "Task.h"
#include "Spectrometer.h"
#include "DataP.h"

#include <string>
#include <vector>
#include <memory>
 
class SpectrometerTask: public Task{
public:
	
	typedef std::shared_ptr<SpectrometerTask> SpectrometerTask_ptr;
	
	/**
	 * @brief saves the properties which should be applied to the Spectrometer
	 * @param s the Spectrometer which should be used to capture the Spectrum
	 * @param triggerMode the Spectrometer::TRIGGER_MODE which should be used to capture the Spectrum
	 * @param integrationTimeMicros nuber of micro seconds the Spectrometer counts the intensity of the wavelength. To get the time it takes for the method to return, the time needs to be multiplied by scansToAverage
	 * @param scansToAverage number of Spectrums which are requested from the Spectromenter and turned into an average Spectrum to reduce white noise
	 */
	SpectrometerTask(Spectrometer *s, Spectrometer::TRIGGER_MODE triggerMode = Spectrometer::TRIGGER_MODE::TRIGGER_MODE_NORMAL, unsigned long integrationTimeMicros = 1000, unsigned int scansToAverage = 1);
	
	virtual ~SpectrometerTask();
	
	/**
	 * @brief creates a new SpectrometerTask from a tinyxml2::XMLElement by using an existing Spectrometer
	 * @param task_element the element which has been extracted from a xml file
	 * @param s the Spectrometer which should be used to capture the Spectrum
	 * @return the created SpectrometerTask
	 */
	static SpectrometerTask_ptr loadSpectrometerTask(tinyxml2::XMLElement* task_element, Spectrometer *s);
	
	/**
	 * @brief triggers the Spectrometer to capture a Spectrum and stores a pointer to the caputed Spectrum in the ExperimentData parameter variable and the class varible
	 * @param data pointer to an ExperimentData Object which is used to store the captured data from the execute functions during the execution of an experiment
	 * @param executeNext if false the execution of the experiment should be stopped -> used to stop a running experiment without directly killing the thread
	 * @param mtx mutex to synchronize the access on executeNext
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief creates a tinyxml2::XMLElement which contains the properties of the object to save in a xml file
	 * @param doc the document in which the returned element should be saved
	 * @param externElements if false, everything has to be stored in the same document, no links allowed
	 * @return the created XMLElement
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get the type of the Task
	 * @return "SpectrometerTask"
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
private:
	
	Spectrometer::Spectrometer_ptr spectrometer;
	Spectrometer::TRIGGER_MODE triggerMode;
	unsigned long integrationTimeMicros;
	unsigned int scansToAverage;
	std::string getSpectrometerParams() const;
};
