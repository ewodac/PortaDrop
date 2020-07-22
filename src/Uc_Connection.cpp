#include "Uc_Connection.h"

#include <wiringPiI2C.h>
#include <iostream>
#include <unistd.h>

std::list<std::weak_ptr<Uc_Connection>> Uc_Connection::all_Connections;
std::mutex Uc_Connection::all_ConnectionsMtx;
std::mutex Uc_Connection::i2cMutex;

Uc_Connection::Uc_Connection_ptr Uc_Connection::create(int slaveAddress){
	Uc_Connection_ptr p;
	
	//search for an already opended connection to the slave
	all_ConnectionsMtx.lock();
	for(std::list<std::weak_ptr<Uc_Connection>>::const_iterator cit = all_Connections.cbegin(); cit != all_Connections.cend(); cit++){
		if (Uc_Connection_ptr tempPointer = cit->lock()){ // pointer not expired
			if (tempPointer->getSlaveAddress() == slaveAddress){
				p = tempPointer;
				break;
			}
		}
	}
	all_ConnectionsMtx.unlock();
	
	if (p == nullptr){ // no openend connection found
		p = Uc_Connection_ptr(new Uc_Connection(slaveAddress));
		all_Connections.push_back(p);
	}
	return p;
}


Uc_Connection::Uc_Connection(int deviceID): deviceID(deviceID){
	Uc_Connection::fd = wiringPiI2CSetup(deviceID); // 7 bit slave address
}
Uc_Connection::~Uc_Connection(){
	
	//remove pointer from global list
	all_ConnectionsMtx.lock();
	for(std::list<std::weak_ptr<Uc_Connection>>::const_iterator cit = all_Connections.cbegin(); cit != all_Connections.cend(); cit++){
		if (Uc_Connection_ptr tempPointer = cit->lock()){ // pointer not expired
			if (tempPointer->getSlaveAddress() == deviceID){ // found pointer
				all_Connections.erase(cit);
				break;
			}
		}else{
			all_Connections.erase(cit);
			break;
		}
	}
	all_ConnectionsMtx.unlock();
}

/*
* sends one byte to the slave
*/
int Uc_Connection::sendByte(int data) const{
	int retval = 0;
	
	i2cMutex.lock();
	retval = wiringPiI2CWrite(Uc_Connection::fd, data);
	//std::cout << "Uc_Connection::sendByte - sending "  << data << std::endl;
	i2cMutex.unlock();
	
	return retval;
}

int Uc_Connection::readRegister(int reg) const{
	int retval = 0;
	
	i2cMutex.lock();
	wiringPiI2CWrite(fd, reg);
	usleep(5);
	retval = wiringPiI2CRead(fd);
	i2cMutex.unlock();
	
//	std::cout << "r\tslave " << deviceID << "\t- reg " << reg << "\t- data " << retval << std::endl;
	return retval;
}

int Uc_Connection::writeRegister(int reg, int data) const{
	int retval = 0;
	
	i2cMutex.lock();
	retval = wiringPiI2CWriteReg8(fd, reg, data);
	i2cMutex.unlock();
	
//	std::cout << "w\tslave " << deviceID << "\t- reg " << reg << "\t- data " << data << std::endl;
	return retval;
}

/*
* receives one byte from the slave
*/
int Uc_Connection::receiveByte() const{
	int retval = 0;
	
	i2cMutex.lock();
	retval = wiringPiI2CRead(Uc_Connection::fd);
	i2cMutex.unlock();
	
	return retval;
}

int Uc_Connection::getSlaveAddress() const{
	return deviceID;
}

bool Uc_Connection::isConnected(){
	return (readRegister(0x00) == deviceID);
}
