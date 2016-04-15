#ifndef _EVENT_LOOP_THREAD_H_
#define _EVENT_LOOP_THREAD_H_

#include "include.h"

namespace marco {

	class EventLoop;
	class EventLoopThread {
		public:
			EventLoopThread();
			~EventLoopThread();
			EventLoop *GetLoop() {	return pLoop_;	}
			void Start();

		private:
			void run();
			EventLoop *pLoop_;
			std::thread thread_;
			std::condition_variable cond_;
			std::mutex mutex_;
	};
}

#endif //_EVENT_LOOP_THREAD_H_
