/**
 * Channel 是一个文件描述符的观察者类
 * 可以是socket，eventfd，timerfd，signalfd
 *
 */

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "call_back.h"

namespace marco {
	class EventLoop;
	class IChannelCallback;
	class Channel {
		public:
			Channel(EventLoop *pLoop, int sockfd);
			~Channel();

			//void setCallback(IChannelCallback *pCallback);
			void handleEvent();
			
			void setReadCallback(const EventCallback &cb)  { readCallback_ = cb; 	}
			void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; 	}
			void setCloseCallback(const EventCallback &cb) { closeCallback_ = cb; 	}

			void setRevents(int revent);
			void setIndex(int index);
			void enableReading();
			void enableWriting();
			void disableWriting();
			bool isWriting();
			void disableAll();
			bool isNoneEvent();

			int getEvents();
			int getfd();
			int getIndex();
		private:
			void update();
			int sockfd_;
			int events_;
			int revents_;
			int index_;
			EventCallback readCallback_;
			EventCallback writeCallback_;
			EventCallback closeCallback_;

			EventLoop *pLoop_;
	};
}

#endif //_CHANNEL_H_
