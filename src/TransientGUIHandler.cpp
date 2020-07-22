#include "TransientGUIHandler.h";
#include "FSHelper.h"

#include <iostream>


TransientGUIHandler::TransientGUIHandler(){
	selected_freq_no = -1;
	x_val = TransSpect::X_VALUE::X_TIME_DIFF;
}
void TransientGUIHandler::init(){
	dispatcher.connect(sigc::mem_fun(*this, &TransientGUIHandler::onTransImpSpecAdded_mainContext));
	listView_frequencies->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &TransientGUIHandler::onListViewFreq_selection_changed));
	radioButton_transient_abs->signal_clicked().connect(sigc::bind<DataP::COMPLEX_MODE>(sigc::mem_fun(*this, &TransientGUIHandler::onComplMode_selection_changed), DataP::COMPLEX_MODE::COMPLEX_ABS));
	radioButton_transient_imag->signal_clicked().connect(sigc::bind<DataP::COMPLEX_MODE>(sigc::mem_fun(*this, &TransientGUIHandler::onComplMode_selection_changed), DataP::COMPLEX_MODE::COMPLEX_IMAG));
	radioButton_transient_phase->signal_clicked().connect(sigc::bind<DataP::COMPLEX_MODE>(sigc::mem_fun(*this, &TransientGUIHandler::onComplMode_selection_changed), DataP::COMPLEX_MODE::COMPLEX_PHASE_DEG));
	radioButton_transient_real->signal_clicked().connect(sigc::bind<DataP::COMPLEX_MODE>(sigc::mem_fun(*this, &TransientGUIHandler::onComplMode_selection_changed), DataP::COMPLEX_MODE::COMPLEX_REAL));
	
	onListViewFreq_selection_changed();
	
	if (radioButton_transient_abs->get_active()){
		selected_complexMode = DataP::COMPLEX_MODE::COMPLEX_ABS;
		
	}else if (radioButton_transient_imag->get_active()){
		selected_complexMode = DataP::COMPLEX_MODE::COMPLEX_IMAG;
		
	}else if (radioButton_transient_phase->get_active()){
		selected_complexMode = DataP::COMPLEX_MODE::COMPLEX_PHASE_DEG;
		
	}else if (radioButton_transient_real->get_active()){
		selected_complexMode = DataP::COMPLEX_MODE::COMPLEX_REAL;
		
	}
}

void TransientGUIHandler::onTransImpSpecAdded (TransSpect::TransSpect_ptr p, TransSpect::Spectrum s, unsigned int position, double timediff, std::string path){
	dispatcher_data.mtx.lock();
	
	dispatcher_data.path = path;
	dispatcher_data.lastSpectrum = s;
	dispatcher_data.position = position;
	dispatcher_data.timediff = timediff;
	dispatcher_data.transSpectrum = p;
	
	dispatcher_data.mtx.unlock();
	
	dispatcher.emit();
}
void TransientGUIHandler::onTransImpSpecAdded_mainContext(){
	if (dispatcher_data.mtx.try_lock()){
		if (spectrum != dispatcher_data.transSpectrum){ // first transient spectrum
			spectrum = dispatcher_data.transSpectrum;
			std::vector<double> frequencies = dispatcher_data.transSpectrum->getFrequencies();
			listView_frequencies->setFrequencies(frequencies);
			showTransSpectrum();
		}else{
			if (selected_freq_no >= 0 && selected_freq_no < dispatcher_data.lastSpectrum.size()){
				DataP::DataP_ptr p = dispatcher_data.lastSpectrum.at(selected_freq_no);
				
				double x;
				double y_real = p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
				double y_imag = p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
				switch(x_val){
					case TransSpect::X_VALUE::X_POINT:
						x = dispatcher_data.position;
						break;
					case TransSpect::X_VALUE::X_TIME:
					case TransSpect::X_VALUE::X_TIME_DIFF:
						x = dispatcher_data.timediff;
						break;
				}
				plotWindow_transSpectr->addData(std::make_shared<DataP>(x, y_real, y_imag));
				setTextViewDataInfo();
			}
		}
		showSpectrum(dispatcher_data.lastSpectrum);
		dispatcher_data.mtx.unlock();
	}
}

void TransientGUIHandler::onListViewFreq_selection_changed(){
	selected_freq_no = listView_frequencies->getSelectedFreq_rowNumber();
	
	if (selected_freq_no != -1){
		showTransSpectrum();
	}
}

void TransientGUIHandler::showTransSpectrum(){
	if (selected_freq_no != -1){
		plotWindow_transSpectr->autoMax_x = true;
		plotWindow_transSpectr->autoScl_x = true;
		plotWindow_transSpectr->autoScl_y = true;
		plotWindow_transSpectr->autoMax_y = true;
		plotWindow_transSpectr->autoPadding_x = true;
		plotWindow_transSpectr->autoPadding_y = true;
		plotWindow_transSpectr->showGrid = true;
		plotWindow_transSpectr->showZeroAxis = true;
		plotWindow_transSpectr->scale_x = PlotWindow::SCALE::LINEAR;
		plotWindow_transSpectr->scale_y = PlotWindow::SCALE::LINEAR;
		plotWindow_transSpectr->setComplexMode(selected_complexMode);
		plotWindow_transSpectr->setData(spectrum->getTransSpect(selected_freq_no, x_val));
		plotWindow_transSpectr->x_axis_label = get_transientPlotWindow_xAxisLabel_text();
		plotWindow_transSpectr->y_axis_label = get_transientPlotWindow_yAxisLabel_text();
		
		setTextViewDataInfo();
	}
}

void TransientGUIHandler::showSpectrum(TransSpect::Spectrum s){
	std::vector<DataP::DataP_ptr> nyquist_data;
	
	for (std::vector<DataP::DataP_ptr>::const_iterator cit = s.cbegin(); cit != s.cend(); cit++){
		DataP::DataP_ptr p = *cit;
		DataP::DataP_ptr dataPoint = std::make_shared<DataP>(p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL), (-1.0) * p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG));
		nyquist_data.push_back(dataPoint);
	}
	
	plotWindow_Abs->autoMax_y = true;
	plotWindow_Abs->autoMax_x = true;
	plotWindow_Abs->autoScl_x = true;
	plotWindow_Abs->autoScl_y = true;
	plotWindow_Abs->autoPadding_x = true;
	plotWindow_Abs->autoPadding_y = true;
	plotWindow_Abs->showGrid = true;
	plotWindow_Abs->scale_x = PlotWindow::SCALE::LOG;
	plotWindow_Abs->scale_y = PlotWindow::SCALE::LOG;
	plotWindow_Abs->setComplexMode(DataP::COMPLEX_ABS);
	plotWindow_Abs->x_axis_label = "frequency [Hz]";
	plotWindow_Abs->y_axis_label = "abs [OHM]";
	
	
	plotWindow_Phase->autoMax_x = true;
	plotWindow_Phase->autoScl_x = true;
	plotWindow_Phase->autoScl_y = true;
	plotWindow_Phase->autoMax_y = false;
	plotWindow_Phase->y_min = -180;
	plotWindow_Phase->y_max = 180;
	plotWindow_Phase->autoPadding_x = true;
	plotWindow_Phase->autoPadding_y = true;
	plotWindow_Phase->showGrid = true;
	plotWindow_Phase->showZeroAxis = true;
	plotWindow_Phase->scale_x = PlotWindow::SCALE::LOG;
	plotWindow_Phase->scale_y = PlotWindow::SCALE::LINEAR;
	plotWindow_Phase->setComplexMode(DataP::COMPLEX_PHASE_DEG);
	plotWindow_Phase->x_axis_label = "frequency [Hz]";
	plotWindow_Phase->y_axis_label = "phase [DEG]";
	
	plotWindow_Nyquist->autoMax_x = true;
	plotWindow_Nyquist->autoScl_x = true;
	plotWindow_Nyquist->autoScl_y = true;
	plotWindow_Nyquist->autoMax_y = true;
	plotWindow_Nyquist->autoPadding_x = true;
	plotWindow_Nyquist->autoPadding_y = true;
	plotWindow_Nyquist->showGrid = true;
	plotWindow_Nyquist->showZeroAxis = true;
	plotWindow_Nyquist->scale_x = PlotWindow::SCALE::LINEAR;
	plotWindow_Nyquist->scale_y = PlotWindow::SCALE::LINEAR;
	plotWindow_Nyquist->setComplexMode(DataP::COMPLEX_REAL);
	plotWindow_Nyquist->x_axis_label = "real part [OHM]";
	plotWindow_Nyquist->y_axis_label = "neg. imag. part [OHM]";
	
	plotWindow_Abs->setData(s);
	plotWindow_Phase->setData(s);
	plotWindow_Nyquist->setData(nyquist_data);
	label_impedance_plot_title->set_text("transient Spectrum");
	label_impedance_plot_nyquist_title->set_text("transient Spectrum");
}

void TransientGUIHandler::onComplMode_selection_changed(DataP::COMPLEX_MODE c){
	if (spectrum != 0 && selected_complexMode != c){
		selected_complexMode = c;
		plotWindow_transSpectr->setComplexMode(selected_complexMode);
		plotWindow_transSpectr->y_axis_label = get_transientPlotWindow_yAxisLabel_text();
		setTextViewDataInfo();
	}
}

void TransientGUIHandler::setTextViewDataInfo(){
	std::string text = "";
//	double freq = 
	double y_min = plotWindow_transSpectr->getYminData();
	double y_max = plotWindow_transSpectr->getYmaxData();
	double y_delta = y_max - y_min;
	std::string unit;
	
	switch(selected_complexMode){
		case DataP::COMPLEX_MODE::COMPLEX_ABS:
		case DataP::COMPLEX_MODE::COMPLEX_IMAG:
		case DataP::COMPLEX_MODE::COMPLEX_REAL:
			unit = "OHM";
			break;
		case DataP::COMPLEX_MODE::COMPLEX_PHASE_DEG:
			unit = "DEG";
			break;
		case DataP::COMPLEX_MODE::COMPLEX_PHASE_RAD:
			unit = "RAD";
			break;
	}
	
	text += "Minimum value:\n" + FSHelper::formatDouble(y_min) + " " + unit + "\n\n";
	text += "Maximum value:\n" + FSHelper::formatDouble(y_max) + " " + unit + "\n\n";
	text += "Delta value:\n"   + FSHelper::formatDouble(y_delta) + " " + unit + "\n\n";
	text += "Average value:\n" + FSHelper::formatDouble(plotWindow_transSpectr->getYaverage()) + " " + unit +  "\n\n";
	text += "Progress:\n"      + FSHelper::formatDouble(spectrum->progress * 100) + "%";
	
//	if (textView_dataInfoMtx.try_lock()){
		textView_dataInfo->get_buffer()->set_text(text);
//		textView_dataInfoMtx.unlock();
//	}
}

std::string TransientGUIHandler::get_transientPlotWindow_yAxisLabel_text() const{
	switch(selected_complexMode){
		case DataP::COMPLEX_MODE::COMPLEX_ABS:
			return "abs [OHM]";
		case DataP::COMPLEX_MODE::COMPLEX_IMAG:
			return "imaginary part [OHM]";
		case DataP::COMPLEX_MODE::COMPLEX_REAL:
			return "real part [OHM]";
		case DataP::COMPLEX_MODE::COMPLEX_PHASE_DEG:
			return "phase [DEG]";
		case DataP::COMPLEX_MODE::COMPLEX_PHASE_RAD:
			return "phase [RAD]";
		default:
			return "";
	}
}

std::string TransientGUIHandler::get_transientPlotWindow_xAxisLabel_text() const{
	switch(x_val){
		case TransSpect::X_VALUE::X_POINT:
			return "measurement point";
		case TransSpect::X_VALUE::X_TIME:
			return "measurement time [s]";
		case TransSpect::X_VALUE::X_TIME_DIFF:
			return "measurement time [s]";
		default:
			return "";
	}
}