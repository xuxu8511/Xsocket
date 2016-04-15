#include "event_loop.h"
#include "include.h"
#include "channel.h"
#include "epoll.h"
#include "call_back.h"
#include "timing_wheel.h"
#include "timer_queue.h"

using namespace marco;

EventLoop::EventLoop() :
	eventfd_(::epoll_create1(EPOLL_CLOEXEC)),
	looping_(true),
	callingPendingTask_(false),
	threadId_(::pthread_self()),
	pEpoll_(new Epoll()),
	pTimerQueue_(new TimerQueue(this)),
	pTimingWheel_(new TimingWheel(this, TIMING_WHEEL_SIZE))
{
	LOG(INFO) << "EventLoop cons eventfd: " << eventfd_;
	assert(eventfd_ > 0);
	
	pEventfdChannel_ = std::shared_ptr<Channel>(new Channel(this, eventfd_));
	//pEventfdChannel_->setCallback(this);
	pEventfdChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	pEventfdChannel_->setWriteCallback(std::bind(&EventLoop::handleWrite, this));
	pEventfdChannel_->setCloseCallback(std::bind(&EventLoop::handleClose, this));
	pEventfdChannel_->enableReading();
}

EventLoop::~EventLoop() {
	LOG(INFO) << "EventLoop des";
	assert(looping_ == false);
	::close(eventfd_);
}

void EventLoop::Loop() {
	while (looping_) {
		std::vector<Channel*> channels;
		pEpoll_->poll(&channels);

		for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
			(*it)->handleEvent();
		}
		doPendingTasks();
	}
}

void EventLoop::Exit() {
	looping_ = false;
}

void EventLoop::update(Channel *pChannel) {
	pEpoll_->update(pChannel);
}

bool EventLoop::handleRead() {
	//LOG(INFO) << "eventloop handle read, eventfd: " << eventfd_;
	uint64_t one = 1;
	ssize_t n = ::read(eventfd_, &one, sizeof one);
	if (n != sizeof one) {
		//std::cout << "event loop handle read reads " << n << " bytes" << std::endl;
		return false;
	}
	return true;
}

bool EventLoop::handleWrite() {
	//LOG(INFO) << "eventloop handle write, eventfd: " << eventfd_;
	uint64_t one = 1;
	ssize_t n = ::write(eventfd_, &one, sizeof one);
	if (n != sizeof one) {
		//std::cout << "event loop handle write " << n << " bytes " << std::endl;
		return false;
	}
	return true;
}

bool EventLoop::handleClose() {
	return true;
}

void EventLoop::wakeup() {
	handleWrite();
}

void EventLoop::queueInLoop(const std::function<void()> &task) {
	{
		std::lock_guard<std::mutex> guard(mutex_);
		pendingTasks_.push_back(task);
	}

	//不是在相应线程中执行或者相应线程正在调用doPendingTasks方法，则唤醒相应线程
	if (!isInLoopThread() || callingPendingTask_) {
		wakeup();
	}
}

void EventLoop::runInLoop(const std::function<void()> &task) {
	if (isInLoopThread()) {
		task();
	} else {
		queueInLoop(task);
	}
}

bool EventLoop::isInLoopThread() {
	return threadId_ == ::pthread_self();  
}

void EventLoop::assertInLoopThread() {
	if (!isInLoopThread()) {
		LOG(FATAL) << "not in loop thread";
	}
}

void EventLoop::doPendingTasks() {
	std::vector<std::function<void()> > tempTasks;
	callingPendingTask_ = true;
	{
		std::lock_guard<std::mutex> guard(mutex_);
		tempTasks.swap(pendingTasks_);
	}
	for (size_t i = 0; i < tempTasks.size(); ++i) {
		tempTasks[i]();
	}

	callingPendingTask_ = false;
}

void EventLoop::InsertTimingWheel(const WeakTcpConnPtr &conn) {
	assertInLoopThread();
	pTimingWheel_->Add(conn);
}

void EventLoop::UpdateTimingWheel(const WeakTcpConnPtr &conn) {
	assertInLoopThread();
	pTimingWheel_->Update(conn);
}

void EventLoop::DeleteTimingWheel(const WeakTcpConnPtr &conn) {
	assertInLoopThread();
	pTimingWheel_->Remove(conn);
}

void EventLoop::AddTimer(const Functor &functor, uint32_t start, double interval, bool repeat) {
	pTimerQueue_->AddTimer(functor, start, interval, repeat);
}

