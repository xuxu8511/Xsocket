#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "include.h"
#include "call_back.h"
#include "thread_pool.h"

namespace marco {

	class EventLoop;
	class TcpConnection;
	class Acceptor;

	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
	class TcpServer : public IAcceptorCallback, 
					  public ICloseCallback {
		public:
			TcpServer(EventLoop *pLoop, const std::string &ip, uint32_t port, int numThread);
			~TcpServer();
			void Start();
			void setNewConnCallback(ITcpConnCallback *pConnCallback);
			virtual void newConnection(int sockfd);
			virtual void closeConnection(int sockfd); 

		private:
			void closeConnectionInLoop(int coskfd);

			std::map<int, TcpConnectionPtr> connMap_;
			Acceptor *pAcceptor_;
			EventLoop *pLoop_; //base loop
			ITcpConnCallback  *pConnCallback_;
			ThreadPool threadPool_;
			int numThread_;
	};

}

#endif //_TCP_SERVER_H_
