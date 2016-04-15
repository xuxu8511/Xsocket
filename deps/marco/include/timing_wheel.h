#ifndef _TIMING_WHEEL_H_
#define _TIMING_WHEEL_H_

#include "include.h"

namespace marco {

	const int TIMING_WHEEL_SIZE = 300; //5分钟

	class EventLoop;
	class TcpConnection;
	class Timer;

	typedef std::shared_ptr<TcpConnection> SharedTcpConnPtr;
	typedef std::weak_ptr<TcpConnection> WeakTcpConnPtr;

	class TimingWheel {
		public:
			TimingWheel(EventLoop *pLoop, int tim = 60);
			~TimingWheel();

			void run();
			void runInLoop();

			void Add(const WeakTcpConnPtr &ptr);
			void Update(const WeakTcpConnPtr &ptr);
			void Remove(const WeakTcpConnPtr &ptr);

			void dump();

		private:
			EventLoop *pLoop_;
			std::unique_ptr<Timer> pTimer_;

			int max_bucket_size_;
			int delete_bucket_;
			int insert_bucket_;

			typedef std::vector<WeakTcpConnPtr> TimingSlotVec;
			typedef std::vector<TimingSlotVec> BucketVec;

			BucketVec buckets_;
	};
}

#endif //_TIMING_WHEEL_H_
