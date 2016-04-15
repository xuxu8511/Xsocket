#ifndef _EPOLL_H_
#define _EPOLL_H_

#include "include.h"

struct epoll_event;

namespace marco {
	class Channel;

	class Epoll {
		public:
			Epoll();
			~Epoll();

			void poll(std::vector<Channel*> *pChannels);
			void update(Channel *pChannel);
		private:
			int epollfd_;
			typedef std::vector<struct epoll_event> EventList;
			EventList events_;
	};

}
#endif //_EPOLL_H_
