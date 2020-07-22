#include "DataP.h"

#include <iostream>
#include <math.h>

DataP::DataP(double x, double y_real, double y_imag): x(x){
	setY_ReIm(y_real, y_imag);
}
DataP::~DataP(){
//	std::cout << "deleted DataP - " << toCsvLine() << std::endl;
}

DataP::DataP_ptr DataP::create(double x, double y_real, double y_imag){
	return std::make_shared<DataP>(x, y_real, y_imag);
}
	
double DataP::getX() const{
	return x;
}
double DataP::getY(COMPLEX_MODE m) const{
	switch(m){
		case COMPLEX_MODE::COMPLEX_PHASE_RAD:
			return y_phase;
		case COMPLEX_MODE::COMPLEX_PHASE_DEG:
			return radToDeg(y_phase);
		case COMPLEX_MODE::COMPLEX_REAL:
			return y_real;
		case COMPLEX_MODE::COMPLEX_IMAG:
			return y_imag;
		case COMPLEX_MODE::COMPLEX_ABS:
		default:
			return y_abs;
	}
}
void DataP::setY_AbsPhase(double abs, double phase, COMPLEX_MODE m){
	y_abs = abs;
	y_phase = (m == COMPLEX_PHASE_RAD) ? phase : degToRad(phase);
	y_real = std::cos(phase) * abs;
	y_imag = std::sin(phase) * abs;
}
void DataP::setY_ReIm(double real, double imag){
	y_real = real;
	y_imag = imag;
	y_abs = std::sqrt(real * real + imag * imag);
	y_phase = std::atan2(imag, real);
}
void DataP::setX(double x){
	DataP::x = x;
}
std::string DataP::toCsvLine(bool real, bool imag, bool abs, bool phase) const{
	std::string returnString = std::to_string(x);
	
	if (real){
		returnString += ";";
		returnString += std::to_string(y_real);
	}
	
	if (imag){
		returnString += ";";
		returnString += std::to_string(y_imag);
	}
	
	if (abs){
		returnString += ";";
		returnString += std::to_string(y_abs);
	}
	
	if (phase){
		returnString += ";";
		returnString += std::to_string(y_phase);
	}
	
	return returnString;
}
