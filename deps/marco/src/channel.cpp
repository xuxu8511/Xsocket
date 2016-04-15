#include "include.h"
#include "call_back.h"
#include "event_loop.h"
#include "channel.h"

using namespace std;
using namespace marco;

Channel::Channel(EventLoop *pLoop, int sockfd) : 
	sockfd_(sockfd),
	events_(0),
	revents_(0),
	index_(-1),
	pLoop_(pLoop)
{
//	LOG(INFO) << "Channel cons, sockfd:" << sockfd_;
}

Channel::~Channel() 
{
//	LOG(INFO) << "channel des, sockfd:" << sockfd_;
}

void Channel::setRevents(int revents) {
	revents_ = revents;
}

void Channel::setIndex(int index) {
	index_ = index;
}

void Channel::handleEvent() {

	if (revents_ & EPOLLIN) {
		if (readCallback_) readCallback_();
	}
	if (revents_ & EPOLLOUT) {
		if (writeCallback_) writeCallback_();
	}
}

void Channel::enableReading() {
	events_ |= EPOLLIN;
	update();
}

void Channel::enableWriting() {
	events_ |= EPOLLOUT;
	update();
}

void Channel::disableWriting() {
	events_ &= ~EPOLLOUT;
	update();
}

bool Channel::isWriting() {
	return events_ & EPOLLOUT;
}

void Channel::disableAll() {
	events_ = 0;
	update();
}

bool Channel::isNoneEvent() {
	return events_ == 0;
}

void Channel::update() {
	pLoop_->update(this);
}

int Channel::getEvents() {
	return events_;
}

int Channel::getfd() {
	return sockfd_;
}

int Channel::getIndex() {
	return index_;
}
