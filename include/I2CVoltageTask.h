#pragma once
/**
 * @file I2CVoltageTask.h
 * 
 * @class I2CVoltageTask
 * @author Nils Bosbach
 * @date 20.07.2019
 * @brief This class can be used to change the voltage of the attiiny45 boost converter controller via i2c. The value will be transmitted during the
 * execute function.
 */
#include "I2CTask.h"
#include "Uc_Connection.h"
#include "DialogExtVolt.h"

#include <string>
#include <pthread.h>
#include <memory>
#include <chrono>

class I2CVoltageTask: public I2CTask{

public:
	
	/// smart pointer to a I2CVoltageTask object
	typedef std::shared_ptr<I2CVoltageTask> I2CVoltageTask_ptr;
	
	/// time type
	typedef std::chrono::system_clock::time_point TimePoint;
	
	/// possible modes of the task -> set the duty cycle or the setpoint of the internal voltage controller or choose the external voltage source
	enum VOLTAGE_MODE {MODE_DUTY_CYCLE, MODE_CONTROLLER, MODE_EXTERN, MODE_ERROR};
	
	/**
	 * @brief constructor, opens the i2c connection to the attiny45 voltage controller
	 * @param voltage voltage setpoint [V]
	 * @param m set the mode of the task (voltage controller, constant duty cycle, external source)
	 */
	I2CVoltageTask(unsigned int voltage = 20, VOLTAGE_MODE m = VOLTAGE_MODE::MODE_CONTROLLER);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~I2CVoltageTask();
	
	/**
	 * @brief creates a new I2CVoltageTask object and returns a smart pointer of the object
	 * @param voltage voltage setpoint [V]
	 * @param m set the mode of the task (voltage controller, constant duty cycle, external source)
	 * @return smart pointer to the created object
	 */
	static I2CVoltageTask_ptr create(unsigned int voltage = 20, VOLTAGE_MODE m = VOLTAGE_MODE::MODE_CONTROLLER);
	
	
	/**
	 * @brief switches relais (if necessary) and transmits the new setpoint to the controller. If voltage_on is false, relais are switched to deactivate the boost converter
	 * @param data object where captured data can be stored - not used in this I2CTempTask
	 * @param executeNext used to stop a running experiment if value is false
	 * @param executeNext
	 */
	virtual void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override; // sends the request to uc
	
	/**
	 * @brief get the kind of task as string
	 * @return 'I2CVoltageTask'
	 */
	virtual std::string getType() const override;
	
	/**
	 * @brief saves the task as part of a xml file
	 * @param doc the xml document which should contain the task
	 * @param externElements if true, links to other files are allowed
	 * @return the part of the xml file
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
	/**
	 * @brief create a I2CVoltageTask from data defined in a xml file and return a smart pointer to the created object
	 * @param task_element part of the xml file which contains the params of the I2CFreqTask
	 * @return smart pointer to the created object
	 */
	static I2CVoltageTask_ptr loadVoltageTask(tinyxml2::XMLElement* task_element);
	
	/**
	 * @brief sets the internal voltage variable, the value will not be transmittet until execute is called
	 * @param v voltage [V], which should be transmittet during execute
	 */
	void setVoltage(unsigned int v);
	
	/**
	 * @brief returns the value of the internal voltage variable (which does not have to be the current value of the attiny45)
	 * @return current internal voltage value
	 */
	unsigned int getVoltage() const;
	
	/**
	 * @brief check whether a attiny45 voltage controller is connected via i2c
	 * @return true if a voltage controller is connected
	 */
	bool isConnected();
	
	/**
	 * @brief read the current duty cycle from the voltage controller
	 * @return the duty cycle (value between 0 and 1)
	 */
	double readCurrentDutyCycle();
	
	/**
	 * @brief read the current mode from the attiny45
	 * @return the current running mode of the attin45
	 */
	VOLTAGE_MODE readCurrentMode();
	
	/**
	 * @brief read the current voltage from the voltage controller
	 * @return the current voltage [V]
	 */
	double readVoltage();
	
	/**
	 * @brief read the current voltage of the external voltage source (atmega32)
	 * @return the current voltage [V] of the ext. voltage source
	 */
	double readExternalVoltage();
	
	/**
	 * @brief read the current frequency from the voltage controller
	 * @return the current frequency [V]
	 */
	int readFrequency();
	
	/**
	 * @brief read the setpoint voltage from the voltage controller
	 * @return the setpoint voltage [V]
	 */
	double readSetpointVoltage();
	
	/**
	 * @brief decide whether the execute function should wait until the setpoint voltage has been reached
	 * @param wait if true, the execute function will wait until the setpoint has been reached
	 */
	void setWaitForVoltage(bool wait);
	
	/**
	 * @brief set the parameter which should be changed (setpoint or dutycycle)
	 * @param mode the parameter which should be changed
	 */
	void setVoltageMode(VOLTAGE_MODE mode);
	
	/**
	 * @brief set the duty cylce which will be transmitted to the voltage controller during execute function, if the mode of the task is MODE_DUTY_CYCLE
	 * @param d duty cylce, needs to be in the range 0..1 -> if the value is out of range, the internal duty cycle varibale will not be changed!
	 * @see setVoltageMode
	 */
	void setDutyCycle(double d);
	
	/**
	 * @brief get the duty cycle, which will be transmitted to the attiny45 voltage controller during the execute function, if the mode of the task is MODE_DUTY_CYCLE
	 * @return the duty cycle 
	 */
	double getDutyCycle() const;
	
	/**
	 * @brief the the mode of the voltage task (set duty cycle, set setpoint or external source)
	 * @return the voltage mode which tells if setpoint, duty cylce will be changed in the execute function or if an external source is used
	 */
	VOLTAGE_MODE getVoltageMode() const;
	
	/**
	 * @brief check whether the execute function will wait until the setpoint voltage has been reached
	 * @return true if the execute function will wait for the setpoint to be reached
	 */
	bool getWaitForVoltage() const;
	
	std::string to_string() const;
	
	static DialogExtVolt *dialogExtVolt;
	
	static TimePoint getCurrentTime();
	static double getElapsedSeconds(TimePoint startTime);
	
private:
	unsigned int voltage;
	bool waitForVoltage;
	double dutyCycle;
	VOLTAGE_MODE mode;
	
	
	void setInternalVoltage(bool* executeNext);
	void setExternalVoltage(bool* executeNext);
};
