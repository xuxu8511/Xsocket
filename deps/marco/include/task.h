/**
 * 任务类
 *
 */
#ifndef _TASK_H_
#define _TASK_H_

#include "include.h"
#include "call_back.h"

namespace marco {

	class Task {
		public:
			Task(IRun0 *func);
			Task(IRun2 *func, const std::string &str, const std::shared_ptr<void> &param);
			void doTask();

		private:
			IRun0 *func0_;
			IRun2 *func2_;
			std::string str_;
			std::weak_ptr<void> param_;
	};
}

#endif //_TASK_H_
