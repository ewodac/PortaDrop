#include "GpibConnection.h"

#include <stdexcept>
#include <gpib/ib.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

GpibConnection::GpibConnection(int slaveAddress): slaveAddress(slaveAddress){	
	try{
		openConnection();
	}catch (std::runtime_error e){
		std::cout << e.what() << std::endl;
	}
}
GpibConnection::~GpibConnection(){
	
}

void GpibConnection::send(std::string command){
	if (devDescr < 0){ // connection has not been opened yet
		openConnection();
	}
	
	if (devDescr >= 0){ // device has been opened
		if (ibwrt(devDescr, command.c_str(), std::strlen(command.c_str())) & ERR){
//			std::cout << "Schreibfehler: " << command;
			throw std::runtime_error("gpib writing error: " + command);
		}else{
//			std::cout << "command '" + command + "' send" << std::endl;
		}
	}else{ // device has not been opened
		
	}
}

bool GpibConnection::isConnected(){
	try{
		send("*IDN?");
	}catch(std::runtime_error &e){
		return false;
	}
	return true;
}

std::string GpibConnection::read(){
	char buffer[1024];
	if (devDescr < 0){ // connection has not been opened yet
		openConnection();
	}
	if (devDescr >= 0){ // device has been opened
		ibrd(devDescr, buffer, sizeof(buffer)-1);
//		std::cout << "message read: " << buffer << std::endl;
		return std::string(buffer);
	}else{ // device has not been opened
		return "";
	}
}

int GpibConnection::getSlaveAddress() const{
	return slaveAddress;
}

void GpibConnection::openConnection(){
	const int sad = 0;
	const int send_eoi = 1;
	const int eos_mode = 0;
	const int timeout = T1s;
	
	devDescr = ibdev(0, slaveAddress, sad, timeout, send_eoi, eos_mode);
	if (devDescr < 0){ //failed to open device
		throw std::runtime_error("error opening gpib connection to slave" + std::to_string(slaveAddress));
	}else{ // opened device
		ibsic(devDescr);
		usleep(1000*200);
		ibclr(devDescr);
		usleep(1000*200);
	}
}
bool GpibConnection::connectionOpened() const{
	return (devDescr < 0);
}
std::string GpibConnection::getName(){
	send("*IDN?");
	return read();
}