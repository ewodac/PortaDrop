#pragma once
/**
 * @file GpibConnection.h
 * 
 * @class GpibConnection
 * @author Nils Bosbach
 * @date 22.05.2019
 * @brief class which enables a gpib connection to communicate with measurement tools via gpib
 */
#include <string>

class GpibConnection{
public:
	
	/**
	 * @brief constructor, opens a new gpib connection
	 * @param slaveAddress address of the slave which should be addressed
	 */
	GpibConnection(int slaveAddress);
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~GpibConnection();
	
	/**
	 * @brief get the gpib address of the slave
	 * @return the address which has been set in the constructor
	 */
	int getSlaveAddress() const;
	
	/**
	 * @brief send a command to the slave via gpib
	 * @param command the command which should be send to the slave
	 */
	void send(std::string command);
	
	/**
	 * @brief read a response from the slave
	 * @return the response of the slave
	 */
	std::string read();
	
	/**
	 * @brief sends '*IDN?' and returns the response
	 * @return the name of the slave
	 */
	std::string getName();
	
	/**
	 * @brief check, if the connection could be connectionOpened
	 * @return true, if the connection has been connectionOpened
	 */
	inline bool connectionOpened() const;
	
	/**
	 * @brief check, if messages can be send to the slave
	 * @return true, if a message can be delivered to the slave
	 */
	bool isConnected();
	
private:
	int slaveAddress;
	int devDescr;
	
	void openConnection();
};
