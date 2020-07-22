#include "PlotWindow.h"
#include "FSHelper.h"

#include <string>
#include <iostream>
#include <math.h>

#define degToRad(angleDeg) ((angleDeg) * M_PI / 180.0)
#define radToDeg(angleRad) ((angleRad) * 180.0 / M_PI)

#define DISTANCE_LEFT 10
#define DISTANCE_RIGHT 10
#define DISTANCE_TOP 10
#define DISTANCE_BOTTOM 40
#define GRID_LINE_WIDTH 0.3
#define AXIS_LINE_WIDTH 1.0
#define AXIS_STROKE_LENGTH 6
#define PLOT_LINE_WIDTH 1.0
#define STROKES_LINE_WIDTH 1.2
#define OFFSET_TEXT_X 15
#define OFFSET_TEXT_Y 5
#define AVERAGE_LETTER_WIDTH 10
#define LABEL_SIZE 10
#define LABEL_BODER_DISTANCE 10
#define MIN_DECADE_PIXELS_X 80
#define MIN_DECADE_PIXELS_Y 50


PlotWindow::PlotWindow(){
	init();
}
PlotWindow::PlotWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::DrawingArea(cobject){
	init();
}
void PlotWindow::init(){
	x_min = -20;
	x_max = 100;
	x_scl = 2;
	y_min = -15;
	y_max = 15;
	y_scl = 0.2;
	
	x_min_data = 0;
	x_max_data = 0;
	y_min_data = 0;
	y_max_data = 0;
}

PlotWindow::~PlotWindow(){

}

/*
 * draw the coordinate system and plot data
 */
bool PlotWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& cr){
//	std::cout << "on_draw begin" << std::endl;
	if (dataMtx.try_lock()){
		if (!plot_data.empty()){
			dataMtx.unlock();
			updateWindowSizes();
		}else{
			dataMtx.unlock();
		}
		if (dataMtx.try_lock()){
			
			const int x_axis_label_height = (x_axis_label.empty() ? 0 : LABEL_SIZE + LABEL_BODER_DISTANCE);
			const int y_axis_label_width = (y_axis_label.empty() ? 0 : LABEL_SIZE + LABEL_BODER_DISTANCE);
			const int maxYLabelWidth =  getMaxLabelWidth(y_min, y_max, y_scl);
			Gtk::Allocation allocation = get_allocation();
			const int window_width = allocation.get_width();
			const int window_height = allocation.get_height();
			const int x_axis_length = window_width - (DISTANCE_LEFT + maxYLabelWidth + y_axis_label_width + DISTANCE_RIGHT);
			const int y_axis_length = window_height - (DISTANCE_TOP + x_axis_label_height + DISTANCE_BOTTOM);
			const int x_zero = DISTANCE_LEFT + maxYLabelWidth + y_axis_label_width;
			const int y_zero = DISTANCE_TOP + y_axis_length;
			double toDraw_x; 
			double toDraw_y; 
			
			cr->set_source_rgb(0.0, 0.0, 0.0);
			
			// x axis
			cr->set_line_width(AXIS_LINE_WIDTH);
			cr->move_to(x_zero,                 y_zero);
			cr->line_to(x_zero + x_axis_length, y_zero);
			
			// y axis
			cr->set_line_width(AXIS_LINE_WIDTH);
			cr->move_to(x_zero, y_zero);
			cr->line_to(x_zero, y_zero - y_axis_length);
			cr->stroke();
			
			// x-Axis-Label
			draw_text(cr, x_axis_label, x_zero + x_axis_length / 2.0, window_height - LABEL_BODER_DISTANCE, ALIGN::CENTER);
			
			// y-Axis-Label
			draw_text(cr, y_axis_label, LABEL_BODER_DISTANCE, y_zero - y_axis_length / 2.0, ALIGN::CENTER, degToRad(-90));
			
			if (!plot_data.empty()){
				
				switch(scale_x){
					case SCALE::LINEAR:
						toDraw_x =  x_axis_length / (x_max - x_min);
						break;
					case SCALE::LOG:
						toDraw_x = x_axis_length / (log10(x_max) - log10(x_min));
						break;
				}
				switch(scale_y){
					case SCALE::LINEAR:
						toDraw_y =  y_axis_length / (y_max - y_min);
						break;
					case SCALE::LOG:
						toDraw_y = y_axis_length / (log10(y_max) - log10(y_min));
						break;
				}
				
				
				if(scale_x == SCALE::LINEAR){
					
					double x_coord; //absolute x_coord. in Draw Area
					
					// x-Axis numbers
					for (double x = x_min; x <= x_max; x += x_scl){
						x_coord = x_zero + (x-x_min)  * toDraw_x;
						
						draw_text(cr, FSHelper::formatDouble(x), x_coord, y_zero + OFFSET_TEXT_X, ALIGN::CENTER);
						
						//strokes
						if (showAxisStrokes){
							cr->set_line_width(STROKES_LINE_WIDTH);
							cr->move_to(x_coord, y_zero - AXIS_STROKE_LENGTH / 2);
							cr->line_to(x_coord, y_zero + AXIS_STROKE_LENGTH / 2);
						}
						cr->stroke();
						
						//grid
						if (showGrid){
							cr->set_line_width(GRID_LINE_WIDTH);
							cr->move_to(x_zero + (x-x_min) * toDraw_x, y_zero);
							cr->line_to(x_zero + (x-x_min) * toDraw_x, y_zero - y_axis_length);
						}
						cr->stroke();
					}
				
				}else if (scale_x == SCALE::LOG){
					int max_exp = ceil(log10(x_max));
					int min_exp = floor(log10(x_min));
					
					
					double value; //the current value (actual value)
					double x_coord; //absolute x_coord. in Draw Area
					int x_decades = max_exp - min_exp;
					double decade_step_x = ((double)(x_decades * MIN_DECADE_PIXELS_X)) / ((double)x_axis_length);
					int decade_step_x_int = (ceil(decade_step_x) > 0 ? ceil(decade_step_x) : 1);
					
					//draw the grid, strokes and axes labes
					for(int current_exp = min_exp; current_exp < max_exp; current_exp += decade_step_x_int){
						for(int i = 1; i < 10; i++){
							value = i * pow(10, current_exp);
							x_coord = x_zero + (log10(value) - log10(x_min)) * toDraw_x; //absolute y_coord. in Draw Area
							
							//draw just the first no of a decade (0.1, 1, 10, ...) or the whole decade if decade_step_x_int is 1
							if((decade_step_x_int == 1) || (i == 1)){
								
								if (i == 1){
									draw_text(cr, FSHelper::formatDouble(value), x_coord, y_zero + OFFSET_TEXT_X, ALIGN::CENTER);
									
									//Strokes
									if (showAxisStrokes){
										cr->set_line_width(STROKES_LINE_WIDTH);
										cr->move_to(x_coord, y_zero - AXIS_STROKE_LENGTH / 2);
										cr->line_to(x_coord, y_zero + AXIS_STROKE_LENGTH / 2);
									}
									cr->stroke();
								}
								
								//grid
								if (showGrid){
									cr->set_line_width(GRID_LINE_WIDTH);
									cr->move_to(x_coord, y_zero);
									cr->line_to(x_coord, y_zero - y_axis_length);
								}
								cr->stroke();
							}
						}
					}
				}
				
				if(scale_y == SCALE::LINEAR){
					double y_coord; //absolute y_coord. in Draw Area
					
					// y-Axis numbers
					for (double y = y_min; y <= y_max; y += y_scl){
						y_coord = y_zero - (y-y_min) * toDraw_y;
						
						draw_text(cr, FSHelper::formatDouble(y), x_zero - OFFSET_TEXT_Y, y_coord, ALIGN::RIGHT);
						//Strokes
						if (showAxisStrokes){
							cr->set_line_width(STROKES_LINE_WIDTH);
							cr->move_to(x_zero - AXIS_STROKE_LENGTH / 2, y_coord);
							cr->line_to(x_zero + AXIS_STROKE_LENGTH / 2, y_coord);
						}
						cr->stroke();
						
						//grid
						if (showGrid){
							cr->set_line_width(GRID_LINE_WIDTH);
							cr->move_to(x_zero,                 y_coord);
							cr->line_to(x_zero + x_axis_length, y_coord);
						}
						cr->stroke();
					}
				}else if (scale_y == SCALE::LOG){
					int max_exp = ceil(log10(y_max));
					int min_exp = floor(log10(y_min));
					
					
					
					double value; //the current value (actual value)
					double y_coord; //absolute y_coord. in Draw Area
					int y_decades = max_exp - min_exp;
					double decade_step_y = ((double)(y_decades * MIN_DECADE_PIXELS_Y)) / ((double)y_axis_length);
					int decade_step_y_int = ceil(decade_step_y);
					
					//draw the grid, strokes and axes labes
					for(int current_exp = min_exp; current_exp < max_exp; current_exp += decade_step_y_int){
						for(int i = 1; i < 10; i++){
							value = i * pow(10, current_exp);
							y_coord = y_zero - (log10(value) - log10(y_min)) * toDraw_y; //absolute y_coord. in Draw Area
							
							//draw just the first no of a decade (0.1, 1, 10, ...) or the whole decade if decade_step_x_int is 1
							if((decade_step_y_int == 1) || (i == 1)){
								
								if (i == 1){
									draw_text(cr, FSHelper::formatDouble(value), x_zero - OFFSET_TEXT_Y, y_coord, ALIGN::RIGHT);
								
									//Strokes
									if (showAxisStrokes){
										cr->set_line_width(STROKES_LINE_WIDTH);
										cr->move_to(x_zero - AXIS_STROKE_LENGTH / 2, y_coord);
										cr->line_to(x_zero + AXIS_STROKE_LENGTH / 2, y_coord);
									}
									cr->stroke();
								}
								
								//grid
								if (showGrid){
									cr->set_line_width(GRID_LINE_WIDTH);
									cr->move_to(x_zero, y_coord);
									cr->line_to(x_zero + x_axis_length, y_coord);
								}
								cr->stroke();
							}
						}
					}
				}
				
				cr->stroke();
				
				// additional line at x = 0
				if(x_min < 0 && x_max > 0 && showZeroAxis && scale_x == LINEAR){
					cr->set_line_width(AXIS_LINE_WIDTH);
					cr->move_to(x_zero - x_min * toDraw_x, y_zero );
					cr->line_to(x_zero - x_min * toDraw_x, y_zero - y_axis_length);
				}
				// additional line at y = 0
				if(y_min < 0 && y_max > 0 && showZeroAxis && scale_y == LINEAR){
					cr->set_line_width(AXIS_LINE_WIDTH);
					cr->move_to(x_zero,                 y_zero + y_min * toDraw_y);
					cr->line_to(x_zero + x_axis_length, y_zero + y_min * toDraw_y);
				}
				cr->stroke();
				
				
				//plot data
				
				cr->set_source_rgb(0.0, 0.0, 1.0);
				if (plot_data.size() > 1){ //only plot data if there are at least 2 data points
					cr->set_line_width(PLOT_LINE_WIDTH);
					
					//for each data point
					for (std::vector<DataP::DataP_ptr>::iterator it = plot_data.begin() ; it != plot_data.end(); ++it){
						int x, y; //absolute coordinates 
						DataP::DataP_ptr data_point = *it;
						
						switch (scale_x){
							case SCALE::LINEAR:
								x = (data_point->getX() - x_min)*toDraw_x + x_zero;
								break;
							case SCALE::LOG:
								x = (log10(data_point->getX()) - log10(x_min))*toDraw_x + x_zero;
								break;
						}
						switch (scale_y){
							case SCALE::LINEAR:
								y = (y_min - (data_point->getY(complexMode)))*toDraw_y + y_zero;
								break;
							case SCALE::LOG:
								y = (log10(y_min) - log10(data_point->getY(complexMode)))*toDraw_y + y_zero;
								break;
						}
						
						if (it != plot_data.begin()){
							cr->line_to(x, y);
						}
						cr->move_to(x, y);
					}
				}
			}
			dataMtx.unlock();
			cr->stroke();
		}
	}
//	std::cout << "on_draw end" << std::endl;
	return true;
}

int PlotWindow::getMaxLabelWidth(int startVal, int stopVal, double step){
	int max_width = 0;
//	std::cout << std::endl <<  "start: " << startVal << "\tstop: " << stopVal << "\tstep: " << step << std::endl;
	for (double x = startVal; x <= stopVal; x += step){
		std::string text = FSHelper::formatDouble(x);
//		std::cout << text << std::endl;
		int text_width = text.size() * AVERAGE_LETTER_WIDTH; //use a average letter width for calculation
		
		if (text_width > max_width){
			max_width = text_width;
		}
	}
	return max_width;
}
/*
 * add a text at position (x,y) to the Drawing Area
 */
void PlotWindow::draw_text(const Cairo::RefPtr<Cairo::Context>& cr, Glib::ustring text, int x, int y, ALIGN a, double rotation_rad){
	// http://developer.gnome.org/pangomm/unstable/classPango_1_1FontDescription.html
	Pango::FontDescription font;
	
	font.set_family("Monospace");
	font.set_weight(Pango::WEIGHT_LIGHT);
	
	// http://developer.gnome.org/pangomm/unstable/classPango_1_1Layout.html
	Glib::RefPtr<Pango::Layout> layout = create_pango_layout(text);
	
	layout->set_font_description(font);
	
	int text_width;
	int text_height;
	
	//get the text dimensions (vars are updated by reference)
	layout->get_pixel_size(text_width, text_height);
	cr->save();
	cr->rotate(rotation_rad); 
	
	double x_dest, y_dest; // refer to the unrotated coordiate system
	switch(a){
	case ALIGN::CENTER:
		x_dest = x - (std::cos(rotation_rad) * text_width - std::sin(rotation_rad) * text_height) / 2;
		y_dest = y - (std::sin(rotation_rad) * text_width + std::cos(rotation_rad) * text_height) / 2;
		break;
	case ALIGN::RIGHT:
		x_dest = x - std::cos(rotation_rad) * text_width + std::sin(rotation_rad) * text_height / 2;
		y_dest = y - std::sin(rotation_rad) * text_width - std::cos(rotation_rad) * text_height / 2;
		break;
	case ALIGN::LEFT:
		x_dest = x + std::sin(rotation_rad) * text_height / 2;
		y_dest = y - std::cos(rotation_rad) * text_height / 2;
		break;
	}
	
	// transform the coordinates to the transformed system
	double x_dest_trans =  x_dest * std::cos(rotation_rad) + y_dest * std::sin(rotation_rad);
	double y_dest_trans =  (-1) * x_dest * std::sin(rotation_rad) + y_dest * std::cos(rotation_rad);
	cr->move_to(x_dest_trans, y_dest_trans);
	
	layout->show_in_cairo_context(cr);
	cr->restore();
}

/*
 * calculate x_max, x_min, x_scl, y_max, y_min, y_scl depending on plot_data entries
 */
void PlotWindow::updateWindowSizes(){
	dataMtx.lock();
	/* given:
	 * x_min_data, x_max_data
	 * y_min_data, y_max_data
	 * autoMax_x, autoMax_y
	 * scale_x, scale_y
	 * autoPadding_x, autoPadding_y
	 * 
	 * needed:
	 * x_min, x_max
	 * y_min, y_max
	 * x_scl, y_scl
	 */
	
	
	if (!plot_data.empty()){
		if (autoMax_x){
			if (autoPadding_x){
				if (autoScl_x){
					x_scl = getScl_x();
				}
				switch(scale_x){
				case SCALE::LINEAR:
					x_min = floor(x_min_data / x_scl) * x_scl;
					x_max = ceil(x_max_data / x_scl) * x_scl;
					break;
				case SCALE::LOG:
					x_min = pow(10, floor(log10(x_min_data)));
					x_max = pow(10, ceil (log10(x_max_data)));
					break;
				}
			}else{
				x_min = x_min_data;
				x_max = x_max_data;
			}
		}
		
		if (autoMax_y){
			if (autoPadding_y){
				if (autoScl_y){
					y_scl = getScl_y();
				}
				switch(scale_y){
				case SCALE::LINEAR:
					y_min = floor(y_min_data / y_scl) * y_scl;
					y_max = ceil (y_max_data / y_scl) * y_scl;
					break;
				case SCALE::LOG:
					y_min = pow(10, floor(log10(y_min_data)));
					y_max = pow(10, ceil (log10(y_max_data)));
					
					if (y_min == 0){ //error in log scale
						y_min = 1e-20;
					}
					break;
				}
			}else{
				y_min = y_min_data;
				y_max = y_max_data;
			}
		}
		
		if ((plot_data.size() > 1)){
			if (autoScl_x){
				x_scl = getScl_x();
			}
			if (autoScl_y){
				y_scl = getScl_y();
			}
		}
	}
	dataMtx.unlock();
}

// dataMtx needs to be locked externaly
void PlotWindow::calculateDataRage(){
	if (!plot_data.empty()){
		x_min_data = plot_data.front()->getX();
		x_max_data = plot_data.front()->getX();
		y_min_data = plot_data.front()->getY(complexMode);
		y_max_data = plot_data.front()->getY(complexMode);
	}else{
		x_min_data = 0;
		x_max_data = 0;
		y_min_data = 0;
		y_max_data = 0;
	}
	
	for (std::vector<DataP::DataP_ptr>::const_iterator cit = plot_data.cbegin(); cit != plot_data.cend(); cit++){
//		std::cout << "x_min_data: " << x_min_data << std::endl;
//		std::cout << "x_max_data: " << x_min_data << std::endl;
//		std::cout << "y_min_data: " << y_min_data << std::endl;
//		std::cout << "y_max_data: " << y_max_data << std::endl;
		
		DataP::DataP_ptr data_point = *cit;
		if (data_point->getX() < x_min_data){
			x_min_data = data_point->getX();
		}
		
		if (data_point->getX() > x_max_data){
			x_max_data = data_point->getX();
		}
		
		if (data_point->getY(complexMode) < y_min_data){
			y_min_data = data_point->getY(complexMode);
		}
		
		if (data_point->getY(complexMode) > y_max_data){
			y_max_data = data_point->getY(complexMode);
		}
	}
}

/*
 * set the entire data
 */
void PlotWindow::setData(std::vector<DataP::DataP_ptr> data){
//	for (std::vector<DataP::DataP_ptr>::iterator it = data.begin(); it != data.end(); it++){
//		DataP::DataP_ptr data = *it;
//		std::cout << data->getX() << " - " << data->getY(DataP::COMPLEX_MODE::COMPLEX_REAL) << " - " << data->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG) << std::endl;
//	}
	
	dataMtx.lock();
	
	plot_data.clear();
	plot_data = data;
	calculateDataRage();

	dataMtx.unlock();
	
	updateWindowSizes();
	queue_draw();
}

/*
 * calculate scl from width
 */
double PlotWindow::getScl_x(){
	return getScl(get_width(), x_max - x_min);
}
double PlotWindow::getScl_y(){
	return getScl(get_height(), y_max - y_min);
}
double PlotWindow::getScl(double widget_dim, double axis_dim){
	double unitsPerDot = axis_dim / (double)widget_dim;
	double scale = 0;
	
	if ( unitsPerDot == 0){
		return 1;
	}
	
	if (unitsPerDot > 1){
		scale = ceil(unitsPerDot) * 100;
	}else{
		int exp = floor(log10(unitsPerDot));
	
		scale = ceil(unitsPerDot / pow(10, exp)) * 100 * pow(10, exp);
	}
	return scale;
}

/*
 * add one DataPoint
 */
void PlotWindow::addData(DataP::DataP_ptr data){
	dataMtx.lock();
	if (plot_data.empty()){
		x_min_data = data->getX();
		x_max_data = data->getX();
		y_min_data = data->getY(complexMode);
		y_max_data = data->getY(complexMode);
	}
	
	plot_data.push_back(data);
	if (data->getX() < x_min_data){
		x_min_data = data->getX();
	}
	
	if (data->getX() > x_max_data){
		x_max_data = data->getX();
	}
	
	if (data->getY(complexMode) < y_min_data){
		y_min_data = data->getY(complexMode);;
	}
	
	if (data->getY(complexMode) > y_max_data){
		y_max_data = data->getY(complexMode);;
	}
	dataMtx.unlock();
	updateWindowSizes();
	queue_draw();
}
void PlotWindow::setComplexMode(DataP::COMPLEX_MODE c){
	complexMode = c;

	dataMtx.lock();
	calculateDataRage();
	dataMtx.unlock();
	
	updateWindowSizes();
	queue_draw();
}

double PlotWindow::getXminData() const{
	return x_min_data;
}
double PlotWindow::getXmaxData() const{
	return x_max_data;
}
double PlotWindow::getYminData() const{
	return y_min_data;
}
double PlotWindow::getYmaxData() const{
	return y_max_data;
}
double PlotWindow::getYaverage(){
	double sum = 0;
	int size = 1;
	dataMtx.lock();
	for (std::vector<DataP::DataP_ptr>::const_iterator cit = plot_data.cbegin(); cit != plot_data.cend(); cit++){
		sum += (*cit)->getY(complexMode);
	}
	
	size = plot_data.size();
	dataMtx.unlock();
	return sum / size;
}