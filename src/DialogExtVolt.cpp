#include "DialogExtVolt.h"
#include "Addresses.h"
#include "FSHelper.h"

#include <string>
#include <iostream>

#define LOAD_WIDGET(WNAME, ONAME) refBuilder->get_widget(WNAME, ONAME); if(!ONAME) throw std::runtime_error(WNAME + std::string(" not found in glade file"));
#define CONNECT_SIGNAL_CLICKED(BNAME, FKT) BNAME->signal_clicked().connect(sigc::mem_fun(*this, &DialogExtVolt::FKT)); 


DialogExtVolt::DialogExtVolt(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder): Gtk::Dialog(cobject) {
	LOAD_WIDGET("button_extVolt_cancel", button_close);
	LOAD_WIDGET("label_extVolt_setpointVolt", label_setpointVoltage);
	LOAD_WIDGET("label_extVolt_actualVolt", label_actualVoltage);
	
	CONNECT_SIGNAL_CLICKED(button_close, onButtonClose_clicked);
	
	timer.connect(sigc::mem_fun(*this, &DialogExtVolt::timerFunction));
	timer.setInterval(200);
	
	atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	
	dispatcher.connect(sigc::mem_fun(*this, &DialogExtVolt::onDispatcherEmit_mainContext));
	running = false;
	
}

void DialogExtVolt::timerFunction(){
	std::cout << "timer function" << std::endl;
	if (atmega32->isConnected()){
		int ADCL = atmega32->readRegister(I2C_ATMEGA32_BUFFER_ADCL);
		int ADCH = atmega32->readRegister(I2C_ATMEGA32_BUFFER_ADCH);
		int ADC = ADCH * 256 + ADCL;
		double ext_voltage = (((double) VOLTAGE_ATMEGA32_R1) + ((double) VOLTAGE_ATMEGA32_R2)) / ((double) VOLTAGE_ATMEGA32_R2) * ((double) ADC) / 1023.0 * 5.0;
		double e_rel = (ext_voltage - (setpointVoltage)) / (setpointVoltage); // error
		e_rel = (e_rel > 0 ? e_rel : e_rel * -1.0); //abs
		
		label_actualVoltage->set_text(FSHelper::formatDouble(ext_voltage).append("V"));
		
		if (e_rel > 0.05){ // relative error is bigger than 5 %
			lastTimeWithRightVoltage = getCurrentTime();
			setLabelTextColor(label_actualVoltage, 238*255, 59*255, 59*255);
		}else{
			setLabelTextColor(label_actualVoltage, 0*255, 205*255, 0*255);
			if(getElapsedSeconds(lastTimeWithRightVoltage) > VOLTAGE_TASK_TIME_CORRECT_VOLT){
				timer.stop(); //stop timer
				closeDialog(GTK_RESPONSE_ACCEPT); //close dialog
			}
		}
	}else{
		label_actualVoltage->set_text("-");
	}
}

void DialogExtVolt::onButtonClose_clicked(){
	closeDialog(GTK_RESPONSE_CANCEL);
}

void DialogExtVolt::setSetpointVoltage(double setpoint){
	DialogExtVolt::setpointVoltage = setpoint;
	label_setpointVoltage->set_text(FSHelper::formatDouble(setpoint).append("V"));
}
void DialogExtVolt::startTimer(){
	lastTimeWithRightVoltage = getCurrentTime();
	timer.start();
}
void DialogExtVolt::closeDialog(int r){
	timer.stop();
	response(r);
	signal_response();
	hide();
}
int DialogExtVolt::show(){
	if (!running){
		startTimer();
		running = true;
		dispatcher.emit();
		while (running){
			
		}
		return resp;
	}
	return -1;
}


void DialogExtVolt::onDispatcherEmit_mainContext(){
	if (running_mutex.try_lock()){ // ensure that the dialog will only be showed once
		resp = run();
		running = false;
		running_mutex.unlock();
	}
}

DialogExtVolt::TimePoint DialogExtVolt::getCurrentTime(){
	return std::chrono::system_clock::now();
}

double DialogExtVolt::getElapsedSeconds(DialogExtVolt::TimePoint startTime){
	std::chrono::duration<double> elapsed_seconds = getCurrentTime() - startTime;
	
	return elapsed_seconds.count();
}

void DialogExtVolt::setLabelTextColor(Gtk::Label* label, unsigned int r, unsigned int g, unsigned int b){
	Pango::AttrList pngList;
	Pango::Attribute pngFgColor;
	Gdk::RGBA rgba;
	
//	pngList = label->get_attributes();
	pngFgColor = Pango::Attribute::create_attr_foreground(r, g, b);
	pngList.change(pngFgColor);
	
	label->set_attributes(pngList);
}