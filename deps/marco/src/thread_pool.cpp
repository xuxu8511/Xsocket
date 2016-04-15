#include "thread_pool.h"
#include "include.h"
#include "event_loop.h"
#include "event_loop_thread.h"

using namespace marco;

ThreadPool::ThreadPool(EventLoop *pLoop) :
	pBaseLoop_(pLoop),
	next_(0)
{}

void ThreadPool::start(int numThreads) {
	for (int i = 0; i < numThreads; i++) {
		EventLoopThread *pLoopThread = new EventLoopThread();
		loopThreads_.push_back(pLoopThread);
		pLoopThread->Start();
	}
}

EventLoop* ThreadPool::GetNextLoop() {
	EventLoop *loop = pBaseLoop_;
	if (!loopThreads_.empty()) {
		loop = loopThreads_[next_++]->GetLoop();
		if (next_ == loopThreads_.size()) {
			next_ = 0;
		}
	}
	return loop;
}

