#pragma once 
/**
 * @file Camera_cv.h
 * 
 * @class Camera_cv
 * @author Nils Bosbach
 * @date 17.06.2019
 * @brief Uses the opencv libary to access the raspberry pi camera
 */
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <memory>
#include <pthread.h>
#include <gtkmm.h>
#include <mutex>
 
class Camera_cv {
public:
	
	///smart pointer, points to a captured frame
	typedef std::shared_ptr<cv::Mat> Image_ptr;
	
	///codecs, which can be used to encode the video file
	enum CODEC {CODEC_MPEG};
	
	/** image sizes
	 * SIZE_SMALL: 240x320
	 * SIZE_MEDIUM: 480x640
	 * SIZE_LARGE: 576x768
	 */
	enum SIZE {SIZE_SMALL, SIZE_MEDIUM, SIZE_LARGE};
	
	/** 
	 * @brief constructor - opens the connection to the camera and sets the params
	 * @param c Codec of the video file
	 * @param fps frames per second
	 * @param s size of one frame
	 */
	Camera_cv(CODEC c = CODEC_MPEG, double fps = 10, SIZE s = SIZE_MEDIUM);
	virtual ~Camera_cv();
	
	/**
	 * @brief set the codec of the video file
	 * @param c codec
	 */
	void setCodec(CODEC c);
	
	/**
	 * @brief set the size of one frame
	 * @param s framesize
	 */
	void setImageSize(SIZE s);
	
	/**
	 * @brief set the fps of the video file
	 * @param fps frames per second
	 */
	void setFps(double fps);
	
	/**
	 * @brief start recording - a thread is created, so this method is not blocking. The last recorded frame can be accessed via getLastRecordedImage function
	 * @param path destination of the recorded video file
	 */
	void startRecord(std::string path);
	
	/**
	 * @brief grabs frames from the camera and calls the onFrameCapturedListener each time a frame has been captured. The listener can be set via connect_onFrameCapturedListener method. startPreview can be called at any time, especially during a running recording of a video.
	 */
	void startPreview();
	
	/**
	 * @brief stops saving the video to the specified file. If there is a running preview, the preview will not be stopped by this method
	 */
	void stopRecord();
	
	/**
	 * @brief stops calling the onFrameCapturedListener each time a frame has been caputured
	 */
	void stopPreview();
	
	/**
	 * @brief get the last frame which has been recorded
	 * @return pointer to the last frame which has been recorded
	 */
	Image_ptr getLastRecordedImage() const;
	
	/**
	 * @brief stores the last recorded frame on disk. The fileending of the path determines the format of the image (png, jpeg, ...)
	 * @param path place, where the last captured image should be stored
	 */
	void saveLastRecordedImage(std::string &path) const;
	
	/**
	 * @brief connect a listener, which whill be called after startPreview has been called, unitl stopPreview has been called, each time a new frame has been caputred
	 * @param listener object, which will be called each time, a frame has been captured
	 */
	void connect_onFrameCapturedListener(sigc::slot<void, Image_ptr> listener);
	
private:
	typedef std::shared_ptr<cv::VideoCapture> VideoCapture_ptr;
	typedef struct {
		VideoCapture_ptr vCapture;
		bool *recording;
		bool *previewing;
		Image_ptr lastImage;
		std::string path;
		int fourcc;
		double fps;
		int width;
		int height;
		Glib::Dispatcher* disp;
	} thread_Record_data_t;
	
	VideoCapture_ptr videoCapture;
	int fourcc;
	double fps;
	int width;
	int height;
	bool recording;
	bool preview;
	Image_ptr lastCapture;
	pthread_t thread_Record;
	thread_Record_data_t thread_Record_data;
	Glib::Dispatcher frameCapturedDispatcher;
	sigc::slot<void, Image_ptr> slot_onFrameCaptured;
	
	static void *record(void * arg); //thread function
	void onImageCaptured_mainContext(); //is called from record function in the main context
};
