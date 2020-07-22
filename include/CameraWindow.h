#pragma once
/**
 * @file CameraWindow.h
 * 
 * @class CameraWindow
 * @author Nils Bosbach
 * @date 15.08.2019
 * @brief derives from Gtk::DrawingArea and implements a GUI widget, which can display a Camera_cv::Image_ptr image
 */
#include "Camera_cv.h"

#include <gtkmm.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class CameraWindow: public Gtk::DrawingArea{
public:
	
	/**
	 * @brief constructor, initializes the widget
	 */
	CameraWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~CameraWindow();
	
	/**
	 * @brief set a new imgage. The image will be converted from BRG to RGB
	 * @param image the BRG image which should be displayed
	 */
	void setImage(Camera_cv::Image_ptr image);
	
protected:
	Camera_cv::Image_ptr image;
	
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	
	
};
