#pragma once
/**
 * @file TransientGUIHandler.h
 * 
 * @class TransientGUIHandler
 * @author Nils Bosbach
 * @date 15.08.2019
 * @brief Handles the part of the GUI which is responsible for the display of the transient spectrum
 */
#include "TransSpect.h"
#include "ListView_Freq.h"
#include "PlotWindow.h"

#include <gtkmm.h>
#include <string>
#include <mutex>

class TransientGUIHandler{
public:
	
	TransientGUIHandler();
	
	//GUI elements
	ListView_Freq *listView_frequencies;
	PlotWindow *plotWindow_transSpectr;
	PlotWindow *plotWindow_Abs;
	PlotWindow *plotWindow_Phase;
	PlotWindow *plotWindow_Nyquist;
	Gtk::RadioButton *radioButton_transient_abs;
	Gtk::RadioButton *radioButton_transient_phase;
	Gtk::RadioButton *radioButton_transient_real;
	Gtk::RadioButton *radioButton_transient_imag;
	Gtk::Label *label_impedance_plot_title;
	Gtk::Label *label_impedance_plot_nyquist_title;
	Gtk::TextView *textView_dataInfo;
	
	void onTransImpSpecAdded (TransSpect::TransSpect_ptr p, TransSpect::Spectrum s, unsigned int position, double timediff, std::string path);
	void init();
	
private:
	typedef struct dispatcherData{
		TransSpect::TransSpect_ptr transSpectrum;
		TransSpect::Spectrum lastSpectrum;
		unsigned int position;
		double timediff;
		std::string path;
		std::mutex mtx;
	} dispatcherData_t;
	
	TransSpect::TransSpect_ptr spectrum;
	int selected_freq_no;
	DataP::COMPLEX_MODE selected_complexMode;
	TransSpect::X_VALUE x_val;
	Glib::Dispatcher dispatcher;
	dispatcherData_t dispatcher_data;
	
	void onTransImpSpecAdded_mainContext();
	void onListViewFreq_selection_changed();
	void onComplMode_selection_changed(DataP::COMPLEX_MODE c);
	void showTransSpectrum();
	void showSpectrum(TransSpect::Spectrum s);
	void setTextViewDataInfo();
	
	inline std::string get_transientPlotWindow_yAxisLabel_text() const;
	inline std::string get_transientPlotWindow_xAxisLabel_text() const;
};
