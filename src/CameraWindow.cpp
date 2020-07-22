#include "CameraWindow.h"

#include <iostream>

CameraWindow::CameraWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::DrawingArea(cobject){
	
}
CameraWindow::~CameraWindow(){
	
}

bool CameraWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr){
	if (image != nullptr){
		Gtk::Allocation allocation = get_allocation();
		const int area_width = allocation.get_width(); //width of the drawing area in the gui
		const int area_height = allocation.get_height(); //height of the drawing area in the gui
		double image_width = image->cols; //width of the unscaled image
		double image_height = image->rows; //height of the unscaled image
		double image_width_new; //width of the scaled image
		double image_height_new; //height of the scaled image
		double offset_x;
		double offset_y;
		
		// calculate the dimensions of the scaled image
		if ((image_height / area_height) > (image_width / area_width)){
			image_height_new = area_height;
			image_width_new = image_height_new * image_width / image_height;
			offset_y = 0;
			offset_x = (area_width - image_width_new) / 2;
		}else{
			image_width_new = area_width;
			image_height_new = image_width_new * image_height / image_width;
			offset_x = 0;
			offset_y = (area_height - image_height_new) / 2;
		}
		
		Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data((guint8*)image->data, Gdk::Colorspace::COLORSPACE_RGB, false, 8, image->cols, image->rows, (int)image->step);
		
		Gdk::Cairo::set_source_pixbuf(cr, pixbuf->scale_simple(image_width_new, image_height_new, Gdk::INTERP_BILINEAR), offset_x, offset_y);
		cr->paint();
	}
	return true;
}

void CameraWindow::setImage(Camera_cv::Image_ptr image){
	Camera_cv::Image_ptr converted = std::make_shared<cv::Mat>();
	cv::cvtColor(*image, *converted, CV_BGR2RGB);
	CameraWindow::image = converted;
	queue_draw();
}