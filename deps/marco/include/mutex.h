/**
 * 互斥锁
 */
#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <pthread.h>

namespace marco {

class MutexLock {
	public:
		MutexLock() {
			pthread_mutex_init(&mutexid_, NULL);
		}
		~MutexLock() {
			pthread_mutex_destroy(&mutexid_);
		}
		void lock() {
			pthread_mutex_lock(&mutexid_);
		}
		void unlock() {
			pthread_mutex_unlock(&mutexid_);
		}

	private:
		pthread_mutex_t mutexid_;
};

class MutexLockGuard {
	public:
		MutexLockGuard(MutexLock &mutex) :
			mutex_(mutex) 
	{
		mutex_.lock();
	}
		~MutexLockGuard() {
			mutex_.unlock();
		}
	private:
		MutexLock &mutex_;
};


}

#endif // _MUTEX_H_
