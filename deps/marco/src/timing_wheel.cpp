#include "timing_wheel.h"
#include "event_loop.h"
#include "tcp_connection.h"
#include "timer.h"
#include "include.h"

using namespace std;
using namespace marco;

TimingWheel::TimingWheel(EventLoop *pLoop, int tim) :
	pLoop_(pLoop),
	pTimer_(new Timer(pLoop, 1, std::bind(&TimingWheel::run, this))),
	max_bucket_size_(tim),
	delete_bucket_(0),
	insert_bucket_(max_bucket_size_ -1)
{
	LOG(INFO) << "timing wheel cons";
	buckets_.resize(max_bucket_size_);
}

TimingWheel::~TimingWheel() {
	LOG(INFO) << "timing wheel des";
}

void TimingWheel::run() {
	if (pLoop_ != NULL) {
		pLoop_->runInLoop(std::bind(&TimingWheel::runInLoop, this));
	} else {
		LOG(FATAL) << "event loop not find";
	}
}

void TimingWheel::runInLoop() {
	pLoop_->assertInLoopThread();

	auto& slots = buckets_[delete_bucket_];
	for (auto iter = slots.begin(); iter != slots.end(); ++iter) {
		WeakTcpConnPtr conn = *iter;
		SharedTcpConnPtr pConn = conn.lock();
		if (pConn != NULL) {
			pConn->getEventLoop()->runInLoop(std::bind(&TcpConnection::ConnectDestroy, pConn));
		}
	}
	slots.clear();
	delete_bucket_ = (delete_bucket_ + 1) % max_bucket_size_;
	if (delete_bucket_ == 0) {
		insert_bucket_ = max_bucket_size_ - 1;
	} else {
		insert_bucket_ = delete_bucket_ - 1;
	}
}

void TimingWheel::dump() {
//	LOG(INFO) << "timing wheel dump";
}

void TimingWheel::Add(const WeakTcpConnPtr &weakConn) {
	pLoop_->assertInLoopThread();

	SharedTcpConnPtr pConn = weakConn.lock();
	if (pConn != NULL) {
		buckets_[insert_bucket_].push_back(weakConn);
		uint64_t info = uint64_t(uint64_t(buckets_[insert_bucket_].size()-1 << 32) | uint64_t(insert_bucket_));
		pConn->setBucketInfo(info);
	}
}

void TimingWheel::Update(const WeakTcpConnPtr &weakConn) {
	pLoop_->assertInLoopThread();

	SharedTcpConnPtr pConn = weakConn.lock();
	if (pConn != NULL) {
		uint64_t info = pConn->getBucketInfo();	
		if (info != 0) {
			uint32_t insert_slot_id = uint32_t(info >> 32);
			uint32_t insert_bucket_id = uint32_t(info);
			
			//如果连接在当前bucket中，则什么都不做
			if (insert_bucket_id == insert_bucket_) {
				return;
			}
			//否则，删除已有记录
			auto& slots = buckets_[insert_bucket_id];
			if (insert_slot_id >= uint32_t(slots.size())) {
				LOG(FATAL) << "bug";
			}
			slots[insert_slot_id] = SharedTcpConnPtr(NULL);
		}
		//并插入新记录
		Add(weakConn);
	}
}

void TimingWheel::Remove(const WeakTcpConnPtr &weakConn) {
	pLoop_->assertInLoopThread();

	SharedTcpConnPtr pConn = weakConn.lock();
	if (pConn != NULL) {
		uint64_t info = pConn->getBucketInfo();	
		uint32_t insert_slot_id = uint32_t(info >> 32);
		uint32_t insert_bucket_id = uint32_t(info);
		auto& slots = buckets_[insert_bucket_id];
		if (insert_slot_id >= uint32_t(slots.size())) {
			LOG(FATAL) << "bug";
		}
		slots[insert_slot_id] = SharedTcpConnPtr(NULL);
	}
}
