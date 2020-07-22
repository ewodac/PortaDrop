#pragma once
/**
 * @file PlotWindow.h
 * 
 * @class PlotWindow
 * @author Nils Bosbah
 * @date 17.04.2019
 * @brief derivates from Gtk::DrawingArea and implements a plotting window.
 */
#include "DataP.h"
 
#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>
#include <algorithm>
#include <vector>
#include <mutex>

class PlotWindow : public Gtk::DrawingArea{

public:
	enum SCALE {LINEAR, LOG};
	
	/// @todo move attributes to private, add getter / setter, and force redraw after changing the values
	
	/// show the grid (x and y axis)
	bool showGrid = true;
	
	/// show additional axis if ((x_min < 0) && (x_max > 0)) resp. ((y_min < 0) && (y_max > 0))
	bool showZeroAxis = true;
	
	/// show little strokes near the numbers on the axes
	bool showAxisStrokes = true;
	
	/// automatically calculate scale dimensions of x_axis
	bool autoScl_x = true;
	
	/// automatically calculate scale dimensions of y_axis
	bool autoScl_y = true;
	
	/// automatically calculate x_min, x_max should be false if min / max values are set manually
	bool autoMax_x = true;
	
	/// automatically calculate y_min, y_max; should be false if min / max values are set manually
	bool autoMax_y = true;
	
	/// add a padding to the min / max value of the x_axis to get better values - autoMax_x needs to be true
	bool autoPadding_x = true;
	
	/// add a padding to the min / max value of the y_axis to get better values - autoMax_y needs to be true
	bool autoPadding_y = true;
	
	/// scale of the x axis (linear or logarithmic)
	SCALE scale_x = LINEAR;
	
	/// scale of the y axis (linear or logarithmic)
	SCALE scale_y = LOG;
	
	/// distance beween strokes / numbers on the axes
	double x_scl, y_scl;
	
	/// dimensions of the plot
	double x_max, x_min, y_max, y_min; 
	
	std::string x_axis_label = "";
	
	std::string y_axis_label = "";
	
	PlotWindow();
	PlotWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~PlotWindow();
	
	/**
	 * @brief set the whole data
	 * sets the data which should be plotted in the view. If autoMax is true, min / max values are calculated
	 * and set. If autoScl is true, scl values are calculated and set. The plot will be redrawn
	 * @param data the data points
	 */
	void setData(std::vector<DataP::DataP_ptr> data);
	
	/**
	 * @brief adds one data point to the existing ones
	 * adds one data point, which will be plotted in the view. If autoMax is true, min / max values are calculated
	 * and set. If autoScl is true, scl values are calculated and set. The plot will be redrawn
	 * @param data the data point which should be added
	 */
	void addData(DataP::DataP_ptr data);
	
	/**
	 * @brief set the component of the complex y coordinate, that should be displayed
	 * @param c component of the complex y coordinate
	 */
	void setComplexMode(DataP::COMPLEX_MODE c);
	
	
	double getXminData() const;
	double getXmaxData() const;
	double getYminData() const;
	double getYmaxData() const;
	double getYaverage();
	
protected:
	/**
	 * @brief implements the drawing of the axes and data
	 * This method will be called by gtk everytime the view needs to be redrawn (change of the widget size,
	 * manually redraw requested (e.g. because new data has been set), ...). It implements the drawing of the
	 * axis and plotting the data.
	 * @todo change some preprocessor constants to attributes (e.g. distances) and adapt them dynamically
	 */
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override; //override signal handler to draw on DrawingArea

private:
	enum ALIGN {LEFT, CENTER, RIGHT}; // aligment of a text element 
	std::vector<DataP::DataP_ptr> plot_data; // stores the data (which will be plottet)
	std::mutex dataMtx; //synchronizes the access on plot_data
	
	void init();
	
	/// used to draw a text on the DrawingArea
	void draw_text(const Cairo::RefPtr<Cairo::Context>& cr, Glib::ustring text, int x, int y, ALIGN a, double rotation_rad = 0);
	
	/// calculates dynamically the distance between two strokes on the axis (in dependence of the range of the data)
	double getScl_x();
	double getScl_y();
	static double getScl(double widget_dim, double axis_dim);
	
	/// updates x_max, x_min, x_scl, y_max, y_min, y_scl depending on plot_data
	void updateWindowSizes();
	
	inline void calculateDataRage();
	
	static int getMaxLabelWidth(int startVal, int stopVal, double step);
	
	double x_min_data, x_max_data, y_min_data, y_max_data;
	
	DataP::COMPLEX_MODE complexMode;
};

