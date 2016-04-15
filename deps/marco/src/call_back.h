#ifndef _CALL_BACK_H_
#define _CALL_BACK_H_

#include "include.h"

namespace marco {
	class TcpConnection;
	class Buffer;
	class IAcceptorCallback {
		public:
			virtual void newConnection(int sockfd) = 0;
	};
	class ICloseCallback {
		public:
			virtual void closeConnection(int sockfd) = 0;
	};
	class IChannelCallback {
		public:
			virtual bool handleRead() = 0;
			virtual bool handleWrite() = 0;
			virtual bool handleClose() = 0;
	};
	class ITcpConnCallback {
		public:
			virtual void onConnection(TcpConnection *pCon) = 0;
			virtual void onClose(TcpConnection *pConn) = 0;
			virtual int onMessage(TcpConnection *pCon, Buffer *pBuf) = 0;
			virtual void onWriteComplete(TcpConnection *pCon) = 0;
	};

	typedef std::function<void()> EventCallback;

	const uint32_t EPOLL_INIT_SIZE = 64;
	const uint32_t TCP_BUF_SIZE = 65536;
	const uint32_t BUFFER_MAX_SIZE = 2048;
}

#endif //_CALL_BACK_H_
