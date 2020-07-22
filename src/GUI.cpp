#include "SpectrometerTask.h"
#include "TestTask.h"
#include "GUI.h"
#include "FSHelper.h"
#include "DataP.h"
#include "TransImpTask.h"
#include "ImpAnalyserTask.h"
#include "HP4294A.h"
#include "Novocontrol.h"
#include "I2CTempTask.h"
#include "DelayTask.h"
#include "I2CFreqTask.h"
#include "I2CVoltageTask.h"
#include "DummyImpAnalyser.h"
#include "Freq_Test.h"
#include "Addresses.h"
#include "EmStatPico.h"
#include "DialogExtVolt.h"

#include <wiringPi.h>
#include <bitset>
#include <iostream> 
#include <vector>
#include <sys/types.h>
#include <pthread.h>
#include <sys/stat.h>
#include <ctime>
#include <fstream>
#include <stdio.h>

#define LOAD_WIDGET(WNAME, ONAME) refBuilder->get_widget(WNAME, ONAME); if(!ONAME) throw std::runtime_error(WNAME + endOfMessage);
#define LOAD_WIDGET_DERIVED(WNAME, ONAME, OTYPE) refBuilder->get_widget_derived<OTYPE>(WNAME, ONAME); if(!ONAME) throw std::runtime_error(WNAME + endOfMessage);
#define LOAD_OBJECT(GNAME, ONAME, OTYPE) ONAME = Glib::RefPtr<OTYPE>::cast_dynamic(refBuilder->get_object(GNAME)); if(!ONAME) throw std::runtime_error(GNAME + endOfMessage);
#define CONNECT_SIGNAL_CLICKED(BNAME, FKT) BNAME->signal_clicked().connect(sigc::mem_fun(*this, &GUI::FKT)); 
/// path of the gui file
#define GLADEFILE_PATH "../glade/ewod_gui.glade"
#define CSSFILE_PATH "../glade/styles.css"
#define PREF_FILE_FOLDER "./pref/"
#define PREF_FILE_NAME "pref.xml"

GUI::GUI(Logbook::Logbook_ptr l){
	log = l;
	app = Gtk::Application::create();
	
	try{
		init_loadPreferences();
		init_grabWidgetsFromBuilder();
		init_loadCSSFile();
		init_variables();
		init_connectSignals();
		init_loadRecipes();
		init_setRelais();
		
		
		on_button_autorefresh_toggled(); //start update thread
		on_button_fullscreen_toggled();
	}catch (std::runtime_error ex){
		std::cout << "fatal error during startup - " << ex.what() << std::endl;
	}
	
}
void GUI::init_loadRecipes(){
	std::vector<Task::Task_ptr> recipes;
	std::vector<std::string>* foldersRecipes = pref.getFolders_recipes();
	for (std::vector<std::string>::const_iterator cit = foldersRecipes->cbegin(); cit != foldersRecipes->cend(); cit++){
		DIR *dirp = opendir(cit->c_str());
		if (dirp != nullptr){
			struct dirent *dp;
			
			while((dp = readdir(dirp)) != NULL){
				std::string path = FSHelper::composePath(*cit, dp->d_name); // full path of file in the folder
				if (FSHelper::endsWith(path, ".csv") || FSHelper::endsWith(path, ".xml")){
					recipes.push_back(Recipe::loadRecipe(path, &spectrometer));
				}
			}
			closedir(dirp);
		}else {
			std::cout << "failed to open folder - " << cit->c_str() << std::endl;
		}
	}	
}
void GUI::init_grabWidgetsFromBuilder(){
	refBuilder = Gtk::Builder::create(); //Load the GtkBuilder file and instantiate its widgets:
	try {
		refBuilder->add_from_file(GLADEFILE_PATH);
		std::string endOfMessage =  std::string(" not found in glade file '").append(GLADEFILE_PATH).append("'");
		
		LOAD_WIDGET("mainwindow", mainWindow);
		LOAD_WIDGET("button_voltContr_refresh", button_voltContr_refresh);
		LOAD_WIDGET("label_voltage_controller", label_voltage_controller);
		LOAD_WIDGET("label_frequency_generator", label_frequency_generator);
		LOAD_WIDGET("label_voltContr_voltage", label_voltContr_voltage);
		LOAD_WIDGET("label_voltContr_extVolt", label_voltContr_extvoltage);
		LOAD_WIDGET("label_voltContr_svoltage", label_voltContr_svoltage);
		LOAD_WIDGET("label_voltContr_dcycle", label_voltContr_dcycle);
		LOAD_WIDGET("label_voltContr_mode", label_voltContr_mode);
		LOAD_WIDGET("label_voltContr_freq", label_voltContr_freq);
		LOAD_WIDGET("label_freqGen_freq", label_freqGen_freq);
		LOAD_WIDGET("button_freqGen_refresh", button_freqGen_refresh);
		LOAD_WIDGET("button_overview_general_refresh", button_overview_general_refresh);
		LOAD_WIDGET("entry_pads", entryPads);
		LOAD_WIDGET("textView_log", textViewLog);
		LOAD_WIDGET("treeView1", treeView_allRecipes);
		LOAD_WIDGET("treeView2", treeView_myRecipe);
		LOAD_WIDGET("button_addToMyRecipe", button_addToMyRecipe);
		LOAD_WIDGET("button_duplicateTask", button_duplicateTask);
		LOAD_WIDGET("button_removeFromMyRecipe", button_removeFromMyRecipe);
		LOAD_WIDGET("button_runCustomRecipe", button_runMyRecipe);
		LOAD_WIDGET("button_stopMyRecipe", button_stopMyRecipe);
		LOAD_WIDGET("button_saveMyRecipe", button_saveMyRecipe);
		LOAD_WIDGET("button_clearMyReicpe", button_clearMyRecipe);
		LOAD_WIDGET("button_pads_execute", buttonPadsExecute);
		LOAD_WIDGET("button_saveProject", button_saveProject);
		LOAD_WIDGET("button_record", button_record);
		LOAD_WIDGET("button_autorefresh", button_autorefresh);
		LOAD_WIDGET("button_shutdown", button_shutdown);
		LOAD_WIDGET("button_setFrequency", button_setFrequency);
		LOAD_WIDGET("button_setVoltage", button_setVoltage);
		LOAD_WIDGET("button_setDutyCycle", button_setDutyCycle);
		LOAD_WIDGET("button_setVoltFreq", button_setVoltPwmFreq);
		LOAD_WIDGET("button_fullscreen", button_fullscreen);
		LOAD_WIDGET("button_page_addTask_page_PadTask_add", button_addPadTask);
		LOAD_WIDGET("button_page_addTask_page_impTask_addTask", button_addHP4294ATask);
		LOAD_WIDGET("button_page_addTask_page_SpectrometerTask_addTask", button_addSpectrometerTask);
		LOAD_WIDGET("button_page_exData_page_overview_project", button_selectProject);
		LOAD_WIDGET("button_page_exData_page_overview_spectrum", button_selectSpectrum);
		LOAD_WIDGET("checkbutton_videoPreview", checkbutton_preview);
		LOAD_WIDGET("checkButton_page_addTask_page_impTask_transientTask", checkbutton_impTask_transient);
		LOAD_WIDGET("entry_gpib_slaveAddress", entry_gpib_slaveAddress);
		LOAD_WIDGET("entry_gpib_message", entry_gpib_message);
		LOAD_WIDGET("button_gpib_send", button_gpib_send);
		LOAD_WIDGET("button_gpib_read", button_gpib_read);
		LOAD_WIDGET("button_page_exData_page_overview_experiment", button_selectExperiment);
		LOAD_WIDGET("textView_gpib_message", textView_gpib_message);
		LOAD_WIDGET("dialog_save", dialogSave);
		LOAD_WIDGET("dialogSave_button_cancel", dialogSave_button_cancel);
		LOAD_WIDGET("dialogSave_button_save", dialogSave_button_save);
		LOAD_WIDGET("button_page_addTask_page_freq_addTask", button_addFreqTask);
		LOAD_WIDGET("button_page_addTask_page_voltage_addTask", button_addVoltageTask);
		LOAD_WIDGET("dialogSave_entry_name", dialogSave_entry_name);
		LOAD_WIDGET("dialogSave_label_description", dialogSave_label_description);
		LOAD_WIDGET("label_page_exData_page_overview_experiment", label_experiment_selectedProject);
		LOAD_WIDGET("label_page_exData_page_overview_spectrum", label_spectrum_selectedExperiment);
		LOAD_WIDGET("label_page_exData_Impedance_title", label_impedance_plot_title);
		LOAD_WIDGET("label_page_exData_Impedance_nyquist_title", label_impedance_plot_nyquist_title);
		LOAD_WIDGET("label_page_exData_Spectrometer_title", label_spectrum_plot_title);
		LOAD_WIDGET("label_page_addTask_page_impTask_termValue", label_impTask_transient_termValue);
		LOAD_WIDGET("label_page_addTask_page_impTask_bw", label_impTask_bw);
		LOAD_WIDGET("scale_page_addTask_page_impTask_bw", scale_impTask_bw);
		LOAD_WIDGET("notebook_page_exData", notebook_page_exData);
		LOAD_WIDGET("notebook_mainWindow", notebook_main);
		LOAD_WIDGET("radiobutton_page_exData_page_impedance_transientView_abs", transientViewHandler.radioButton_transient_abs);
		LOAD_WIDGET("radiobutton_page_exData_page_impedance_transientView_phase", transientViewHandler.radioButton_transient_phase);
		LOAD_WIDGET("radiobutton_page_exData_page_impedance_transientView_real", transientViewHandler.radioButton_transient_real);
		LOAD_WIDGET("radiobutton_page_exData_page_impedance_transientView_imag", transientViewHandler.radioButton_transient_imag);
		LOAD_WIDGET("textView_page_exData_page_impedance_transientView", transientViewHandler.textView_dataInfo);
		LOAD_WIDGET("radioButton_page_addTask_page_impTask_termTime", radiobutton_impTask_transient_termTime);
		LOAD_WIDGET("radioButton_page_addTask_page_impTask_termPoints", radiobutton_impTask_transient_termPoints);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_type_hp4294A", radiobutton_impTask_type_hp4294a);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_type_novocontrol", radiobutton_impTask_type_novocontrol);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_type_emstat", radiobutton_impTask_type_emstat);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_two_wire", radiobutton_impTask_wire_twoWire);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_three_wire", radiobutton_impTask_wire_threeWire);
		LOAD_WIDGET("radiobutton_page_addTask_page_impTask_four_wire", radiobutton_impTask_wire_fourWire);
		LOAD_WIDGET("spinbutton_page_addTask_page_impTask_termSeconds", spinButton_impTask_term_Second);
		LOAD_WIDGET("spinbutton_page_addTask_page_impTask_termMinutes", spinButton_impTask_term_Minute);
		LOAD_WIDGET("spinbutton_page_addTask_page_impTask_termHours", spinButton_impTask_term_Hour);
		LOAD_WIDGET("label_page_addTask_page_impTask_termHours", label_impTask_term_Hour);
		LOAD_WIDGET("label_page_addTask_page_impTask_termMinutes", label_impTask_term_Minute);
		LOAD_WIDGET("label_page_addTask_page_impTask_termSeconds", label_impTask_term_Second);
		LOAD_WIDGET("box_page_addTask_page_impTask_wireMode", box_impTask_wire);
		LOAD_WIDGET("label_page_addTask_page_impTask_wireMode", label_impTask_wire);
		LOAD_WIDGET("scale_page_addTask_page_impTask_termValue", scale_impTask_transient_termValue);
		LOAD_WIDGET("spinButton_page_addTask_page_impTask_termValue", spinButton_impTask_transient_termValue);
		LOAD_WIDGET("checkbutton_voltage_controller", checkbutton_voltageContr_connected);
		LOAD_WIDGET("checkbutton_frequency_generator", checkbutton_freqGen_connected);
		LOAD_WIDGET("checkbutton_spectrometer", checkbutton_spectrometer_connected);
		LOAD_WIDGET("checkbutton_hp4294a", checkbutton_hp429a_connected);
		LOAD_WIDGET("checkbutton_novocontrol", checkbutton_novocontrol_connected);
		LOAD_WIDGET("checkbutton_atmega32", checkbutton_atmega32_connected);
		LOAD_WIDGET("checkbutton_page_general_relais_ac", checkbutton_relais_ac);
		LOAD_WIDGET("checkbutton_page_general_relais_boost_in", checkbutton_relais_boost_in);
		LOAD_WIDGET("checkbutton_page_general_relais_hv_ext", checkbutton_relais_hv_ext);
		LOAD_WIDGET("checkbutton_page_general_relais_ac_2", checkbutton_relais_ac_2);
		LOAD_WIDGET("checkbutton_page_general_relais_safety", checkbutton_relais_safety);
		LOAD_WIDGET("checkbutton_page_general_relais_imp_ewod", checkbutton_relais_imp_ewod);
		LOAD_WIDGET("checkbutton_page_general_relais_we_p_wt_s", checkbutton_relais_we_p_wt_s);
		LOAD_WIDGET("checkbutton_page_general_relais_re_p_ce", checkbutton_relais_re_p_ce);
		LOAD_WIDGET("checkbutton_page_general_relais_ext_pico", checkbutton_relais_ext_pico);
		LOAD_WIDGET("button_page_general_relais_read", button_relais_read);
		LOAD_WIDGET("button_page_general_relais_write", button_relais_write);
		LOAD_WIDGET("button_page_pref_restore", button_page_pref_restore);
		LOAD_WIDGET("button_page_pref_save", button_page_pref_save);
		LOAD_WIDGET("button_page_pref_recipeFolder_browse", button_page_pref_recipeFolder_browse);
		LOAD_WIDGET("button_page_pref_projectFolder_browse", button_page_pref_projectFolder_browse);
		LOAD_WIDGET("entry_page_pref_projectFolder", entry_page_pref_projectFolder);
		LOAD_WIDGET("entry_page_pref_recipeFolder", entry_page_pref_recipeFolder);
		LOAD_WIDGET("checkbutton_waitForVoltage", checkbutton_waitForVoltage);
		LOAD_WIDGET("button_page_addTask_page_delay_add", button_addDelayTask);
		LOAD_WIDGET("button_page_addTask_page_PadTask_activate", button_activatePad);
		LOAD_WIDGET("notebook_overview", notebook_overview);
		LOAD_WIDGET("radioButton_page_addTask_page_voltage_internal", radioButton_page_addTask_page_voltage_internal);
		LOAD_WIDGET("radioButton_page_addTask_page_voltage_external", radioButton_page_addTask_page_voltage_external);
		
		LOAD_WIDGET_DERIVED("drawingArea_page_exData_Spectrometer", plotWindowSpectrum, PlotWindow);
		LOAD_WIDGET_DERIVED("drawingArea_page_exData_Impedance_abs", plotWindowImpedance_Abs, PlotWindow);
		LOAD_WIDGET_DERIVED("drawingArea_page_exData_Impedance_phase", plotWindowImpedance_Phase, PlotWindow);
		LOAD_WIDGET_DERIVED("drawingArea_page_exData_Impedance_nyquist", plotWindowImpedance_Nyquist, PlotWindow);
		LOAD_WIDGET_DERIVED("drawingArea_page_exData_page_impedance_transientView", transientViewHandler.plotWindow_transSpectr, PlotWindow);
		LOAD_WIDGET_DERIVED("treeview_page_exData_page_impedance_transientView_frequencies", transientViewHandler.listView_frequencies, ListView_Freq);
		LOAD_WIDGET_DERIVED("treeview_page_exData_page_overview_project", listView_projects, ListView_Project);
		LOAD_WIDGET_DERIVED("treeview_page_exData_page_overview_experiment", listView_experiments, ListView_Experiment);
		LOAD_WIDGET_DERIVED("treeview_page_exData_page_overview_spectrum", listView_spectrums, ListView_SavedData);
		LOAD_WIDGET_DERIVED("image_preview", image_preview, CameraWindow);
		LOAD_WIDGET_DERIVED("dialog_extVolt", I2CVoltageTask::dialogExtVolt, DialogExtVolt);
		
		LOAD_OBJECT("adjustment_frequency", adjustment_frequency, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_voltage", adjustment_voltage, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_dutyCycle", adjustment_dutyCycle, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_voltageFreq", adjustment_pwmFreq, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_PadTask_time", adjustment_PadTask_time, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_SpectrometerTask_integrationtime_us", adjustment_SpectrometerTask_integrationTime, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_SpectrometerTask_scansToAverage", adjustment_SpectrometerTask_scansToAverage, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_startFreq", adjustment_impTask_startFreq, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_stopFreq", adjustment_impTask_stopFreq, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_voltage", adjustment_impTask_volt, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_points", adjustment_impTask_points, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_bw", adjustment_impTask_bw, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_pointAverage", adjustment_impTask_pointAverage, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_transTermValue", adjustment_impTask_transTermValue, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_termHours", adjustment_impTask_termHours, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_termMinutes", adjustment_impTask_termMinutes, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_impTask_termSeconds", adjustment_impTask_termSeconds, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_freq_freq", adjustment_page_addTask_page_freq_freq, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_voltage_voltage", adjustment_page_addTask_page_voltage_voltage, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_delay_seconds", adjustment_page_addTask_page_delay_seconds, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_delay_milliseconds", adjustment_page_addTask_page_delay_milliseconds, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_delay_minutes", adjustment_page_addTask_page_delay_minutes, Gtk::Adjustment);
		LOAD_OBJECT("adjustment_page_addTask_page_delay_hours", adjustment_page_addTask_page_delay_hours, Gtk::Adjustment);
		
		
		std::string toggleButtonName = "";
		for (int i = TOGGLEBUTTON_PADNO_FIRST; i <= TOGGLEBUTTON_PADNO_LAST; i++){
			toggleButtonName = "button_page_addTask_page_PadTask_pad" + std::to_string(i);
			
			LOAD_WIDGET(toggleButtonName, toggleButtonPads[i- TOGGLEBUTTON_PADNO_FIRST]);
		}
	}catch(const Glib::FileError& ex){
		throw std::runtime_error("FileError: " + ex.what());
	} catch(const Glib::MarkupError& ex) {
		throw std::runtime_error("FileError: " + ex.what());
	} catch(const Gtk::BuilderError& ex){
		throw std::runtime_error("BuilderError: " + ex.what());
	}
}
void GUI::init_connectSignals(){
	CONNECT_SIGNAL_CLICKED(button_addToMyRecipe, on_buttonAddToMyRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_duplicateTask, on_buttonDuplicateTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_voltContr_refresh, on_button_voltContr_refresh_clicked);
	CONNECT_SIGNAL_CLICKED(button_freqGen_refresh, on_button_freqGen_refresh_clicked);
	CONNECT_SIGNAL_CLICKED(button_overview_general_refresh, on_button_overview_general_refresh_clicked);
	CONNECT_SIGNAL_CLICKED(button_runMyRecipe, on_buttonRunRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_stopMyRecipe, on_buttonStopRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_saveMyRecipe, on_buttonSaveMyRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_addDelayTask, on_buttonAddDelayTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_addSpectrometerTask, on_buttonAddSpectrometerTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_addPadTask, on_buttonAddPadTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_addHP4294ATask, on_buttonAddImpTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_addVoltageTask, on_buttonAddVoltageTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_addFreqTask, on_buttonAddFreqTask_clicked);
	CONNECT_SIGNAL_CLICKED(buttonPadsExecute, on_buttonExecutePadTask_clicked);
	CONNECT_SIGNAL_CLICKED(button_saveProject, on_buttonSaveProject_clicked);
	CONNECT_SIGNAL_CLICKED(button_setFrequency, on_buttonSetFrequency_clicked);
	CONNECT_SIGNAL_CLICKED(button_setVoltage, on_buttonSetVoltage_clicked);
	CONNECT_SIGNAL_CLICKED(button_setVoltPwmFreq, on_button_setPwmFreq_clicked);
	CONNECT_SIGNAL_CLICKED(button_setDutyCycle, on_buttonSetDutyCycle_clicked);
	CONNECT_SIGNAL_CLICKED(button_clearMyRecipe, on_buttonClearMyRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_selectProject, on_buttonSelectProject_clicked);
	CONNECT_SIGNAL_CLICKED(button_selectExperiment, on_buttonSelectExperiment_clicked);
	CONNECT_SIGNAL_CLICKED(button_selectSpectrum, on_buttonSelectSpectrum_clicked);
	CONNECT_SIGNAL_CLICKED(dialogSave_button_cancel, dialogSave_on_buttonCancel_clicked);
	CONNECT_SIGNAL_CLICKED(button_gpib_read, on_buttonGpibRead_clicked);
	CONNECT_SIGNAL_CLICKED(button_gpib_send, on_buttonGpibSend_clicked);
	CONNECT_SIGNAL_CLICKED(dialogSave_button_save, dialogSave_on_buttonSave_clicked);
	CONNECT_SIGNAL_CLICKED(button_relais_read, on_button_read_realis_clicked);
	CONNECT_SIGNAL_CLICKED(button_relais_write, on_button_write_realis_clicked);
	CONNECT_SIGNAL_CLICKED(button_removeFromMyRecipe, on_buttonRemoveFromMyRecipe_clicked);
	CONNECT_SIGNAL_CLICKED(button_page_pref_save, on_button_page_pref_save_clicked);
	CONNECT_SIGNAL_CLICKED(button_page_pref_restore, on_button_page_pref_restore_clicked);
	CONNECT_SIGNAL_CLICKED(button_page_pref_recipeFolder_browse, on_button_page_pref_recipeFolder_browse_clicked);
	CONNECT_SIGNAL_CLICKED(button_page_pref_projectFolder_browse, on_button_page_pref_projectFolder_browse_clicked);
	CONNECT_SIGNAL_CLICKED(button_activatePad, on_buttonActivatePad_clicked);
	CONNECT_SIGNAL_CLICKED(button_shutdown, on_button_shutdown_clicked);
	
	checkbutton_preview->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_checkbuttonPreview_toggled));
	radiobutton_impTask_type_hp4294a->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_addImpTask_radiobutton_changed));
	radiobutton_impTask_type_novocontrol->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_addImpTask_radiobutton_changed));
	radiobutton_impTask_type_emstat->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_addImpTask_radiobutton_changed));
	radiobutton_impTask_transient_termTime->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_checkbutton_addImpTask_termMode_toggled));
	radiobutton_impTask_transient_termPoints->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_checkbutton_addImpTask_termMode_toggled));
	checkbutton_impTask_transient->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_checkbutton_addImpTask_transient_toggled));
	button_autorefresh->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_button_autorefresh_toggled));
	button_fullscreen->signal_toggled().connect(sigc::mem_fun(*this, &GUI::on_button_fullscreen_toggled));
	
	logEventDispatcher.connect(sigc::mem_fun(*this, &GUI::logEventDispatcherFunction));
	camera.connect_onFrameCapturedListener(sigc::mem_fun(*this, &GUI::on_preview_captured));
	mainWindow->signal_delete_event().connect(sigc::mem_fun(*this, &GUI::on_window_close));
	updateTimer.connect(sigc::mem_fun(*this, &GUI::updateThread));
}
void GUI::init_variables(){
	recTv1 = new TreeView_Recipe(treeView_allRecipes, mainWindow);
	recTv2 = new TreeView_Recipe(treeView_myRecipe, mainWindow);
	
	thread_execute_MyRecipe_data = nullptr;
	gpib = nullptr;
	status_leds = StatusLed::create();
	relais = Relais::create();
	
	Task::status_leds = status_leds;
	Task::relais = relais;
	
	
	listView_projects->scanFolder(pref.getFolderProjects());
	
	transientViewHandler.plotWindow_Abs = plotWindowImpedance_Abs;
	transientViewHandler.plotWindow_Nyquist = plotWindowImpedance_Nyquist;
	transientViewHandler.plotWindow_Phase = plotWindowImpedance_Phase;
	transientViewHandler.label_impedance_plot_nyquist_title = label_impedance_plot_nyquist_title;
	transientViewHandler.label_impedance_plot_title = label_impedance_plot_title;
	transientViewHandler.init();
	
	on_button_page_pref_restore_clicked();
	on_button_overview_general_refresh_clicked();
	
	adjustment_voltage->set_upper(MAX_OUT_VOLT_BOOST);
	adjustment_page_addTask_page_voltage_voltage->set_upper(MAX_OUT_VOLT_BOOST);
	
//	GpibConnection c(8);
//	c.send("*IDN?");
//	std::cout << "*IDN?: " << c.read() << std::endl;
}
void GUI::init_loadPreferences(){
	std::string path = FSHelper::composePath(PREF_FILE_FOLDER, PREF_FILE_NAME);
	
	if (!FSHelper::folderExists(PREF_FILE_FOLDER)){
		FSHelper::createDirectory(PREF_FILE_FOLDER);
	}
	
	//load / create preferences 
	if (pref.loadFromFile(path) != 0){ //pref file does not exist
		if (!FSHelper::folderExists("./recipes/")){
			FSHelper::createDirectory("./recipes/");
		}
		pref.addRecipesFolder("./recipes/");
		
		if (!FSHelper::folderExists("./recipes/myRecipes/")){
			FSHelper::createDirectory("./recipes/myRecipes/");
		}
		pref.addRecipesFolder("./recipes/myRecipes/");
		pref.setFolderMyRecipes("./recipes/myRecipes/");
		
		if (!FSHelper::folderExists("./projects/")){
			FSHelper::createDirectory("./projects/");
		}
		pref.setFolderProjects("./projects/");
		
		
		pref.saveToFile(path);
	}
}
void GUI::init_loadCSSFile(){
	Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path(CSSFILE_PATH);
	Glib::RefPtr<Gtk::StyleContext> styleContext = Gtk::StyleContext::create();
	Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
	styleContext->add_provider_for_screen(screen, cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void GUI::init_setRelais(){
		relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_BOOST_IN, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, true);
	relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_AC_2, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, false);
	relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, false);
	//relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, checkbutton_relais_ext_pico->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, true); //external measurement
	relais->writeRelais();
}

GUI::~GUI(){
	delete mainWindow;
	delete entryPads;
	delete buttonPadsExecute;
	delete button_addToMyRecipe;
	delete button_removeFromMyRecipe;
	delete button_runMyRecipe;
	delete button_saveMyRecipe;
	
}

void GUI::on_buttonAddToMyRecipe_clicked(){
	Task::Task_ptr selectedTask = recTv1->getSelectedTask();
	if (selectedTask != nullptr){
		recTv2->addTask(selectedTask);
	}else{
		std::cout << "on_buttonAddToMyRecipe_clicked received nullptr" << std::endl;
	}
}
void GUI::on_buttonDuplicateTask_clicked(){
	Task::Task_ptr selectedTask = recTv2->getSelectedTask();
	if (selectedTask != nullptr){
		recTv2->addTask(selectedTask);
	}else{
		std::cout << "on_buttonDuplicateTask_clicked received nullptr" << std::endl;
	}
}
void GUI::on_buttonRemoveFromMyRecipe_clicked(){
	recTv2->deleteSelectedRow();
}

void* GUI::executemyrecipe(void *d){
	ExecuteMyRecipeThreadData* data = static_cast<ExecuteMyRecipeThreadData*>(d);
	try{
		//execute all tasks
		data->recipe->execute(data->pData, data->executeNext);
	}catch (std::runtime_error r){
		data->error = new std::runtime_error(r);
	}
	
	data->finished = true;
//	std::cout << "emitting..." << std::endl;
	data->dispatcher.emit();
//	std::cout << "done here..." << std::endl;
}
void GUI::on_finished_executing_myRecipe(){
	if (on_finished_executing_myRecipeMtx.try_lock()){
		if (thread_execute_MyRecipe_data != nullptr){
			
			if (thread_execute_MyRecipe_data->error != nullptr) { //diplaying error message box
				Gtk::MessageDialog dialog(*mainWindow, "error occured during recipe", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
				dialog.set_secondary_text(thread_execute_MyRecipe_data->error->what());
				dialog.run();
				
				delete thread_execute_MyRecipe_data->error;
				thread_execute_MyRecipe_data->error = nullptr;
			}
			
			if(thread_execute_MyRecipe_data->finished){
				//switch relais
				relais->setRelais(Relais::RELAIS::R_STEUER_AC, false);
				//relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, false);
				relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, true);
				relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, false);
				//relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, false);
				relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, true);
				relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, false);
				relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, false);
				relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, false);
				try{
					relais->writeRelais();
					log->add_event(Log_Event::create("resetting relais", "relais have been resetted", Log_Event::TYPE::LOG_INFO));
				}catch (std::runtime_error &e){
					Gtk::MessageDialog dialog(*mainWindow, "error writing relais", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
					dialog.set_secondary_text(e.what());
					dialog.run();
				} 
				
				status_leds->recipeRunning(false);
				status_leds->write_reg_val();
				
				log->add_event(Log_Event::create("execution finished", "the custom recipe is finished", Log_Event::TYPE::LOG_INFO));
				log->set_temp_Logbook(Logbook::create());
				thread_execute_MyRecipe_data->pData->saveLogfile();
				
				if (button_record->get_active()){
					camera.stopRecord();
				}
				
//				if (checkbutton_preview->get_active()){
//					startPreview();
//				}
				
				
				//enable / disable widgets
				button_addToMyRecipe->set_sensitive(true);
				button_runMyRecipe->set_sensitive(true);
				button_stopMyRecipe->set_sensitive(false);
				button_removeFromMyRecipe->set_sensitive(true);
				buttonPadsExecute->set_sensitive(true);
				recTv2->set_TreeView_sensitive(true);
				checkbutton_preview->set_sensitive(true);
				button_record->set_sensitive(true);
				button_shutdown->set_sensitive(true);
				
				//display spectre
				if (thread_execute_MyRecipe_data->pData->capturedSpectrum()){
					setSpectrum(thread_execute_MyRecipe_data->pData->getLastSpectreDataPoints(), "last measured spectrum");
				}
				
				if (thread_execute_MyRecipe_data->pData->capturedImpedance()){
					setImpSpectrum(thread_execute_MyRecipe_data->pData->getLastImpedanceDataPoints(), "last measured impedance spectrum");
				}
				
				delete thread_execute_MyRecipe_data;
				thread_execute_MyRecipe_data = nullptr;
			}
		}
		on_finished_executing_myRecipeMtx.unlock();
	}
}
void GUI::on_buttonRunRecipe_clicked(){
	Recipe::Recipe_ptr r = recTv2->getAllTasksAsOneRecipe();
	std::list<Task::DEVICES> necessaryDevices = r->getNecessaryDevices();
	bool unconnected_device = false;
	std::string unconnected_devices = "\n";
	
	for(std::list<Task::DEVICES>::iterator it = necessaryDevices.begin(); it != necessaryDevices.end(); it++){
		Task::DEVICES d = *it;
		switch (d){
			case Task::DEVICES::DEVICE_ATTINY_VOLT:{
				Uc_Connection::Uc_Connection_ptr attiny45volt = Uc_Connection::create(I2C_ATTINY45_VOLT_SLAVE_ADDRESS);
				if (!attiny45volt->isConnected()){
					unconnected_device = true;
					unconnected_devices += "-voltage controller\n";
				}
//				std::cout << "needed: attiny45 volt" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_ATTINY_FREQ:{
				Uc_Connection::Uc_Connection_ptr attiny45freq = Uc_Connection::create(I2C_ATTINY45_FREQ_SLAVE_ADDRESS);
				if (!attiny45freq->isConnected()){
					unconnected_device = true;
					unconnected_devices += "-frequency generator\n";
				}
//				std::cout << "needed: attiny45 freq" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_ATMEGA_GENERAL:{
				Uc_Connection::Uc_Connection_ptr atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
				if (!atmega32->isConnected()){
					unconnected_device = true;
					unconnected_devices += "-atmega32 uc\n";
				}
//				std::cout << "needed: attmega32" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_NOVOCONTROL:{
				GpibConnection novo(GPIB_NOVOCONTROL);
				if (!novo.isConnected()){
					unconnected_device = true;
					unconnected_devices += "-novocontrol impedance analyser\n";
				}
//				std::cout << "needed: novocontrol" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_HP4294A:{
				GpibConnection hp(GPIB_HP4294A);
				if (!hp.isConnected()){
					unconnected_device = true;
					unconnected_devices += "-HP4924A impedance analyser\n";
				}
//				std::cout << "needed: hp4294a" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_EMPICO:{
//				std::cout << "needed: EmStatPico" << std::endl;
				break;
			}
			case Task::DEVICES::DEVICE_SPECTROMETER:{
				Spectrometer::Spectrometer_ptr spec = Spectrometer::create();
				if (!spec->isConnected()){
					unconnected_device = true;
					unconnected_devices += "-ocean optics spectrometer\n";
				}
//				std::cout << "needed: spectrometer" << std::endl;
				break;
			}
		}
	}
	
	if (unconnected_device){ // there is at least one unconnected device
		unconnected_devices = unconnected_devices.substr(0, unconnected_devices.size() - 1);
		Gtk::MessageDialog dialog(*mainWindow, "unconnected devices", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text("please connect the following devices:\n" + unconnected_devices);
		dialog.run();
	}else{
		
		dialogSave_label_description->set_text("name of the project");
		dialogSave_entry_name->get_buffer()->set_text(pref.getCurrentProjectName());
		dialogSave->set_transient_for(*mainWindow);
		
		if (dialogSave->run() == Gtk::RESPONSE_ACCEPT){
			if (pref.getCurrentProjectName().compare(dialogSave_entry_name->get_buffer()->get_text()) != 0){ // project name changed
				pref.setCurrentProjectName(dialogSave_entry_name->get_buffer()->get_text());
			}
			std::string projectPath = FSHelper::composePath(pref.getFolderProjects(), pref.getCurrentProjectName());
			listView_projects->addProject(projectPath);
			
//			while (folderExists(projectPath)){
//				
//			}
			
			thread_execute_MyRecipe_data = new ExecuteMyRecipeThreadData();
			ExperimentData::ExperimentData_ptr exDataPtr = std::make_shared<ExperimentData>(pref.getCurrentProjectName(), projectPath);
			const sigc::slot<void, TransSpect::TransSpect_ptr, TransSpect::Spectrum, unsigned int, double, std::string> slot = sigc::mem_fun5<TransSpect::TransSpect_ptr, TransSpect::Spectrum, unsigned int, double, std::string>(transientViewHandler, &TransientGUIHandler::onTransImpSpecAdded);
			exDataPtr->connectOnTransImpSpecAddedListener(slot);
			
			thread_execute_MyRecipe_data->recipe = r;
			*(thread_execute_MyRecipe_data->executeNext) = true;
			thread_execute_MyRecipe_data->dispatcher.connect(sigc::mem_fun(*this, &GUI::on_finished_executing_myRecipe));
			thread_execute_MyRecipe_data->pData = exDataPtr;
			log->set_temp_Logbook(thread_execute_MyRecipe_data->pData->log);
			r->save(thread_execute_MyRecipe_data->pData->getRecipePath());
			
			//disable / enable widgets
			button_addToMyRecipe->set_sensitive(false);
			button_runMyRecipe->set_sensitive(false);
			button_stopMyRecipe->set_sensitive(true);
			button_removeFromMyRecipe->set_sensitive(false);
			buttonPadsExecute->set_sensitive(false);
			recTv2->set_TreeView_sensitive(false);
			checkbutton_preview->set_sensitive(false);
			button_record->set_sensitive(false);
			button_shutdown->set_sensitive(false);
			
			if (button_record->get_active()){
				camera.startRecord(thread_execute_MyRecipe_data->pData->getVideoPath());
			}
			
			status_leds->recipeRunning(true);
			status_leds->write_reg_val();
			
			log->add_event(Log_Event::create("execution started", "user started custom recipe - project '" + pref.getCurrentProjectName() + "'", Log_Event::TYPE::LOG_INFO));
			pthread_create(&thread_execute_MyRecipe, NULL, executemyrecipe, thread_execute_MyRecipe_data);
			
			dialogSave_entry_name->get_buffer()->set_text("");
		}
	}
}
void GUI::on_buttonStopRecipe_clicked(){
	if ( thread_execute_MyRecipe_data != nullptr ){
		log->add_event(Log_Event::create("recipe canceled", "the execution of the recipe has been canceled by the user", Log_Event::TYPE::LOG_INFO));
		
		//stop the thread
		*(thread_execute_MyRecipe_data->executeNext) = false;
		pthread_join(thread_execute_MyRecipe, NULL);
	}
}

void GUI::on_buttonSaveProject_clicked(){

	std::time_t time = std::time(0);
	std::tm *ptm = std::localtime(&time);
	char buffer[17];
	std::strftime(buffer, 17, "%Y-%m-%d_%H.%M", ptm);
	
	std::string name_folder_project = buffer;
	std::string folder_projects = pref.getFolderProjects();
	std::string folder_project = FSHelper::composePath(folder_projects, name_folder_project);
	
	if (FSHelper::createDirectory(folder_project) != -1){
		Recipe::Recipe_ptr myRecipe = recTv2->getAllTasksAsOneRecipe("myRecipe");
		pref.saveToFile(folder_project + "/pref.xml");
		log->save_log_file(folder_project + "/log.txt");
		myRecipe->save(folder_project + "/myRecipe.xml");
		
	}else{ // error creating folder
		log->add_event(Log_Event::create("error creating folder", "folder '" + folder_project + "' could not be created", Log_Event::TYPE::LOG_ERROR));
	}
}

void GUI::startPreview(){
	std::cout << "starting preview..." << std::endl;
	camera.startPreview();
}
void GUI::stopPreview(){
	camera.stopPreview();
}
void GUI::on_checkbuttonPreview_toggled(){
	if (checkbutton_preview->get_active()){ //start preview
		startPreview();
	}else{ //stop preview
		stopPreview();
	}
	
}
void GUI::on_preview_captured(Camera_cv::Image_ptr image){
	image_preview->setImage(image);
}

void GUI::on_buttonExecutePadTask_clicked(){
	std::string text = entryPads->get_text(); //get text from gui
	Task::Task_ptr task = PadTask::getPadTaskFromString(text);
	bool exNext = true;
	task->execute(std::make_shared<ExperimentData>("",""), &exNext);
}

void GUI::on_buttonSetFrequency_clicked(){
	I2CFreqTask t(adjustment_frequency->get_value());
	bool running = true;
	try{
		t.execute(0, &running);
	}catch (std::runtime_error r){
		Gtk::MessageDialog dialog(*mainWindow, "error occured", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(r.what());
		dialog.run();
	}
		
	
//	///@todo create FrequencyTask
////	ucControl->setFrequency(adjustment_frequency->get_value());
//	int freq = adjustment_frequency->get_value();
//	std::cout << "freq: " << freq << "Hz" << std::endl;
//	
//	char freq_0 =  (0x000000FF & freq);
//	char freq_1 = ((0x0000FF00 & freq) >> 8);
//	char freq_2 = ((0x00FF0000 & freq) >> 16);
//	char freq_3 = ((0xFF000000 & freq) >> 24);
//	
//	std::cout << std::bitset<8>(freq_3) << " - " << std::bitset<8>(freq_2) << " - " << std::bitset<8>(freq_1) << " - " << std::bitset<8>(freq_0) << std::endl;
//	
//	Uc_Connection::Uc_Connection_ptr tinyConnection= Uc_Connection::create(0x20);
//	tinyConnection->writeRegister(0x01, freq_0);
//	tinyConnection->writeRegister(0x02, freq_1);
//	tinyConnection->writeRegister(0x03, freq_2);
//	tinyConnection->writeRegister(0x04, freq_3);
}

void GUI::on_buttonSetVoltage_clicked(){
	I2CVoltageTask t(adjustment_voltage->get_value());
	t.setWaitForVoltage(checkbutton_waitForVoltage->get_active());
	bool running = true;
	
	try{
		t.execute(0, &running);
	}catch (std::runtime_error r){
		Gtk::MessageDialog dialog(*mainWindow, "error occured", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(r.what());
		dialog.run();
	}
	
//	///@todo I2C voltage task
//	
//	int voltage = adjustment_voltage->get_value();
//	
//	std::cout << "voltage: " << voltage << "V" << std::endl;
//	
//	int ADCS = ((double) VOLTAGE_R2) / (((double) VOLTAGE_R1) + ((double) VOLTAGE_R2)) * 1024.0 / 5.0 * ((double) voltage);
//	
//	
//	char ADC_SL =  (0x000000FF & ADCS);
//	char ADC_SH = ((0x0000FF00 & ADCS) >> 8);
//	
//	std::cout << std::bitset<8>(ADC_SH) << " - " << std::bitset<8>(ADC_SL) << std::endl;
//	
//	Uc_Connection::Uc_Connection_ptr tinyConnection= Uc_Connection::create(0x30);
//	tinyConnection->writeRegister(0x03, ADC_SL);
//	tinyConnection->writeRegister(0x04, ADC_SH);
}
void GUI::on_buttonSetDutyCycle_clicked(){
	I2CVoltageTask t;
	double dcycle = ((double) adjustment_dutyCycle->get_value()) / 100.0;
	
	t.setVoltageMode(I2CVoltageTask::VOLTAGE_MODE::MODE_DUTY_CYCLE);
	t.setDutyCycle(dcycle);
	std::cout << "dcycle: " << dcycle << std::endl;
	bool running = true;
	
	try{
		t.execute(0, &running);
	}catch (std::runtime_error r){
		Gtk::MessageDialog dialog(*mainWindow, "error occured", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(r.what());
		dialog.run();
	}
	
}

void GUI::on_buttonAddDelayTask_clicked(){
	int delayHours = adjustment_page_addTask_page_delay_hours->get_value();
	int delayMinutes = adjustment_page_addTask_page_delay_minutes->get_value();
	int delaySeconds = adjustment_page_addTask_page_delay_seconds->get_value();
	int delayMilliSeconds = adjustment_page_addTask_page_delay_milliseconds->get_value();
	int delaySeconds_total = delaySeconds + delayMinutes * 60 + delayHours * 3600;
	
	if (delaySeconds_total > 0 || delayMilliSeconds > 0){
		recTv2->addTask(DelayTask::create(delaySeconds_total, delayMilliSeconds));
	}else{
		Gtk::MessageDialog dialog(*mainWindow, "time is zero", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
		dialog.set_secondary_text("the delay time cannot be zero");
		dialog.run();
	}
}
void GUI::on_buttonAddVoltageTask_clicked(){
	unsigned int voltage = adjustment_page_addTask_page_voltage_voltage->get_value();
	I2CVoltageTask::VOLTAGE_MODE mode = I2CVoltageTask::VOLTAGE_MODE::MODE_CONTROLLER;
	
	
	if(radioButton_page_addTask_page_voltage_internal->get_active()){
		mode = I2CVoltageTask::VOLTAGE_MODE::MODE_CONTROLLER;
	}else if(radioButton_page_addTask_page_voltage_external->get_active()){
		mode = I2CVoltageTask::VOLTAGE_MODE::MODE_EXTERN;
	}
	recTv2->addTask(I2CVoltageTask::create(voltage, mode));
}
void GUI::on_buttonAddFreqTask_clicked(){
	recTv2->addTask(I2CFreqTask::create((unsigned int) adjustment_page_addTask_page_freq_freq->get_value()));
}
void GUI::on_buttonAddSpectrometerTask_clicked(){
	SpectrometerTask::SpectrometerTask_ptr s = std::make_shared<SpectrometerTask>(&spectrometer, Spectrometer::TRIGGER_MODE::TRIGGER_MODE_NORMAL, (long) adjustment_SpectrometerTask_integrationTime->get_value(), (int) adjustment_SpectrometerTask_scansToAverage->get_value());
	recTv2->addTask(s);
}
void GUI::on_buttonActivatePad_clicked(){
	PadTask::PadTask_ptr p = getPadTaskFromGUI();
	bool running = true;
	
	try{
		p->execute(0, &running);
	}catch (std::runtime_error r){
		Gtk::MessageDialog dialog(*mainWindow, "error occured", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(r.what());
		dialog.run();
	}
	
	
}
void GUI::on_buttonAddPadTask_clicked(){
	
	recTv2->addTask(getPadTaskFromGUI());
}
void GUI::on_addImpTask_radiobutton_changed(){
	bool hp4294a = radiobutton_impTask_type_hp4294a->get_active();
	bool novocontrol = radiobutton_impTask_type_novocontrol->get_active();
	bool emstat = radiobutton_impTask_type_emstat->get_active();
	
	label_impTask_bw->set_visible(hp4294a);
	scale_impTask_bw->set_visible(hp4294a);
	
	radiobutton_impTask_wire_fourWire->set_sensitive(!emstat);
	if (!radiobutton_impTask_wire_fourWire->get_sensitive() && radiobutton_impTask_wire_fourWire->get_active()){
		radiobutton_impTask_wire_threeWire->set_active(true);
	}
	
	ImpAnalyser::ImpAnalyser_ptr tmpAnalyser;
	if (hp4294a){
		tmpAnalyser = std::make_shared<HP4294A>(GPIB_HP4294A);
	}else if (novocontrol){
		tmpAnalyser = std::make_shared<Novocontrol>(GPIB_HP4294A);
	}else if (emstat){
		tmpAnalyser = std::make_shared<EmStatPico>();
	}
	
	
	adjustment_impTask_pointAverage->set_upper(tmpAnalyser->getMaxPointAverage());
	if (adjustment_impTask_pointAverage->get_value() > adjustment_impTask_pointAverage->get_upper()){
		adjustment_impTask_pointAverage->set_value(adjustment_impTask_pointAverage->get_upper());
	}else if (adjustment_impTask_pointAverage->get_value() < adjustment_impTask_pointAverage->get_lower()){
		adjustment_impTask_pointAverage->set_value(adjustment_impTask_pointAverage->get_lower());
	}
	
	adjustment_impTask_points->set_upper(tmpAnalyser->getMaxPoints());
	adjustment_impTask_points->set_lower(tmpAnalyser->getMinPoints());
	if (adjustment_impTask_points->get_value() > adjustment_impTask_points->get_upper()){
		adjustment_impTask_points->set_value(adjustment_impTask_points->get_upper());
	}else if (adjustment_impTask_points->get_value() < adjustment_impTask_points->get_lower()){
		adjustment_impTask_points->set_value(adjustment_impTask_points->get_lower());
	}
	
	adjustment_impTask_startFreq->set_upper(tmpAnalyser->getMaxFreq());
	adjustment_impTask_startFreq->set_lower(tmpAnalyser->getMinFreq());
	if (adjustment_impTask_startFreq->get_value() > adjustment_impTask_startFreq->get_upper()){
		adjustment_impTask_startFreq->set_value(adjustment_impTask_startFreq->get_upper());
	}else if (adjustment_impTask_startFreq->get_value() < adjustment_impTask_startFreq->get_lower()){
		adjustment_impTask_startFreq->set_value(adjustment_impTask_startFreq->get_lower());
	}
	
	adjustment_impTask_stopFreq->set_upper(tmpAnalyser->getMaxFreq());
	adjustment_impTask_stopFreq->set_lower(tmpAnalyser->getMinFreq());
	if (adjustment_impTask_stopFreq->get_value() > adjustment_impTask_stopFreq->get_upper()){
		adjustment_impTask_stopFreq->set_value(adjustment_impTask_stopFreq->get_upper());
	}else if (adjustment_impTask_stopFreq->get_value() < adjustment_impTask_stopFreq->get_lower()){
		adjustment_impTask_stopFreq->set_value(adjustment_impTask_stopFreq->get_lower());
	}
	
	adjustment_impTask_volt->set_upper(tmpAnalyser->getMaxVolt());
	adjustment_impTask_volt->set_lower(tmpAnalyser->getMinVolt());
	if (adjustment_impTask_volt->get_value() > adjustment_impTask_volt->get_upper()){
		adjustment_impTask_volt->set_value(adjustment_impTask_volt->get_upper());
	}else if (adjustment_impTask_volt->get_value() < adjustment_impTask_volt->get_lower()){
		adjustment_impTask_volt->set_value(adjustment_impTask_volt->get_lower());
	}
	
}
void GUI::on_buttonAddImpTask_clicked(){
	int startFrequency = (int) adjustment_impTask_startFreq->get_value();
	int stopFrequency = (int) adjustment_impTask_stopFreq->get_value();
	double voltage = (double) adjustment_impTask_volt->get_value();
	int points = (int) adjustment_impTask_points->get_value();
	unsigned short bw = (unsigned short) adjustment_impTask_bw->get_value();
	unsigned short pointAverage = (unsigned short) adjustment_impTask_pointAverage->get_value();
	ImpAnalyser::ImpAnalyser_ptr i;
	
	if (radiobutton_impTask_type_hp4294a->get_active() == true){
		HP4294A::HP4294A_ptr t = std::make_shared<HP4294A>(GPIB_HP4294A);
		t->setStartFrequency(startFrequency);
		t->setStopFrequency(stopFrequency);
		t->setVoltage(voltage);
		t->setPoints(points);
		t->setBw(bw);
		t->setPointAverage(pointAverage);
		i = t;
	}else if (radiobutton_impTask_type_novocontrol->get_active() == true){
		Novocontrol::Novocontrol_ptr t = std::make_shared<Novocontrol>(GPIB_NOVOCONTROL);
		Novocontrol::WIRE_MODE wire_mode = Novocontrol::WIRE_MODE::TWO_WIRE;
		
		
		if (radiobutton_impTask_wire_twoWire->get_active()){
			wire_mode = Novocontrol::WIRE_MODE::TWO_WIRE;
		}else if (radiobutton_impTask_wire_threeWire->get_active()){
			wire_mode = Novocontrol::WIRE_MODE::THREE_WIRE;
		}else if (radiobutton_impTask_wire_fourWire->get_active()){
			wire_mode = Novocontrol::WIRE_MODE::FOUR_WIRE;
		}
		t->setStartFrequency(startFrequency);
		t->setStopFrequency(stopFrequency);
		t->setVoltage(voltage);
		t->setPoints(points);
		t->setPointAverage(pointAverage);
		t->setWireMode(wire_mode);
		i = t;
	}else if (radiobutton_impTask_type_emstat->get_active() == true){
		EmStatPico::EmStatPico_ptr t = std::make_shared<EmStatPico>();
		ImpAnalyser::WIRE_MODE wire_mode = ImpAnalyser::WIRE_MODE::THREE_WIRE;
		
		
		if (radiobutton_impTask_wire_twoWire->get_active()){
			wire_mode = ImpAnalyser::WIRE_MODE::TWO_WIRE;
		}else if (radiobutton_impTask_wire_threeWire->get_active()){
			wire_mode = ImpAnalyser::WIRE_MODE::THREE_WIRE;
		}else if (radiobutton_impTask_wire_fourWire->get_active()){
			wire_mode = ImpAnalyser::WIRE_MODE::FOUR_WIRE;
		}
		t->setStartFrequency(startFrequency);
		t->setStopFrequency(stopFrequency);
		t->setVoltage(voltage);
		t->setPoints(points);
		t->setPointAverage(pointAverage);
		t->setWireMode(wire_mode);
		i = t;
	}
	if (i != 0){
		if (checkbutton_impTask_transient->get_active()){ // transient task
			TransImpTask::TERMINATION_MODE termMode = (radiobutton_impTask_transient_termPoints->get_active() ? TransImpTask::TERMINATION_MODE::TERM_CNT : TransImpTask::TERMINATION_MODE::TERM_TIME);
			int termValue = 1;
			
			if (termMode == TransImpTask::TERMINATION_MODE::TERM_CNT){
				termValue = (int) adjustment_impTask_transTermValue->get_value();
			}else if (termMode == TransImpTask::TERMINATION_MODE::TERM_TIME){
				int hours = adjustment_impTask_termHours->get_value();
				int minutes = adjustment_impTask_termMinutes->get_value();
				int seconds = adjustment_impTask_termSeconds->get_value();
				
				termValue = seconds + minutes * 60 + hours * 3600;
			}
			
			recTv2->addTask(std::make_shared<TransImpTask>(i, termValue, termMode));
		}else{ // non-transient task
			recTv2->addTask(std::make_shared<ImpAnalyserTask>(i));
		}
	}
}
void GUI::on_checkbutton_addImpTask_transient_toggled(){
	bool sensitive = checkbutton_impTask_transient->get_active();
	
	radiobutton_impTask_transient_termPoints->set_sensitive(sensitive);
	radiobutton_impTask_transient_termTime->set_sensitive(sensitive);
	scale_impTask_transient_termValue->set_sensitive(sensitive);
	label_impTask_transient_termValue->set_sensitive(sensitive);
	spinButton_impTask_transient_termValue->set_sensitive(sensitive);
	
	label_impTask_term_Second->set_sensitive(sensitive);
	label_impTask_term_Minute->set_sensitive(sensitive);
	label_impTask_term_Hour->set_sensitive(sensitive);
	spinButton_impTask_term_Second->set_sensitive(sensitive);
	spinButton_impTask_term_Minute->set_sensitive(sensitive);
	spinButton_impTask_term_Hour->set_sensitive(sensitive);
}

void GUI::on_checkbutton_addImpTask_termMode_toggled(){
	bool time = radiobutton_impTask_transient_termTime->get_active();
	
	label_impTask_term_Second->set_visible(time);
	label_impTask_term_Minute->set_visible(time);
	label_impTask_term_Hour->set_visible(time);
	spinButton_impTask_term_Second->set_visible(time);
	spinButton_impTask_term_Minute->set_visible(time);
	spinButton_impTask_term_Hour->set_visible(time);
	
	label_impTask_transient_termValue->set_visible(!time);
	scale_impTask_transient_termValue->set_visible(!time);
	spinButton_impTask_transient_termValue->set_visible(!time);
	
}

void GUI::on_buttonClearMyRecipe_clicked(){
	Gtk::MessageDialog dialog(*mainWindow, "clear recipe", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
	dialog.set_secondary_text("do you really want to clear the recipe?");
	if (dialog.run() == Gtk::RESPONSE_YES){
		recTv2->clear();
	}
}

void GUI::on_buttonSaveMyRecipe_clicked(){
	Recipe::Recipe_ptr r = recTv2->getAllTasksAsOneRecipe();
	
	if (r->getTasks().size() != 0){ // my recipe is not empty
		dialogSave_label_description->set_text("name of the recipe");
		dialogSave->set_transient_for(*mainWindow);
		
		if (dialogSave->run() == Gtk::RESPONSE_ACCEPT){
			std::string name = dialogSave_entry_name->get_buffer()->get_text();
			std::string path = FSHelper::composePath(pref.getFolderMyRecipes() , name + ".xml");
			
			
			if (!FSHelper::fileExists(path)){
				if (Recipe::recipeWithNameExists(name)){
					
					Gtk::MessageDialog dialog(*mainWindow, "error occured during recipe", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
					dialog.set_secondary_text("the recipe " + name + " already exists. Please choose another name");
					dialog.run();
					on_buttonSaveMyRecipe_clicked();
				}else{
					r->setName(name);
					r->save(path);
					log->add_event(Log_Event::create("saved recipe", path + " has been saved", Log_Event::TYPE::LOG_INFO));
				}
			}else{ // file already exists
				Gtk::MessageDialog dialog(*mainWindow, "recipe already exists", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
				dialog.set_secondary_text("the recipe " + name + " already exists in the folder '" + pref.getFolderMyRecipes() + "'. Do you want to overwrite it?");
				if (dialog.run() == Gtk::RESPONSE_YES){ // overwrite the recipe
					r->setName(name);
					r->save(path);
					log->add_event(Log_Event::create("saved recipe", path + " has been overwritten", Log_Event::TYPE::LOG_INFO));
				}else{ // do not overwrite the recipe -> reopen the dialog to enter a new name
					on_buttonSaveMyRecipe_clicked();
				}
			}
			
			if (r != nullptr){
				recTv1->addTask(r);
			}
		}
		dialogSave_entry_name->get_buffer()->set_text("");
	}else{ // my recipe is empty
		Gtk::MessageDialog dialog(*mainWindow, "recipe is empty", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
		dialog.set_secondary_text("nothing to save - add tasks to the recipe");
		dialog.run();
	}
}

void GUI::on_buttonGpibRead_clicked(){
	if (entry_gpib_slaveAddress->get_text_length() != 0){ //slave Address set
		int slaveAddress = std::stoi(entry_gpib_slaveAddress->get_text());
		
		if (gpib == nullptr){
			gpib = new GpibConnection(slaveAddress);
		}
		
		if (gpib->getSlaveAddress() != slaveAddress){
			delete gpib;
			gpib = new GpibConnection(slaveAddress);
		}
		
		try{
			std::cout << "trying to read the message..." << std::endl;
			std::string message = gpib->read();
			if (!message.empty()){
//				textView_gpib_message->get_buffer()->set_text(message);
				std::cout << "received message: " << message << std::endl;
			}else{
//				textView_gpib_message->get_buffer()->set_text("--nothing received--");
				std::cout << "received message: " << "--nothing received--" << std::endl;
			}
			textView_gpib_message->queue_draw();
		}catch(std::runtime_error err){
			std::cout << "error: " << err.what() << std::endl;
		}catch(std::exception e){
			std::cout << "errorx: " << e.what() << std::endl;
		}
	}
}
void GUI::on_buttonGpibSend_clicked(){
	if (entry_gpib_slaveAddress->get_text_length() != 0){ //slave Address set
		int slaveAddress = std::stoi(entry_gpib_slaveAddress->get_text());
		
		if (gpib == nullptr){
			gpib = new GpibConnection(slaveAddress);
		}
		
		if (gpib->getSlaveAddress() != slaveAddress){
			delete gpib;
			gpib = new GpibConnection(slaveAddress);
		}
		
		std::string message = entry_gpib_message->get_text();
		try{
			std::cout << "trying to send the message..." << std::endl;
			gpib->send(message);
		}catch(std::runtime_error err){
			std::cout << "error: " << err.what() << std::endl;
		}
	}
}

void GUI::on_buttonSelectProject_clicked(){
	std::string selected_Project_path = listView_projects->getSelectedProjectPath();
	std::string selected_Project_name = listView_projects->getSelectedProjectName();
	
	if(!selected_Project_path.empty()){ // something selected
		if (FSHelper::folderExists(selected_Project_path)){ //selected folder exists
			listView_experiments->scanFolder(selected_Project_path);
			label_experiment_selectedProject->set_text("recorded experiments of project " + selected_Project_name);
		}else{ //selected folder does not exist
			Gtk::MessageDialog dialog(*mainWindow, "selected project does not exist", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
			dialog.set_secondary_text("the project '" + selected_Project_path + "' does not exist");
			dialog.run();
		}
	}
}
void GUI::on_buttonSelectExperiment_clicked(){
	std::string selected_Experiment_path = listView_experiments->getSelectedExperimentPath();
	
	if(!selected_Experiment_path.empty()){ // something selected
		if (FSHelper::folderExists(selected_Experiment_path)){ //selected folder exists
			std::string pathMeasurementsFolder = FSHelper::composePath(selected_Experiment_path, "measurements");
			if (FSHelper::folderExists(pathMeasurementsFolder)){
				listView_spectrums->scanFolder(pathMeasurementsFolder);
			}else{
//				listView_spectrums->
			}
			
			label_spectrum_selectedExperiment->set_text("recorded spectrums of experiment " + selected_Experiment_path);
		}else{ //selected folder does not exist
			Gtk::MessageDialog dialog(*mainWindow, "selected experiment does not exist", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
			dialog.set_secondary_text("the experiment '" + selected_Experiment_path + "' does not exist");
			dialog.run();
		}
	}
	
}
void GUI::on_buttonSelectSpectrum_clicked(){
	ListView_SavedData::SPECTRUM_TYPE spectrumType = listView_spectrums->getSelectedSpectrumType();
	std::string path = listView_spectrums->getSelectedSpectrumPath();
	try{
		switch (spectrumType){
			case ListView_SavedData::SPECTRUM_TYPE::SPECTRUM:
				setSpectrum(loadSpectrum(path), path); //load spectrum
				notebook_page_exData->set_current_page(1); //switch to plot page
				break;
			
			case ListView_SavedData::SPECTRUM_TYPE::IMPEDANCE:
				setImpSpectrum(loadImpSpectrum(path), path); //load spectrum
				notebook_page_exData->set_current_page(2); //switch to plot page
				break;
				
			case ListView_SavedData::SPECTRUM_TYPE::TRANSIENT:
				setTransImpSpectrum(TransSpect::loadTransImpSpectrum(listView_spectrums->getSelectedTransSpectrumPaths()));
				break;
		}
	}catch (std::runtime_error &e){
		Gtk::MessageDialog dialog(*mainWindow, "error loading spectrum", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(e.what());
		dialog.run();
	} 
}
std::vector<DataP::DataP_ptr> GUI::loadSpectrum(std::string path){
	std::vector<DataP::DataP_ptr> spectrum;
	std::ifstream spectrum_file(path);
	
	if (spectrum_file.is_open()){
		std::string line;
		short wavelength = -1, intensity = -1;
		short i = 0;
		int lineNo = 1;
		if (std::getline(spectrum_file, line)){ // get header
			if (line.find_first_of("timediff") != std::string::npos){ // timediff=... line
				std::getline(spectrum_file, line); // read next line
			}
			
			// read header
			std::string element;
			while (!line.empty()){ // read header
				if (line.find_first_of(";") == std::string::npos){ // last element in row
					element = line;
					line = "";
				}else{ // not the last element in row
					element = line.substr(0, line.find_first_of(";"));
					line = line.substr(line.find_first_of(";")+1);
				}
				if (element.compare("wavelength") == 0){
					wavelength = i;
					i++;
				}else if (element.compare("intensity_real") == 0 || element.compare("intensity") == 0){
					intensity = i;
					i++;
				}
			}
			
			if (intensity == -1){ // no frequency information found
				throw std::runtime_error("no intensity_real found in file" + path);
			}
			
			//read data
			std::vector<std::string> elementsOfLine;
			while(std::getline(spectrum_file, line)){ // for the rest of the document
				lineNo++;
				elementsOfLine.clear();
				
				while (!line.empty()){ // fill elementsOfLineVector
					if (line.find_first_of(";") == std::string::npos){ // last element in row
						elementsOfLine.push_back(line);
						line = "";
					}else{ // not the last element in row
						elementsOfLine.push_back(line.substr(0, line.find_first_of(";")));
						line = line.substr(line.find_first_of(";")+1);
					}
				}
				if (elementsOfLine.size() != i){ // no of elements does not fit the header
					throw std::runtime_error("line " + std::to_string(lineNo) + " parsing error - no. of elements dismatch header - " + path);
				}
				
				DataP::DataP_ptr p = std::make_shared<DataP>(std::stod(elementsOfLine[wavelength]), std::stod(elementsOfLine[intensity]));
				spectrum.push_back(p);
			}
			spectrum_file.close();
		}
	}
	return spectrum;
}
std::vector<DataP::DataP_ptr> GUI::loadImpSpectrum(std::string path){
	std::vector<DataP::DataP_ptr> spectrum;
	
	std::ifstream spectrum_file(path);
	if (spectrum_file.is_open()){
		std::string line;
		short frequency = -1, impedance_real = -1, impedance_imag = -1, impedance_abs = -1, impedance_phase = -1;
		short i = 0;
		int lineNo = 1;
		
		//overread possible header of transient impedance files
		while (std::getline(spectrum_file, line) && (line.find("position") == 0 || line.find("timediff") == 0)){
			lineNo++;
		}
		
			
			if (!line.empty()){ // get header
			
			// read header
			std::string element;
			while (!line.empty()){ // read header
				if (line.find_first_of(";") == std::string::npos){ // last element in row
					element = line;
					line = "";
				}else{ // not the last element in row
					element = line.substr(0, line.find_first_of(";"));
					line = line.substr(line.find_first_of(";")+1);
				}
				if (element.compare("frequency") == 0){
					frequency = i;
					i++;
				}else if (element.compare("impedance_real") == 0){
					impedance_real = i;
					i++;
				}else if (element.compare("impedance_imag") == 0){
					impedance_imag = i;
					i++;
				}else if (element.compare("impedance_abs") == 0){
					impedance_abs = i;
					i++;
				}else if (element.compare("impedance_phase") == 0){
					impedance_phase = i;
					i++;
				}
			}
			
			if (frequency == -1){ // no frequency information found
				throw std::runtime_error("no freq information found in file" + path);
			}
			
			//read data
			std::vector<std::string> elementsOfLine;
			while(std::getline(spectrum_file, line)){ // for the rest of the document
				lineNo++;
				elementsOfLine.clear();
				
				while (!line.empty()){ // fill elementsOfLineVector
					if (line.find_first_of(";") == std::string::npos){ // last element in row
						elementsOfLine.push_back(line);
						line = "";
					}else{ // not the last element in row
						elementsOfLine.push_back(line.substr(0, line.find_first_of(";")));
						line = line.substr(line.find_first_of(";")+1);
					}
				}
				if (elementsOfLine.size() != i){ // no of elements does not fit the header
					throw std::runtime_error("line " + std::to_string(lineNo) + " parsing error - no. of elements dismatch header - " + path);
				}
				
				if ((impedance_real != -1) && (impedance_imag != -1)){ // got real and imag information
					DataP::DataP_ptr p = std::make_shared<DataP>(std::stod(elementsOfLine[frequency]), std::stod(elementsOfLine[impedance_real]), std::stod(elementsOfLine[impedance_imag]));
					spectrum.push_back(p);
				}else if ((impedance_phase != -1) && (impedance_abs != -1)){
					DataP::DataP_ptr p = std::make_shared<DataP>();
					p->setX(std::stod(elementsOfLine[frequency]));
					p->setY_AbsPhase(std::stod(elementsOfLine[impedance_abs]), std::stod(elementsOfLine[impedance_phase]), DataP::COMPLEX_MODE::COMPLEX_PHASE_RAD);
					spectrum.push_back(p);
				}else { ///@todo add other possible combinations
					throw std::runtime_error("data combination of real / imag / abs / phase not implemented");
				}
			}
			spectrum_file.close();
		}
	}
	return spectrum;
}


void GUI::dialogSave_on_buttonSave_clicked(){
	std::cout << "save" << std::endl;
	dialogSave->response(Gtk::RESPONSE_ACCEPT);
	dialogSave->signal_response();
	dialogSave->hide();
	
}
void GUI::dialogSave_on_buttonCancel_clicked(){
	std::cout << "cancel" << std::endl;
	dialogSave->response(Gtk::RESPONSE_CANCEL);
	dialogSave->signal_response();
	dialogSave->hide();
}

void GUI::run(){
	app->run(*mainWindow);
}


void GUI::addLogEvent(Log_Event::Log_Event_ptr event){
	
	eventsToAddMtx.lock();
	eventsToAdd.push_back(event);
	eventsToAddMtx.unlock();
	
	logEventDispatcher.emit();
}
void GUI::logEventDispatcherFunction(){
	Glib::ustring text = textViewLog->get_buffer()->get_text();
	
	eventsToAddMtx.lock();
	for (std::vector<Log_Event::Log_Event_ptr>::const_iterator cit = eventsToAdd.cbegin(); cit != eventsToAdd.cend(); cit++){
		text += (*cit)->toString();
		text += "\n";
	}
	eventsToAdd.clear();
	eventsToAddMtx.unlock();
	
	textViewLog->get_buffer()->set_text(text);
	textViewLog->queue_draw();
}


void GUI::setImpSpectrum(std::vector<DataP::DataP_ptr> s, std::string title){
	std::vector<DataP::DataP_ptr> nyquist_data;
	
	for (std::vector<DataP::DataP_ptr>::const_iterator cit = s.cbegin(); cit != s.cend(); cit++){
		DataP::DataP_ptr p = *cit;
		DataP::DataP_ptr dataPoint = std::make_shared<DataP>(p->getY(DataP::COMPLEX_MODE::COMPLEX_REAL), (-1.0) * p->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG));
		nyquist_data.push_back(dataPoint);
	}
	
	plotWindowImpedance_Abs->autoMax_y = true;
	plotWindowImpedance_Abs->autoMax_x = true;
	plotWindowImpedance_Abs->autoScl_x = true;
	plotWindowImpedance_Abs->autoScl_y = true;
	plotWindowImpedance_Abs->autoPadding_x = true;
	plotWindowImpedance_Abs->autoPadding_y = true;
	plotWindowImpedance_Abs->showGrid = true;
	plotWindowImpedance_Abs->scale_x = PlotWindow::SCALE::LOG;
	plotWindowImpedance_Abs->scale_y = PlotWindow::SCALE::LOG;
	plotWindowImpedance_Abs->setComplexMode(DataP::COMPLEX_ABS);
	plotWindowImpedance_Abs->x_axis_label = "frequency [Hz]";
	plotWindowImpedance_Abs->y_axis_label = "abs [OHM]";
	
	plotWindowImpedance_Phase->autoMax_x = true;
	plotWindowImpedance_Phase->autoScl_x = true;
	plotWindowImpedance_Phase->autoScl_y = true;
	plotWindowImpedance_Phase->autoMax_y = false;
	plotWindowImpedance_Phase->y_min = -180;
	plotWindowImpedance_Phase->y_max = 180;
	plotWindowImpedance_Phase->autoPadding_x = true;
	plotWindowImpedance_Phase->autoPadding_y = true;
	plotWindowImpedance_Phase->showGrid = true;
	plotWindowImpedance_Phase->showZeroAxis = true;
	plotWindowImpedance_Phase->scale_x = PlotWindow::SCALE::LOG;
	plotWindowImpedance_Phase->scale_y = PlotWindow::SCALE::LINEAR;
	plotWindowImpedance_Phase->setComplexMode(DataP::COMPLEX_PHASE_DEG);
	plotWindowImpedance_Phase->x_axis_label = "frequency [Hz]";
	plotWindowImpedance_Phase->y_axis_label = "phase [DEG]";
	
	
	plotWindowImpedance_Nyquist->autoMax_x = true;
	plotWindowImpedance_Nyquist->autoScl_x = true;
	plotWindowImpedance_Nyquist->autoScl_y = true;
	plotWindowImpedance_Nyquist->autoMax_y = true;
	plotWindowImpedance_Nyquist->autoPadding_x = true;
	plotWindowImpedance_Nyquist->autoPadding_y = true;
	plotWindowImpedance_Nyquist->showGrid = true;
	plotWindowImpedance_Nyquist->showZeroAxis = true;
	plotWindowImpedance_Nyquist->scale_x = PlotWindow::SCALE::LINEAR;
	plotWindowImpedance_Nyquist->scale_y = PlotWindow::SCALE::LINEAR;
	plotWindowImpedance_Nyquist->setComplexMode(DataP::COMPLEX_REAL);
	plotWindowImpedance_Nyquist->x_axis_label = "real part [OHM]";
	plotWindowImpedance_Nyquist->y_axis_label = "neg. imag. part [OHM]";
	
	plotWindowImpedance_Abs->setData(s);
	plotWindowImpedance_Phase->setData(s);
	plotWindowImpedance_Nyquist->setData(nyquist_data);
	label_impedance_plot_title->set_text(title);
	label_impedance_plot_nyquist_title->set_text(title);
}
void GUI::setSpectrum(std::vector<DataP::DataP_ptr> s, std::string title){
	plotWindowSpectrum->autoMax_y = true;
	plotWindowSpectrum->autoMax_x = true;
	plotWindowSpectrum->autoScl_x = true;
	plotWindowSpectrum->autoScl_y = true;
	plotWindowSpectrum->showGrid = true;
	plotWindowSpectrum->scale_x = PlotWindow::SCALE::LINEAR;
	plotWindowSpectrum->scale_y = PlotWindow::SCALE::LINEAR;
	plotWindowSpectrum->x_axis_label = "wavelength [nm]";
	plotWindowSpectrum->y_axis_label = "intensity";
	
	plotWindowSpectrum->setData(s);
	label_spectrum_plot_title->set_text(title);
}
void GUI::setTransImpSpectrum(TransSpect::TransSpect_ptr p, std::string path){
	transientViewHandler.onTransImpSpecAdded(p, p->getLastSpectrum(), p->getSpectrumCount(), p->getTimeDiff(p->getSpectrumCount()), path);
}
bool GUI::on_window_close(GdkEventAny* event){
	std::cout << "closing... " << std::endl;
	return false;
} 

void GUI::on_button_voltContr_refresh_clicked(){
	I2CVoltageTask::I2CVoltageTask_ptr voltageTask = I2CVoltageTask::create();
	if (voltageTask->isConnected()){
		checkbutton_voltageContr_connected->set_active(true);
		double voltage = voltageTask->readVoltage();
		
		label_voltContr_dcycle->set_text(std::to_string((int)(voltageTask->readCurrentDutyCycle()* 100.0)).append("%"));
		label_voltContr_voltage->set_text(FSHelper::formatDouble(voltage).append("V"));
		label_voltContr_svoltage->set_text(FSHelper::formatDouble(voltageTask->readSetpointVoltage()).append("V"));
		label_voltage_controller->set_text(std::string("V = ").append(FSHelper::formatDouble(voltage).append("V")));
		label_voltContr_freq->set_text(std::to_string(voltageTask->readFrequency()).append("Hz"));
		
		switch (voltageTask->readCurrentMode()){
			case I2CVoltageTask::VOLTAGE_MODE::MODE_CONTROLLER:
				label_voltContr_mode->set_text("voltage controller");
				break;
				
			case I2CVoltageTask::VOLTAGE_MODE::MODE_DUTY_CYCLE:
				label_voltContr_mode->set_text("duty cycle");
				break;
				
			default:
				label_voltContr_mode->set_text("error");
				break;
		}
	}else{
		checkbutton_voltageContr_connected->set_active(false);
		label_voltContr_dcycle->set_text("-");
		label_voltContr_voltage->set_text("-");
		label_voltContr_svoltage->set_text("-");
		label_voltage_controller->set_text("");
		label_voltContr_mode->set_text("-");
		label_voltContr_freq->set_text("-");
	}
	
	Uc_Connection::Uc_Connection_ptr atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	if (atmega32->isConnected()){
		label_voltContr_extvoltage->set_text(FSHelper::formatDouble(voltageTask->readExternalVoltage()).append("V"));
	}else{
		label_voltContr_extvoltage->set_text("-");
	}
}
void GUI::on_button_freqGen_refresh_clicked(){
	I2CFreqTask::I2CFreqTask_ptr freqTask = I2CFreqTask::create();
	if (freqTask->isConnected()){
		checkbutton_freqGen_connected->set_active(true);
		double freq = freqTask->readFreq();
		
		label_freqGen_freq->set_text(FSHelper::formatDouble(freq).append("Hz"));
	}else{
		checkbutton_freqGen_connected->set_active(false);
		label_freqGen_freq->set_text("-");
	}
}
void GUI::on_button_overview_general_refresh_clicked(){
	on_button_freqGen_refresh_clicked();
	on_button_voltContr_refresh_clicked();
	
	Uc_Connection::Uc_Connection_ptr atmega32 = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	checkbutton_atmega32_connected->set_active(atmega32->isConnected());
	
	Spectrometer::Spectrometer_ptr s = Spectrometer::create();
	checkbutton_spectrometer_connected->set_active(s->isConnected());
	
	GpibConnection novo(GPIB_NOVOCONTROL);
	checkbutton_novocontrol_connected->set_active(novo.isConnected());
	
	GpibConnection hp(GPIB_HP4294A);
	checkbutton_hp429a_connected->set_active(hp.isConnected());
	
}
void GUI::on_button_read_realis_clicked(){
	try{
		relais->readRelais();
		
		checkbutton_relais_ac->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_AC));
		checkbutton_relais_boost_in->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_BOOST_IN));
		checkbutton_relais_hv_ext->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_HV_EXT));
		checkbutton_relais_safety->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_SAFETY));
		checkbutton_relais_ac_2->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_AC_2));
		checkbutton_relais_imp_ewod->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_IMP_EWOD));
		checkbutton_relais_we_p_wt_s->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S));
		checkbutton_relais_re_p_ce->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_RE_P_CE));
		checkbutton_relais_ext_pico->set_active(relais->getRelais(Relais::RELAIS::R_STEUER_EXT_PICO));
	}catch (std::runtime_error &e){
		Gtk::MessageDialog dialog(*mainWindow, "error writing relais", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(e.what());
		dialog.run();
	} 
}

void GUI::on_button_write_realis_clicked(){
	relais->setRelais(Relais::RELAIS::R_STEUER_AC, checkbutton_relais_ac->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_BOOST_IN, checkbutton_relais_boost_in->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_HV_EXT, checkbutton_relais_hv_ext->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_SAFETY, checkbutton_relais_safety->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_AC_2, checkbutton_relais_ac_2->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_IMP_EWOD, checkbutton_relais_imp_ewod->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_WE_P_WE_S, checkbutton_relais_we_p_wt_s->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_RE_P_CE, checkbutton_relais_re_p_ce->get_active());
	relais->setRelais(Relais::RELAIS::R_STEUER_EXT_PICO, checkbutton_relais_ext_pico->get_active());
	
	try{
		relais->writeRelais();
	}catch (std::runtime_error &e){
		Gtk::MessageDialog dialog(*mainWindow, "error writing relais", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text(e.what());
		dialog.run();
	} 
}

void GUI::on_button_page_pref_save_clicked(){
	if (FSHelper::folderExists(entry_page_pref_recipeFolder->get_text())){
		pref.setFolderMyRecipes(entry_page_pref_recipeFolder->get_text());
	}else{
		Gtk::MessageDialog dialog(*mainWindow, "folder does not exist", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text("The folder '" + entry_page_pref_recipeFolder->get_text() + "' does not exist");
		dialog.run();
	}
	
	if (FSHelper::folderExists(entry_page_pref_projectFolder->get_text())){
		pref.setFolderProjects(entry_page_pref_projectFolder->get_text());
	}else{
		Gtk::MessageDialog dialog(*mainWindow, "folder does not exist", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
		dialog.set_secondary_text("The folder '" + entry_page_pref_projectFolder->get_text() + "' does not exist");
		dialog.run();
	}
	
	pref.saveToFile(FSHelper::composePath(PREF_FILE_FOLDER, PREF_FILE_NAME));
}
void GUI::on_button_page_pref_restore_clicked(){
	entry_page_pref_recipeFolder->set_text(pref.getFolderMyRecipes());
	entry_page_pref_projectFolder->set_text(pref.getFolderProjects());
}
void GUI::on_button_page_pref_recipeFolder_browse_clicked(){
	Gtk::FileChooserDialog dialog("Please choose a folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	dialog.set_transient_for(*mainWindow);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);
	if (FSHelper::folderExists(entry_page_pref_recipeFolder->get_text())){
		dialog.set_current_folder(entry_page_pref_recipeFolder->get_text());
	}
	
	int result = dialog.run();
	
	switch(result){
	case Gtk::RESPONSE_OK:
		entry_page_pref_recipeFolder->set_text(dialog.get_filename());
		break;
	case Gtk::RESPONSE_CANCEL:
		break;
	}
}
void GUI::on_button_page_pref_projectFolder_browse_clicked(){
	Gtk::FileChooserDialog dialog("Please choose a folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	dialog.set_transient_for(*mainWindow);
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);
	if (FSHelper::folderExists(entry_page_pref_projectFolder->get_text())){
		dialog.set_current_folder(entry_page_pref_projectFolder->get_text());
	}
	
	int result = dialog.run();
	
	switch(result){
	case Gtk::RESPONSE_OK:
		entry_page_pref_projectFolder->set_text(dialog.get_filename());
		break;
	case Gtk::RESPONSE_CANCEL:
		break;
	}
}

PadTask::PadTask_ptr GUI::getPadTaskFromGUI(){
	std::vector<int> pads;
	for (int i = TOGGLEBUTTON_PADNO_FIRST; i <= TOGGLEBUTTON_PADNO_LAST; i++){
		if (toggleButtonPads[i-TOGGLEBUTTON_PADNO_FIRST]->get_active()){
			pads.push_back(i);
		}
	}
	double duration = adjustment_PadTask_time->get_value();
	
	return PadTask::create(pads, duration);
}

void GUI::updateThread(){
	Uc_Connection::Uc_Connection_ptr atmega = Uc_Connection::create(I2C_ATMEGA32_SLAVE_ADDRESS);
	
	switch (notebook_main->get_current_page()){
		case 0:{ //overview
			switch(notebook_overview->get_current_page()){
				case 0: //general
					on_button_overview_general_refresh_clicked();
					break;
					
				case 1: //voltage controller
					on_button_voltContr_refresh_clicked();
					break;
					
				case 2: //freq generator
					on_button_freqGen_refresh_clicked();
					break;
			}
			break;
		}
	}
	
}
void GUI::on_button_autorefresh_toggled(){
	if (button_autorefresh->get_active()){
		updateTimer.start();
	}else{
		updateTimer.stop();
	}
}

void GUI::on_button_shutdown_clicked(){
	Gtk::MessageDialog dialog(*mainWindow, "Do you really want to shutdown?", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
	int result = dialog.run();
	
	if (result == GTK_RESPONSE_YES){
		system("sudo shutdown -P now");
		mainWindow->close();
	}
}

void GUI::on_button_fullscreen_toggled(){
	if (button_fullscreen->get_active()){
		mainWindow->fullscreen();
	}else{
		mainWindow->unfullscreen();
	}
}

void GUI::on_button_setPwmFreq_clicked(){
	Uc_Connection::Uc_Connection_ptr attinyController = Uc_Connection::create(I2C_ATTINY45_VOLT_SLAVE_ADDRESS);
	int freq = adjustment_pwmFreq->get_value();
	
	char freq_0 =  (0x000000FF & freq);
	char freq_1 = ((0x0000FF00 & freq) >> 8);
	char freq_2 = ((0x00FF0000 & freq) >> 16);
	char freq_3 = ((0xFF000000 & freq) >> 24);
	
	bool error = false;
	if (!error) error = (attinyController->writeRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ0, freq_0) != false);
	if (!error) error = (attinyController->writeRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ1, freq_1) != false);
	if (!error) error = (attinyController->writeRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ2, freq_2) != false);
	if (!error) error = (attinyController->writeRegister(I2C_ATTINY45_VOLT_BUFFER_FREQ3, freq_3) != false);
	
	if (!error){
		std::cout << "set pwm freq to " << freq << " Hz" << std::endl;
	}else{
		std::cout << "unable to set pwm freq to " << freq << " Hz" << std::endl;
	}
	
}
