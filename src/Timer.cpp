#include "Timer.h"
#include <chrono>

Timer::Timer(int interval): interval(interval){
	running = false;
	dispatcher = std::make_shared<Glib::Dispatcher>();
}
Timer::~Timer(){
	running = false;
}

void Timer::connect(sigc::slot<void>&& slot){
	dispatcher->connect(slot);
}
void Timer::connect(const sigc::slot<void>& slot){
	dispatcher->connect(slot);
}
void Timer::start_thread(std::shared_ptr<Glib::Dispatcher> dispatcher, bool* running){
	unsigned int *interval_ptr = &interval; 
	
	timer_thread = std::thread([dispatcher, interval_ptr, running](){
		while(*running && interval_ptr != nullptr){
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(*interval_ptr);
			
			dispatcher->emit();
			std::this_thread::sleep_until(x);
		}
	});
	timer_thread.detach();
}
void Timer::start(){
	if (running == false){
		running = true;
		start_thread(dispatcher, &running);
	}
}
void Timer::stop(){
	running = false;
}

unsigned int Timer::getInterval() const{
	return interval;
}

void Timer::setInterval(unsigned int interval){
	Timer::interval = interval;
}
