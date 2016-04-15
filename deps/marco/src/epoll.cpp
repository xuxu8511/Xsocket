#include "epoll.h"
#include "channel.h"
#include "include.h"
#include "call_back.h"

using namespace marco;

const int kNew = -1;
const int kAdded = 1;
const int kDeled = 2;

Epoll::Epoll() : 
	epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	events_(EPOLL_INIT_SIZE)
{}

Epoll::~Epoll()
{}

void Epoll::poll(std::vector<Channel*> *pChannels) {
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), 500);
	if (numEvents < 0 && errno != EINTR) {
		LOG(ERROR) << "epoll wait error, errno: " << errno;
	}
	for (int i = 0; i < numEvents; i++) {
		Channel *pChannel = static_cast<Channel*>(events_[i].data.ptr);
		pChannel->setRevents(events_[i].events);
		pChannels->push_back(pChannel);
	}
	if (numEvents == events_.size()) {
		events_.resize(events_.size() * 1.5);
		LOG(INFO) << "event size increase:" << events_.size();
	}
}

void Epoll::update(Channel *pChannel) {
	int index = pChannel->getIndex();
	if (index == kNew) {
		struct epoll_event ev;
		bzero(&ev, sizeof ev);
		ev.data.ptr = pChannel;
		ev.events = pChannel->getEvents();
		int fd = pChannel->getfd();
		pChannel->setIndex(kAdded);
		if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
			LOG(ERROR) << "epoll ctl EPOLL_CTL_ADD failed";
		}
	} else {
		struct epoll_event ev;
		bzero(&ev, sizeof ev);
		ev.data.ptr = pChannel;
		ev.events = pChannel->getEvents();
		int fd = pChannel->getfd();
		if (pChannel->isNoneEvent()) {
			if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev) < 0) {
				LOG(ERROR) << "epoll ctl EPOLL_CTL_DEL failed";
			}
		} else {
			if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
				LOG(ERROR) << "epoll ctl EPOLL_CTL_MOD failed";
			}
		}
	}
}

