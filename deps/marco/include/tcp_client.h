#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "include.h"
#include "call_back.h"

namespace marco {

	class EventLoop;

	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
	class TcpClient : public ICloseCallback
	{
		public:
			enum States { kDisconnected, kConnected};
			TcpClient(EventLoop *pLoop, const std::string &ip, uint32_t port);
			~TcpClient();

			int connect();
			void disconnect(); //close

			void Start();      //开始
			void Stop();       //关闭
			void Retry();      //重连
			void Write(std::string &msg);
			bool IsConnected() {	return state_ == kConnected;}
			TcpConnectionPtr GetConn() {	return pConn_;}

			void setNewConnCallback(ITcpConnCallback *pConnCallback);

			virtual void closeConnection(int sockfd);

		private:
			static const int kInitRetryDelayMs = 5;
			static const int kMaxRetryDelayMs = 30;

			int sockfd_;
			const std::string ip_;
			uint32_t port_;

			EventLoop *pLoop_;
			TcpConnectionPtr pConn_;

			ITcpConnCallback *pConnCallback_;

			States state_;
			int retryDelayMs_;
	};
}

#endif //_TCP_CLIENT_H_
