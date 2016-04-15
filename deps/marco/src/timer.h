#ifndef _TIMER_H_
#define _TIMER_H_

#include "include.h"
#include "call_back.h"

namespace marco {

	class EventLoop;
	class Channel;

	class Timer : public IChannelCallback {
		public:
			Timer(EventLoop *pLoop, int tim, const std::function<void()> &functor);
			~Timer();
			virtual bool handleRead();
			virtual bool handleWrite();
			virtual bool handleClose();

		private:
			EventLoop *pLoop_;
			std::shared_ptr<Channel> pTimerChannel_;
			
			int timerfd_;
			const std::function<void()> task_;  //回调方法
	}; 
}

#endif //_TIMER_H_
