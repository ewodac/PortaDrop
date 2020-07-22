#pragma once
/**
 * @file FSHelper.h
 * 
 * @class FSHelper
 * @author Nils Bosbach
 * @date 16.05.2019
 * @brief class containing static helping methods to do simple string and file system operations
 */
#include "DataP.h"

#include <string>
#include <vector>

class FSHelper{
public:
	/**
	 * @brief check if a string ends whith anther string
	 * @param str the string which might have the suffix
	 * @param suffix the suffix which might be at the end of the string
	 * @return true if the string has the suffix
	 */
	static bool endsWith(const std::string& str, const std::string& suffix);
	
	/**
	 * @brief creates a folder in the filesystem
	 * @param path the path to the folder which should be created
	 * 
	 * important: the mother folder needs to exist, the function does not create folder recusively
	 */
	static int createDirectory(std::string path);
	
	/**
	 * @brief check if a file exists in the filesystem
	 * @prarm path path to the file which should be tested to exist
	 * @return true, if the file exists
	 */
	static bool fileExists(const std::string& path);
	
	/**
	 * @brief check if a folder exists in the filesystem
	 * @prarm path path to the folder which should be tested to exist
	 * @return true, if the folder exists
	 */
	static bool folderExists(const std::string& path);
	
	/**
	 * @brief compose a path of a file / folder from the path of the mother folder and the file / folder name
	 * @return addes the two strings and adds a "/" in the middle if needed
	 */
	static std::string composePath(std::string folder, std::string file);
	
	/**
	 * @brief add a number to the end of the filename until the path does not belong to an existing file
	 * @param path path of the file whithout ending
	 * @param ending the file ending of the file
	 * @return path of a not existing file
	 *
	 * example:
	 * folder /foofolder/ contains files foo.end foo1.end
	 * param: path = /foofolder/foo   -   ending = .end
	 * reuturn: /foofolder/foo2.end
	 */
	static std::string getNextAvailablePath(std::string path, std::string ending = "", bool showZero = false);
	
	/**
	 * @brief get the current date and time formatted as string
	 * @return "yyyy-mm-dd_hh:mm:ss"
	 */
	static std::string getCurrentTimestamp();
	
	/**
	 * @brief convert a double to string and cut 0s at the end
	 * @param d the double to convert
	 */
	static std::string formatDouble(double d);
	
	/**
	 * @brief saves a DataP vector to a csv file
	 * @param data the data which should be saved
	 * @param path where the csv file should be stored
	 * @param header header of the xml file
	 * @param x_label legend of the x axis
	 * @param y_label legend of the y axis
	 */
	static void save_dataPToCsv(const std::vector<DataP::DataP_ptr> &data, std::string path, bool y_real = true, bool y_imag = true, bool y_abs = false, bool y_phase = false, std::string x_label = "x_coordinate", std::string y_label = "y_coordinate", std::string header = "");
	
	/**
	 * @brief return the content of a directory as vector of strings. The vector is sorted using numeric_string_compare function
	 * @param dir_path the path of the directory which contents should be read
	 * @param justFolder if true, only directories will be added to the return list
	 * @return vector containing the filenames / foldernames of the folder content
	 */
	static std::vector<std::string> getFolderContent(std::string dir_path, bool justFolder = false);
	
	/**
	 * @brief convert a string into a stream of numbers which represent the ASCII Codes of the characters
	 * @param s string which should be converted
	 * @return string which contians the ASCII codes of the characters divided by blanks
	 */
	static std::string toASCIIString(std::string s);
	
	/**
	 * @brief convert a string to double
	 * @param str string which contains the double a string
	 * @return the converted double
	 */
	static double sciToDouble(const std::string& str);
	
	/**
	 * @brief format a duriation in [s] to a string '<hours>h <minutes>m <seconds>s'
	 * @param sec time in seconds
	 * @return formatted time as string
	 */
	static std::string formatTime(unsigned int sec);
	
	/**
	 * @brief check whether a character is a not a is_not_digit
	 * @param c character which should be checked
	 * @return true, if c is not a digit, false otherwise
	 */
	static bool is_not_digit(char c);
	
	/**
	 * @brief compares two strings. If two string differente by a number, they are compared by that number (e.g. "example123" > "example45" because 123 > 45)
	 * @param s1 first string which should be compared
	 * @param s2 second string which should be compared
	 * @return true, if s1 > s2
	 */
	static bool numeric_string_compare(const std::string& s1, const std::string& s2);
};
