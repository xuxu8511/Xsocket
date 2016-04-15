#include "timer_queue.h"
#include "event_loop.h"
#include "timer.h"

using namespace marco;

TimerQueue::TimerQueue(EventLoop *pLoop) :
	pLoop_(pLoop),
	pTimer_(new Timer(pLoop, 1, std::bind(&TimerQueue::Run, this))),
	timerId_(0)	{
	LOG(INFO) << "timer queue cons";
}

TimerQueue::~TimerQueue() {
	LOG(INFO) << "timer queue des";
}

uint32_t TimerQueue::AddTimer(const Functor &functor, uint32_t start, double interval, bool repeat) {
	TimerTriggerPtr ptr(new TimerTrigger(getTimerId(), functor, start, interval, repeat));
	Entry entry(ptr->id(), ptr);
	timerSet.push_back(entry);
	return ptr->id();
}

void TimerQueue::Cancel(uint32_t id) {
	for (std::list<Entry>::iterator it = timerSet.begin(); it != timerSet.end(); it++) {
		TimerTriggerPtr timer = (*it).second;
		if (timer != NULL) {
			if ((*it).first == id) {
				timerSet.erase(it);
				return;
			}
		} else {
			timerSet.erase(it);
		}
	}
}

void TimerQueue::Run() {
	uint32_t now = std::time(NULL);
	for (std::list<Entry>::iterator it = timerSet.begin(); it != timerSet.end();) {
		TimerTriggerPtr timer = (*it).second;
		if (timer != NULL) {
			if (now >= timer->when() + timer->inter()) {
				timer->run();
				if (timer->repeat()) {
					timer->set_when(timer->when() + timer->inter());
				} else {
					it = timerSet.erase(it);
					continue;
				}
			}
		} else {
			it = timerSet.erase(it);
			continue;
		}
		it++;
	}
}
