#include "event_loop_thread.h"
#include "event_loop.h"

using namespace marco;

EventLoopThread::EventLoopThread() :
	pLoop_(NULL),
	thread_(&EventLoopThread::run, this) 
{}

EventLoopThread::~EventLoopThread() {
	if (thread_.joinable()) {
		thread_.join();
	}
	if (pLoop_) { 
		pLoop_->Exit();
		delete pLoop_;
	}
}

void EventLoopThread::Start() {
	std::unique_lock<std::mutex> lock(mutex_);
	while (pLoop_ == NULL) {
		cond_.wait(lock);
	}
}

void EventLoopThread::run() {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		pLoop_ = new EventLoop();
	}
	cond_.notify_one();
	pLoop_->Loop();
}
