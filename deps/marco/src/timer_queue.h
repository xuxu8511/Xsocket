#ifndef _TIMER_QUEUE_H_
#define _TIMER_QUEUE_H_

#include "include.h"

namespace marco {
	class EventLoop;
	class Timer;
	
	typedef std::function<void()> Functor;

	class TimerTrigger {
		public:
			TimerTrigger(uint32_t id, const Functor &functor, uint32_t start, double interval, bool repeat) :
				id_(id),
				callback_(functor),
				start_(start),
				interval_(interval),
				repeat_(repeat), 
				when_(start) 
			{}

			uint32_t when() {	return when_;}
			uint32_t inter(){	return interval_;}
			uint32_t id() 	{	return id_;}
			bool repeat() 	{	return repeat_;}
			void run() 		{	callback_();}

			void set_when(uint32_t n) {	when_ = n;}

		private:
			const uint32_t id_;
			const Functor callback_;
			const uint32_t start_;
			const double interval_;
			const bool repeat_;

			uint32_t when_;
	};
	
	typedef std::shared_ptr<TimerTrigger> TimerTriggerPtr;

	class TimerQueue {
		public:
			TimerQueue(EventLoop *pLoop);
			~TimerQueue();

			uint32_t AddTimer(const Functor &functor, uint32_t start, double interval, bool repeat);
			void Cancel(uint32_t id);
			void Run();

			uint32_t getTimerId() {	return timerId_++;	}

		private:
			EventLoop *pLoop_;
			std::unique_ptr<Timer> pTimer_;	
			
			uint32_t timerId_;

			typedef std::pair<uint32_t, TimerTriggerPtr> Entry;
			std::list<Entry> timerSet;
	};
}

#endif// _TIMER_QUEUE_H_
