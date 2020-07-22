#pragma once
/**
 * @file DialogExtVolt.h
 * 
 * @class DialogExtVolt
 * @author Nils Bosbach
 * @date 23.09.2019
 * @brief GUI dialog which shows the current external voltage and requests the user to adjust the voltage to the setpoint. The dialog closes automatically if the relative error between the actual and setpoint voltage is smaller than 5% for 5s.
 */

#include "Timer.h"
#include "Uc_Connection.h"

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <glibmm/dispatcher.h>

#include <mutex>
#include <chrono>

class DialogExtVolt: public Gtk::Dialog{
public:
	
	/**
	 * @brief constructor
	 * @param cobject 
	 * @param builder builder reference
	 */
	DialogExtVolt(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	
	/**
	 * @brief set the setpoint voltage. If the relative error between the external voltage and the setpoint is smaller than 5% for 5s, the dialog closes automatically
	 * @param setpoint setpoint voltage [V]
	 */
	void setSetpointVoltage(double setpoint);
	
	/**
	 * @brief use this method to display the dialog. The method starts the timer to update the external voltage, displays the dialog and blocks until the dialog has been closeDialog
	 * @return -1, if the method is called when the dialog is running at that moment; the result of the dialog otherwise
	 */
	int show();
	
private:
	typedef std::chrono::system_clock::time_point TimePoint;
	
	Timer timer;
	Uc_Connection::Uc_Connection_ptr atmega32;
	Gtk::Button *button_close;
	Gtk::Label *label_setpointVoltage;
	Gtk::Label *label_actualVoltage;
	double setpointVoltage;
	Glib::Dispatcher dispatcher;
	int resp;
	bool running;
	std::mutex running_mutex;
	TimePoint lastTimeWithRightVoltage;
	
	/**
	 * @brief called regulary when dialog is displayed; updates the external voltage and closes the dialog if the voltage has been in range for 5s
	 */
	void timerFunction();
	
	/**
	 * @brief called when the close button of the dialog has been onButtonClose_clicked. Closes the dialog and returns GTK_RESPONSE_CANCEL
	 */
	void onButtonClose_clicked();
	
	/**
	 * @brief close the dialog and send a response
	 * @param response the respone to send
	 */
	void closeDialog(int response);
	
	/**
	 * @brief needs to be executed in the main context -> called by dispatcher. Used to run the dialog
	 */
	void onDispatcherEmit_mainContext();
	
	/**
	 * @brief start the timer to update the external voltage
	 */
	void startTimer();
	
	/**
	 * @brief get the current time as TimePoint
	 * @return current time
	 */
	static TimePoint getCurrentTime();
	
	/**
	 * @brief get the elapsed seconds between a specified start time and now
	 * @param startTime the begin of the measurement
	 * @return elapsed seconds since the specified start time
	 */
	static double getElapsedSeconds(TimePoint startTime);
	
	/**
	 * @brief set the textcolor of a Gtk::Label as RGB value
	 * @param label the label which text color should be changed
	 * @param r set red value (0, ..., 65535)
	 * @param g set green value (0, ..., 65535)
	 * @param b set blue value (0, ..., 65535)
	 */
	static void setLabelTextColor(Gtk::Label* label, unsigned int r, unsigned int g, unsigned int b);
};
