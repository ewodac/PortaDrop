#pragma once
/** 
 * @file Uc_Connection.h
 *
 * @class Uc_Connection
 * @author Nils Bosbach
 * @date 10.04.2019
 * @brief implements the I2C connection between the raspberry pi (master) and the atmega32 (slave)
 * An I2C connection is opened using the wiringPiI2C libary
 */
 #include <memory>
 #include <mutex>
 #include <list>
 #include <vector>
 
class Uc_Connection{

public:
	typedef std::shared_ptr<Uc_Connection> Uc_Connection_ptr;
	
	/**
	 * @brief creates / returns an exisiting connection to the slave addressed by the defined address
	 * @param slaveAddress the i2c address of the slave
	 * 
	 * There is at max one object for a connection to one slave. When this function is called the first time for a slave, a new Uc_Connection
	 * object is created and a pointer to this object is stored in a static private list of the class. When this method is called a second
	 * time for one slave Address (so there is already an exisiting object for that connection), a second pointer of the existing object is
	 * returned. Because the used pointers are smart pointers, the Uc_Connection object will be deleted if the last exisiting pointer is deleted.
	 * The pointer in the static list of the class will be removed in that case as well. 
	 * To guarantee that there is at max one object for one slave, this method needs to be used to create an Uc_Connection object and the
	 * constructor is not accessible from outside the class.
	 */
	static Uc_Connection_ptr create(int slaveAddress);
	
	/**
	 * @brief removes the pointer in the static class list
	 */
	virtual ~Uc_Connection();
	
	/**
	 * @brief sends one byte to the slave
	 * @param data the byte which should be send (values from 0 to 255 possible)
	 * @return return value of wiringPiI2CWrite
	 */
	int sendByte(int data) const;
	
	/**
	 * @brief requests one byte from the slave
	 * @return the received byte (range 0 to 255)
	 */
	int receiveByte() const;
	
	/**
	 * @brief read one register of the i2c slave
	 * @param reg the number of the register which should be read
	 * @return the value of the register
	 */
	int readRegister(int reg) const;
	
	/**
	 * @brief write one byte to a register of the i2c slave
	 * @param reg the number of the register where the data should be writen to
	 * @param data the byte which should be writte to the register
	 * @return the returnvalue of wiringPiI2CWrite
	 */
	int writeRegister(int reg, int data) const;
	
	
	/**
	 * @brief 
	 * @return the 7 bit address of the I2C Slave 
	 */
	int getSlaveAddress() const;
	
	/**
	 * @brief checks if the uc has its own slave address in register 0x00
	 * @return true if the slave is connected
	 */
	bool isConnected();
	
protected:
	static std::mutex i2cMutex;
	
private:
	int fd = 0; // connection hadler (from wiringPiI2C)
	int deviceID;
	static std::list<std::weak_ptr<Uc_Connection>> all_Connections; // static class list that handles 
	static std::mutex all_ConnectionsMtx;
	
	/**
	 * @brief opens the I2C connection to the slave specified in the param
	 * @param deviceID 7bit Slave address
	 *
	 * constructor not accessible from outside the class -> use create function to create a new object
	 */
	Uc_Connection(int deviceID);
};
