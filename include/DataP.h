#pragma once
/**
 * @file DataP.h
 * 
 * @class DataP
 * @author Nils Bosbach
 * @date 23.05.2019
 * @brief this class implements a data point containing a real x value and a complex y value.
 */
#include <string>
#include <memory>
#define degToRad(angleDeg) ((angleDeg) * M_PI / 180.0)
#define radToDeg(angleRad) ((angleRad) * 180.0 / M_PI)

class DataP{
public:
	///smart pointer to a DataP object
	typedef std::shared_ptr<DataP> DataP_ptr;
	
	///different components of a complex number - used when a part of the complex number is needed
	enum COMPLEX_MODE {COMPLEX_ABS, COMPLEX_PHASE_DEG, COMPLEX_PHASE_RAD, COMPLEX_REAL, COMPLEX_IMAG};
	
	/**
	 * @brief constructor, initializes the components
	 * @param x real x value
	 * @param y_real real part of the y value
	 * @param y_imag imaginary part of the y value
	 */
	DataP(double x = 0, double y_real = 0, double y_imag = 0);
	
	/**
	 * @brief destructor
	 */
	virtual ~DataP();
	
	/**
	 * @brief creates a new DataP object and returns a smart pointer to the object
	 * @param x real x value
	 * @param y_real real part of the y value
	 * @param y_imag imaginary part of the y value
	 */
	static DataP_ptr create(double x = 0, double y_real = 0, double y_imag = 0);
	
	/**
	 * @brief get the x value
	 * @return x value of the DataP
	 */
	double getX() const;
	
	/**
	 * @brief get a part of the y value
	 * @param m defines which part of the y value will be returned
	 * @return the selected part of the y value
	 */
	double getY(COMPLEX_MODE m = COMPLEX_MODE::COMPLEX_ABS) const;
	
	/**
	 * @brief set the complex y value by setting abs and phase
	 * @param abs absolute value of the y value
	 * @param phase phase of the y value
	 * @param m mode of the phase (degree or rad)
	 */
	void setY_AbsPhase(double abs, double phase, COMPLEX_MODE m = COMPLEX_PHASE_RAD);
	
	/**
	 * @brief the the complex y value by setting real and imaginary part
	 * @param real real part of the y value
	 * @param imag imaginary part of the y value
	 */
	void setY_ReIm(double real, double imag = 0);
	
	/**
	 * @brief set the x value of the DataP
	 * @param x x value
	 */
	void setX(double x);
	
	/**
	 * @brief create a semi-colon-seperated String containing the x value and selected components of the y coordinate
	 * @param real select the real part to appear in the list
	 * @param imag select the imaginary part to appear in the list
	 * @param abs select the absolute value to appear in the list
	 * @param phase select the phase (in rad) to appear in the list
	 */
	std::string toCsvLine(bool real = true, bool imag = true, bool abs = false, bool phase = false) const;
	
private:
	double x;
	double y_real;
	double y_imag;
	double y_phase; // phase in rad
	double y_abs;
	
};
