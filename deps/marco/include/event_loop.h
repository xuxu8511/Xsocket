#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include "include.h"
#include "call_back.h"
#include "timing_wheel.h"
#include "timer_queue.h"

namespace marco {
	class Channel;
	class Epoll;
	class TimingWheel;
	class TimerQueue;

	class EventLoop : public IChannelCallback {
		public:
			EventLoop();
			~EventLoop();

			void Loop();
			void Exit();
			void update(Channel *pChannel);
			void queueInLoop(const std::function<void()> &task);
			void runInLoop(const std::function<void()> &task);
			bool isInLoopThread();
			void assertInLoopThread();

			void InsertTimingWheel(const WeakTcpConnPtr &conn);
			void UpdateTimingWheel(const WeakTcpConnPtr &conn);
			void DeleteTimingWheel(const WeakTcpConnPtr &conn);
			void AddTimer(const Functor &functor, uint32_t start, double interval, bool repeat);

			virtual bool handleRead();
			virtual bool handleWrite();
			virtual bool handleClose();

			int GetEventfd() {	return eventfd_;}

		private:
			void wakeup();
			void doPendingTasks();

			int eventfd_;
			bool looping_;
			bool callingPendingTask_;
			const pthread_t threadId_;

			std::unique_ptr<Epoll> 	 pEpoll_;
			std::shared_ptr<Channel> pEventfdChannel_;

			std::mutex mutex_;
			std::vector<std::function<void()> > pendingTasks_;

			std::unique_ptr<TimerQueue> pTimerQueue_;
			std::unique_ptr<TimingWheel> pTimingWheel_;
	};
}


#endif //_EVENT_LOOP_H_
