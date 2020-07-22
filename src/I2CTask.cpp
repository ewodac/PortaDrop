#include "I2CTask.h"

#include <iostream>

I2CTask::I2CTask(int slaveAddress): Task(), connection(Uc_Connection::create(slaveAddress)){
	executed = false;
}
I2CTask::~I2CTask(){
	
}
bool I2CTask::hasBeenExecuted() const{
	return executed;
}
Uc_Connection::Uc_Connection_ptr I2CTask::getUcConnection() const {
	return connection;
}
