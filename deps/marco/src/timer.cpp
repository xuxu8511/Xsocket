#include "timer.h"
#include "event_loop.h"
#include "channel.h"
#include "include.h"

using namespace marco;

Timer::Timer(EventLoop *pLoop, int tim, const std::function<void()> &functor) :
	pLoop_(pLoop),
	task_(functor)
{
	timerfd_ = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
	LOG(INFO) << "Timer cons, timerfd:" << timerfd_;
	
	struct itimerspec value;
	struct timespec now;
	assert(clock_gettime(CLOCK_REALTIME, &now) != -1);
	value.it_value.tv_sec = now.tv_sec + tim;
	value.it_value.tv_nsec = now.tv_nsec;
	value.it_interval.tv_sec = tim;
	value.it_interval.tv_nsec = 0;
	assert(timerfd_settime(timerfd_, TFD_TIMER_ABSTIME, &value, NULL) != -1); 

	pTimerChannel_ = std::shared_ptr<Channel>(new Channel(pLoop, timerfd_));
	pTimerChannel_->setReadCallback(std::bind(&Timer::handleRead, this));
	pTimerChannel_->setWriteCallback(std::bind(&Timer::handleWrite, this));
	pTimerChannel_->setCloseCallback(std::bind(&Timer::handleClose, this));
	pTimerChannel_->enableReading();
}

Timer::~Timer() {
	LOG(INFO) << "Timer des, sockfd:" << timerfd_;
	::close(timerfd_);
}

bool Timer::handleRead() {
 	uint64_t one = 1;
	ssize_t n = ::read(timerfd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG(ERROR) << "timer handle reads " << n << " bytes";
		return false;
	}
	task_();
	return true;
}

bool Timer::handleWrite() {
	return true;
}

bool Timer::handleClose() {
	return true;
}
