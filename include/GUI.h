#pragma once

/**
 * @file GUI.h
 * 
 * @date 23.04.2019
 * @author Nils Bosbach
 * @class GUI
 */
#include "PlotWindow.h"
#include "TreeView_Recipe.h"
#include "Log_Event.h"
#include "Logbook.h"
#include "Preferences.h"
#include "Spectrometer.h"
#include "ExperimentData.h"
#include "GpibConnection.h"
#include "ListView_Project.h"
#include "ListView_Experiment.h"
#include "ListView_SavedData.h"
#include "ListView_Freq.h"
#include "TransientGUIHandler.h"
#include "CameraWindow.h"
#include "Camera_cv.h"
#include "StatusLed.h"
#include "TransSpect.h"
#include "Relais.h"
#include "PadTask.h"
#include "Timer.h"

#include <stdexcept>
#include <gtkmm.h> 

#define TOGGLEBUTTON_PADNO_FIRST 1
#define TOGGLEBUTTON_PADNO_LAST 53
 
class GUI{
public:
	GUI(Logbook::Logbook_ptr l);
	virtual ~GUI();
	
	/**
	 * @brief starts the main window
	 */
	void run();
	void addLogEvent(Log_Event::Log_Event_ptr event);
	
private:
	Preferences pref;
	Logbook::Logbook_ptr log;
	Camera_cv camera;
	
	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::Builder> refBuilder;
	Gtk::Window *mainWindow;
	TreeView_Recipe *recTv1;
	TreeView_Recipe *recTv2;
	PlotWindow *plotWindowSpectrum;
	PlotWindow *plotWindowImpedance_Abs;
	PlotWindow *plotWindowImpedance_Phase;
	PlotWindow *plotWindowImpedance_Nyquist;
	ListView_Project *listView_projects;
	ListView_Experiment *listView_experiments;
	ListView_SavedData *listView_spectrums;
	Gtk::TextView *textViewLog;
	CameraWindow *image_preview;
	
	
	Gtk::Button *button_addToMyRecipe;
	Gtk::Button *button_duplicateTask;
	Gtk::Button *button_removeFromMyRecipe;
	Gtk::ToolButton *button_runMyRecipe;
	Gtk::ToolButton *button_stopMyRecipe;
	Gtk::ToolButton *button_saveProject;
	Gtk::ToolButton *button_shutdown;
	Gtk::ToggleToolButton *button_fullscreen;
	Gtk::ToggleToolButton *button_record;
	Gtk::ToggleToolButton *button_autorefresh;
	Gtk::Button *button_saveMyRecipe;
	Gtk::Button *button_setFrequency;
	Gtk::Button *button_setVoltage;
	Gtk::Button *button_setDutyCycle;
	Gtk::Button *button_setVoltPwmFreq;
	Gtk::CheckButton *checkbutton_waitForVoltage;
	Gtk::CheckButton *checkbutton_preview;
	Gtk::TreeView *treeView_allRecipes;
	Gtk::TreeView *treeView_myRecipe;
	Gtk::Dialog *dialogSave;
	Gtk::Button *dialogSave_button_save;
	Gtk::Button *dialogSave_button_cancel;
	Gtk::Entry *dialogSave_entry_name;
	Gtk::Label *dialogSave_label_description;
	Gtk::Label *label_experiment_selectedProject;
	Gtk::Label *label_spectrum_selectedExperiment;
	Gtk::Label *label_impedance_plot_title;
	Gtk::Label *label_impedance_plot_nyquist_title;
	Gtk::Label *label_spectrum_plot_title;
	Gtk::Button *button_addSpectrometerTask;
	Gtk::Button *button_addPadTask;
	Gtk::Button *button_addHP4294ATask;
	Gtk::Button *button_addVoltageTask;
	Gtk::Button *button_addFreqTask;
	Gtk::Button *button_clearMyRecipe;
	Gtk::ToggleButton * toggleButtonPads[TOGGLEBUTTON_PADNO_LAST - TOGGLEBUTTON_PADNO_FIRST + 1];
	Gtk::Entry *entry_gpib_slaveAddress;
	Gtk::Entry *entry_gpib_message;
	Gtk::Button *button_gpib_send;
	Gtk::Button *button_gpib_read;
	Gtk::Button *button_selectProject;
	Gtk::Button *button_selectExperiment;
	Gtk::Button *button_selectSpectrum;
	Gtk::TextView *textView_gpib_message;
	Gtk::Notebook *notebook_page_exData;
	Gtk::Notebook *notebook_overview;
	Gtk::Notebook *notebook_main;
	Gtk::CheckButton *checkbutton_impTask_transient;
	Gtk::RadioButton *radiobutton_impTask_transient_termTime;
	Gtk::RadioButton *radiobutton_impTask_transient_termPoints;
	Gtk::RadioButton *radiobutton_impTask_type_hp4294a;
	Gtk::RadioButton *radiobutton_impTask_type_novocontrol;
	Gtk::RadioButton *radiobutton_impTask_type_emstat;
	Gtk::RadioButton *radiobutton_impTask_wire_twoWire;
	Gtk::RadioButton *radiobutton_impTask_wire_threeWire;
	Gtk::RadioButton *radiobutton_impTask_wire_fourWire;
	Gtk::SpinButton *spinButton_impTask_term_Hour;
	Gtk::SpinButton *spinButton_impTask_term_Minute;
	Gtk::SpinButton *spinButton_impTask_term_Second;
	Gtk::Label *label_impTask_term_Hour;
	Gtk::Label *label_impTask_term_Minute;
	Gtk::Label *label_impTask_term_Second;
	Gtk::Box *box_impTask_wire;
	Gtk::Label *label_impTask_wire;
	Gtk::Label *label_impTask_bw;
	Gtk::Scale *scale_impTask_bw;
	Gtk::Scale *scale_impTask_transient_termValue;
	Gtk::SpinButton *spinButton_impTask_transient_termValue;
	Gtk::Label *label_impTask_transient_termValue;
	Gtk::Button *button_addDelayTask;
	Gtk::Button *button_activatePad;
	Gtk::RadioButton *radioButton_page_addTask_page_voltage_internal;
	Gtk::RadioButton *radioButton_page_addTask_page_voltage_external;
	
	Gtk::Button *button_voltContr_refresh;
	Gtk::Label *label_voltage_controller;
	Gtk::Label *label_frequency_generator;
	Gtk::Label *label_voltContr_voltage;
	Gtk::Label *label_voltContr_extvoltage;
	Gtk::Label *label_voltContr_svoltage;
	Gtk::Label *label_voltContr_dcycle;
	Gtk::Label *label_voltContr_mode;
	Gtk::Label *label_voltContr_freq;
	Gtk::Label *label_freqGen_freq;
	Gtk::Button *button_freqGen_refresh;
	Gtk::Button *button_overview_general_refresh;
	Gtk::CheckButton *checkbutton_voltageContr_connected;
	Gtk::CheckButton *checkbutton_freqGen_connected;
	Gtk::CheckButton *checkbutton_novocontrol_connected;
	Gtk::CheckButton *checkbutton_hp429a_connected;
	Gtk::CheckButton *checkbutton_spectrometer_connected;
	Gtk::CheckButton *checkbutton_atmega32_connected;
	
	
	Gtk::CheckButton *checkbutton_relais_boost_in;
	Gtk::CheckButton *checkbutton_relais_hv_ext;
	Gtk::CheckButton *checkbutton_relais_safety;
	Gtk::CheckButton *checkbutton_relais_ac;
	Gtk::CheckButton *checkbutton_relais_ac_2;
	Gtk::CheckButton *checkbutton_relais_imp_ewod;
	Gtk::CheckButton *checkbutton_relais_we_p_wt_s;
	Gtk::CheckButton *checkbutton_relais_re_p_ce;
	Gtk::CheckButton *checkbutton_relais_ext_pico;
	Gtk::Button *button_relais_read;
	Gtk::Button *button_relais_write;
	
	Gtk::Button *button_page_pref_restore;
	Gtk::Button *button_page_pref_save;
	Gtk::Button *button_page_pref_recipeFolder_browse;
	Gtk::Button *button_page_pref_projectFolder_browse;
	Gtk::Entry  *entry_page_pref_projectFolder;
	Gtk::Entry  *entry_page_pref_recipeFolder;
	
	
	
	Glib::RefPtr<Gtk::Adjustment> adjustment_frequency;
	Glib::RefPtr<Gtk::Adjustment> adjustment_voltage;
	Glib::RefPtr<Gtk::Adjustment> adjustment_dutyCycle;
	Glib::RefPtr<Gtk::Adjustment> adjustment_pwmFreq;
	Glib::RefPtr<Gtk::Adjustment> adjustment_PadTask_time;
	Glib::RefPtr<Gtk::Adjustment> adjustment_SpectrometerTask_integrationTime;
	Glib::RefPtr<Gtk::Adjustment> adjustment_SpectrometerTask_scansToAverage;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_startFreq;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_stopFreq;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_volt;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_points;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_bw;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_pointAverage;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_termHours;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_termMinutes;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_termSeconds;
	Glib::RefPtr<Gtk::Adjustment> adjustment_impTask_transTermValue;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_voltage_voltage;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_freq_freq;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_delay_hours;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_delay_minutes;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_delay_seconds;
	Glib::RefPtr<Gtk::Adjustment> adjustment_page_addTask_page_delay_milliseconds;
	
	Spectrometer spectrometer;
	GpibConnection *gpib;
	StatusLed::StatusLed_ptr status_leds;
	Relais::Relais_ptr relais;
	
	TransientGUIHandler transientViewHandler;
	
	//manual mode
	Gtk::Entry *entryPads;
	Gtk::Button *buttonPadsExecute;
	
	Glib::Dispatcher logEventDispatcher;
	
	static void onTransImpSpecAdded (TransSpect::TransSpect_ptr p, TransSpect::Spectrum s, std::string path);
	
	std::vector<Log_Event::Log_Event_ptr> eventsToAdd;
	std::mutex eventsToAddMtx;
	
	Timer updateTimer;
	
	PadTask::PadTask_ptr getPadTaskFromGUI();
	
	void on_buttonExecutePadTask_clicked();
	void on_buttonAddToMyRecipe_clicked();
	void on_buttonDuplicateTask_clicked();
	void on_buttonRemoveFromMyRecipe_clicked();
	void on_buttonRunRecipe_clicked();
	void on_buttonStopRecipe_clicked();
	void on_buttonSaveMyRecipe_clicked();
	void on_buttonSaveProject_clicked();
	void on_buttonAddSpectrometerTask_clicked();
	void on_buttonAddDelayTask_clicked();
	void on_buttonAddPadTask_clicked();
	void on_buttonAddImpTask_clicked();
	void on_buttonAddVoltageTask_clicked();
	void on_buttonAddFreqTask_clicked();
	void on_buttonSetFrequency_clicked();
	void on_buttonSetVoltage_clicked();
	void on_buttonSetDutyCycle_clicked();
	void on_buttonGpibRead_clicked();
	void on_buttonGpibSend_clicked();
	void on_buttonClearMyRecipe_clicked();
	void on_buttonSelectProject_clicked();
	void on_buttonSelectExperiment_clicked();
	void on_buttonSelectSpectrum_clicked();
	void on_checkbuttonPreview_toggled();
	void dialogSave_on_buttonSave_clicked();
	void dialogSave_on_buttonCancel_clicked();
	void on_checkbutton_addImpTask_transient_toggled();
	bool on_window_close(GdkEventAny* event);
	void on_button_voltContr_refresh_clicked();
	void on_button_freqGen_refresh_clicked();
	void on_button_overview_general_refresh_clicked();
	void on_addImpTask_radiobutton_changed();
	void on_button_read_realis_clicked();
	void on_button_write_realis_clicked();
	void on_button_page_pref_save_clicked();
	void on_button_page_pref_restore_clicked();
	void on_button_page_pref_recipeFolder_browse_clicked();
	void on_button_page_pref_projectFolder_browse_clicked();
	void on_buttonActivatePad_clicked();
	void on_button_autorefresh_toggled();
	void on_button_shutdown_clicked();
	void on_button_fullscreen_toggled();
	void on_button_setPwmFreq_clicked();
	void on_checkbutton_addImpTask_termMode_toggled();
	
	void init_loadPreferences();
	void init_loadCSSFile();
	void init_loadRecipes();
	void init_grabWidgetsFromBuilder();
	void init_connectSignals();
	void init_variables();
	void init_setRelais();
	
	void startPreview();
	void stopPreview();
	
	void updateThread();
	
	void setImpSpectrum(std::vector<DataP::DataP_ptr> s, std::string title = "");
	void setSpectrum(std::vector<DataP::DataP_ptr> s, std::string title = "");
	void setTransImpSpectrum(TransSpect::TransSpect_ptr p, std::string path = "");
	static std::vector<DataP::DataP_ptr> loadSpectrum(std::string path);
	static std::vector<DataP::DataP_ptr> loadImpSpectrum(std::string path);
	
	
	static void *executemyrecipe(void *recipes);
	static void *runPreview(void *d);
	
public:
	class ExecuteMyRecipeThreadData{
	public:
		ExecuteMyRecipeThreadData(){
//			dispatcher();
			executeNext = new bool(true);
			finished = false;
			error = nullptr;
		}
		~ExecuteMyRecipeThreadData(){
			
		}
		Recipe::Recipe_ptr recipe;
		ExperimentData::ExperimentData_ptr pData;
		bool *executeNext;
		bool finished;
		Glib::Dispatcher dispatcher;
		std::runtime_error* error;
	};
	void on_finished_executing_myRecipe();
	pthread_t thread_execute_MyRecipe;
	std::mutex on_finished_executing_myRecipeMtx;
	ExecuteMyRecipeThreadData *thread_execute_MyRecipe_data;
	
	
	void on_preview_captured(Camera_cv::Image_ptr image);
	
	void logEventDispatcherFunction();
};
