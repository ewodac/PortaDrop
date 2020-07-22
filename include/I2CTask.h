#pragma once
/**
 * @file I2CTask.h
 * 
 * @class I2CTask
 * @author Nils Bosbach
 * @date 10.05.2019
 * @brief This abstract class implements one I2C task. Class must be derived to implement a specific task (e.g. I2CTempTask) and implent the virtual method execute (declared in mother class Task). The I2C tasks are handled by a Uc_Control object. Each I2C task has its own id.
 */
#include "Uc_Connection.h"
#include "Task.h"

class I2CTask: public Task{
	
public:
	
	/**
	 * @brief constructor - creates a Uc_Connection to the specified slave
	 * @param slaveAddress the i2c address of the slave
	 */
	I2CTask(int slaveAddress);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~I2CTask();
	
	/**
	 * @brief check if the execute function has been called
	 * @return true, if the execute function has been called at least once
	 */
	bool hasBeenExecuted() const;
	
	/**
	 * @brief get a pointer to the Uc_Connection object, which handles the i2c connection to the slave
	 * @return smart pointer to the Uc_Connection object
	 */
	Uc_Connection::Uc_Connection_ptr getUcConnection() const;
	
protected:
	Uc_Connection::Uc_Connection_ptr connection; // specifies the I2C connection to the slave
	bool executed;

private:
	
	
};
