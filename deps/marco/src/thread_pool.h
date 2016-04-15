/**
 * 线程池封装
 */
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "include.h"

namespace marco {

	class EventLoop;
	class EventLoopThread;
	class ThreadPool {
		public:
			ThreadPool(EventLoop *pLoop);
			void start(int numThreads);
			EventLoop* GetNextLoop();

		private:
			EventLoop *pBaseLoop_;
			std::vector<EventLoopThread*> loopThreads_;
			int next_;
	};
}

#endif //_THRAD_POOL_H_

