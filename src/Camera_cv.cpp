#include "Camera_cv.h"

#include <iostream>
#include <opencv2/highgui.hpp>

Camera_cv::Camera_cv(CODEC c, double fps, SIZE s): videoCapture(std::make_shared<cv::VideoCapture>(0)){
	if (!videoCapture->isOpened()){
		std::cerr << "init of camera failed" << std::endl;
	}
	setFps(fps);
	setCodec(c);
	setImageSize(s);
	recording = false;
	preview = false;
	lastCapture = std::make_shared<cv::Mat>();
	frameCapturedDispatcher.connect(sigc::mem_fun(*this, &Camera_cv::onImageCaptured_mainContext));
}

Camera_cv::~Camera_cv(){
	videoCapture->release();
}

void Camera_cv::setCodec(CODEC c){
	switch (c){
		case CODEC::CODEC_MPEG:
			fourcc = cv::VideoWriter::fourcc('M', 'P', 'E', 'G');
			break;
	}
}
void Camera_cv::setImageSize(SIZE s){
	switch (s){
		case SIZE::SIZE_SMALL:
			width = 320;
			height = 240;
			break;
		case SIZE::SIZE_MEDIUM:
			width = 640;
			height = 480;
			break;
		case SIZE::SIZE_LARGE:
			width = 768;
			height = 576;
			break;
	}
	
	if (!videoCapture->set(CV_CAP_PROP_FRAME_WIDTH, width)){
		std::cerr << "error setting frame width to " << width << std::endl;
	}
	if (!videoCapture->set(CV_CAP_PROP_FRAME_HEIGHT, height)){
		std::cerr << "error setting frame height to " << height << std::endl;
	}
	setFps(fps);
}
void Camera_cv::setFps(double fps){
	Camera_cv::fps = fps;
	if (!videoCapture->set(CV_CAP_PROP_FPS, fps)){
		std::cerr << "error setting fps to " << fps << std::endl;
	}
}

void Camera_cv::startPreview(){
	if (!preview){
		preview = true;
		
		if(!recording){ // thread not running
			
			thread_Record_data.vCapture = videoCapture;
			thread_Record_data.lastImage = lastCapture;
			thread_Record_data.recording = &recording;
			thread_Record_data.previewing = &preview;
			thread_Record_data.disp = &frameCapturedDispatcher;
			
			pthread_create(&thread_Record, NULL, &record, &thread_Record_data);
		}
	}
}
void Camera_cv::startRecord(std::string path){
	if (!recording){
		thread_Record_data.path = path;
		thread_Record_data.fourcc = fourcc;
		thread_Record_data.fps = fps;
		thread_Record_data.width = width;
		thread_Record_data.height = height;
		
		recording = true;
		
		if(!preview){ // thread not running
			thread_Record_data.vCapture = videoCapture;
			thread_Record_data.lastImage = lastCapture;
			thread_Record_data.recording = &recording;
			thread_Record_data.previewing = &preview;
			thread_Record_data.disp = &frameCapturedDispatcher;
			
			pthread_create(&thread_Record, NULL, &record, &thread_Record_data);
		}
	}else{
		std::cerr << "already recording" << std::endl;
	}
}
void Camera_cv::stopRecord(){
	if (recording){
		recording = false;
	}else{
		std::cerr << "not recording" << std::endl;
	}
}
void Camera_cv::stopPreview(){
	if (preview){
		preview = false;
	}else{
		std::cerr << "not previewing" << std::endl;
	}
}
void *Camera_cv::record(void * arg){
	thread_Record_data_t* data = (thread_Record_data_t*) arg;
	cv::VideoWriter *video = nullptr;
	
	
	while (*(data->recording) || *(data->previewing) == true){
		if (video == nullptr && *(data->recording)){ // create video writer object
			video = new cv::VideoWriter(data->path, data->fourcc, data->fps, cv::Size(data->width, data->height));
		}
		
		if (video != nullptr && *(data->recording) == false){ // delete video writer object
			video->release();
			delete video;
			video = nullptr;
		}
		
		//capture frame
		*(data->vCapture) >> *(data->lastImage);
		if(data->lastImage->empty()){
			break;
		}
		if (video != nullptr) video->write(*(data->lastImage));
		
		if(*(data->previewing) == true){
			data->disp->emit();
		}
	}
	
	if (video != nullptr ){ // delete video writer object
		video->release();
		delete video;
		video = nullptr;
	}
}
void Camera_cv::onImageCaptured_mainContext(){
	if (!slot_onFrameCaptured.empty()) slot_onFrameCaptured(lastCapture);
}

Camera_cv::Image_ptr Camera_cv::getLastRecordedImage() const{
	return lastCapture;
}
void Camera_cv::saveLastRecordedImage(std::string &path) const{
	cv::imwrite(path, *lastCapture);
}

void Camera_cv::connect_onFrameCapturedListener(sigc::slot<void, Camera_cv::Image_ptr> listener){
	slot_onFrameCaptured = listener;
}