#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "call_back.h"
#include "include.h"

namespace marco {

	class EventLoop;
	class Channel;
	class Acceptor : public IChannelCallback {
		public:
			Acceptor(EventLoop *pLoop, const std::string &ip, uint32_t port);
			~Acceptor();

			void start();
			void setCallback(IAcceptorCallback *pCallback);

			virtual bool handleRead();
			virtual bool handleWrite();
			virtual bool handleClose();

		private:
			int createListen(const std::string &ip, uint32_t port);
			int listenfd_;

			EventLoop *pLoop_;
			std::shared_ptr<Channel> pSocketChannel_;

			IAcceptorCallback *pCallback_;

			std::string ip_;
			uint32_t port_;
	};
	
}


#endif //_ACCEPTOR_H_
