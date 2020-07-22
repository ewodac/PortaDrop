#include "FSHelper.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <algorithm>

bool FSHelper::endsWith(const std::string& str, const std::string& suffix){
	return (str.size() >= suffix.size()) && (0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix));
}
int FSHelper::createDirectory(std::string path){
	std::cout << "creating dir: '" << path << "'" << std::endl;
	return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
bool FSHelper::fileExists(const std::string& path){
	std::ifstream f(path.c_str());
	return f.good();
}
bool FSHelper::folderExists(const std::string& path){
	DIR *dirp = opendir(path.c_str());
	if (dirp != nullptr){
		closedir(dirp);
		return true;
	}else{
		return false;
	}
}
std::string FSHelper::composePath(std::string folder, std::string file){
	return (folder + (endsWith(folder, "/") ? "" :"/") + file);
}
std::string FSHelper::getNextAvailablePath(std::string path, std::string ending, bool showZero){
	
	std::string createdPath;
	int appendix = 0;
	
	do {
		createdPath = path;
		if (appendix != 0 || showZero){
			createdPath += "_" + std::to_string(appendix);
		}
		
		if (!ending.empty()){
			createdPath += "." + ending;
		}
		
		appendix++;
	}while (FSHelper::fileExists(createdPath));
	return createdPath;
}
std::string FSHelper::getCurrentTimestamp(){
	std::time_t currentTime = std::time(0);
	std::tm *ptm = std::localtime(&currentTime);
	char buffer[20];
	std::strftime(buffer, 20, "%F_%H:%M:%S", ptm);
	return buffer;
}
std::string FSHelper::formatDouble(double d){
	std::ostringstream streamObj;
	streamObj << d;
	std::string strObj = streamObj.str();
	return strObj;
}

void FSHelper::save_dataPToCsv(const std::vector<DataP::DataP_ptr> &data, std::string path, bool y_real, bool y_imag, bool y_abs, bool y_phase, std::string x_label, std::string y_label, std::string header){
	std::ofstream file;
	file.open(path);
	if (!header.empty()){
		file << header << std::endl;
	}
	file << x_label;
	
	if (y_real){
		file << ";" << y_label << "_real";
	}
	if (y_imag){
		file << ";" << y_label << "_imag";
	}
	if (y_abs){
		file << ";" << y_label << "_abs";
	}
	if (y_phase){
		file << ";" << y_label << "_phase";
	}
	file << std::endl;
	
	for (std::vector<DataP::DataP_ptr>::const_iterator cit = data.begin(); cit != data.end(); cit++){
		const DataP::DataP_ptr data_point = *cit;
		file << data_point->toCsvLine(y_real, y_imag, y_abs, y_phase) << std::endl;
	}
	
	file.close();
}


std::vector<std::string> FSHelper::getFolderContent(std::string folder, bool justFolder){
	std::vector<std::string> entries;
	
	
	DIR *dirp = opendir(folder.c_str());
	if (dirp != nullptr){
		struct dirent *dp;
		
		while((dp = readdir(dirp)) != NULL){
			if ((std::string(dp->d_name).compare(".") != 0) && (std::string(dp->d_name).compare("..") != 0)){
				if (!justFolder || (dp->d_type == DT_DIR)){
					entries.push_back(composePath(folder, dp->d_name));
				}
			}
		}
		closedir(dirp);
	}
	std::sort(entries.begin(), entries.end(), numeric_string_compare);
	return entries;
}

std::string FSHelper::toASCIIString(std::string s){
	std::string returnString = "";
	for (int i = 0; i < s.size(); i++){
		returnString += std::to_string((int) s.at(i));
		returnString += " ";
	}
	return returnString;
}

double FSHelper::sciToDouble(const std::string& str){
	std::stringstream ss(str);
	double d = 0;
	
	ss >> d;
	
	if (ss.fail()){
		throw std::runtime_error("unable to format " + str + " as a number");
	}
	return d;
}
std::string FSHelper::formatTime(unsigned int sec){
	const double minutes = sec / 60.0;
	const double hours = minutes / 60.0;
	const int hours_int = std::floor(hours);
	const int minutes_int = std::floor(minutes - hours_int * 60.0);
	const int seconds_int = sec - (hours_int * 3600 + minutes_int * 60);
	std::string hours_string = std::to_string(hours_int);
	std::string minutes_string = std::to_string(minutes_int);
	std::string seconds_string = std::to_string(seconds_int);
	std::string returnString = "";
	
	if (hours_int > 0){
		returnString += std::to_string(hours_int) + "h ";
	}
	
	if (hours_int > 0 || minutes_int > 0){
		returnString += std::to_string(minutes_int) + "m ";
	}
	
	returnString += std::to_string(seconds_int) + "s";
	
	return returnString;
}

bool FSHelper::is_not_digit(char c){
	return !std::isdigit(c);
}
bool FSHelper::numeric_string_compare(const std::string& s1, const std::string& s2){
	std::string::const_iterator it1 = s1.begin(), it2 = s2.begin();
	int i = 0;
	std::string n1 = "", n2 = "";
	
	for (i = 0; i < s1.length() && i < s2.length(); i++){
		
		if (s1[i] == s2[i]){ // chars match
			if (std::isdigit(s1[i])){
				n1 += s1[i];
				n2 += s2[i];
			}else{
				n1.clear();
				n2.clear();
			}
		}else{ //chars dismatch 
			
			// read the numbers to the end
			for (int j = 0; (j + i) < s1.length() && std::isdigit(s1[i+j]); j++){
				n1 += s1[i+j];
			}
			for (int j = 0; (j + i) < s2.length() && std::isdigit(s2[i+j]); j++){
				n2 += s2[i+j];
			}
			
			if (!n1.empty() && !n2.empty()){ // if numbers have been read
				return std::stoi(n1) < std::stoi(n2);
			}else{ // no numbers have been read
				return std::lexicographical_compare(it1, s1.end(), it2, s2.end());
			}
		}
	}
	return std::lexicographical_compare(it1, s1.end(), it2, s2.end());
}