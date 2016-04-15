#include "event_loop.h"
#include "tcp_client.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "call_back.h"

class CustomerClient : public marco::ITcpConnCallback {
	public:
		CustomerClient(marco::EventLoop *pLoop, const std::string &ip, uint32_t port);
		~CustomerClient();

		void Start();
		void Write(std::string &msg);
		bool IsConnected() {	return client_.IsConnected();	}

		//回调方法中不能使用CustomerClient类的数据
		//因为连接和CustomerClient对象在不同线程里
		virtual void onConnection(marco::TcpConnection *pConn);
		virtual void onClose(marco::TcpConnection *pConn);
		virtual int onMessage(marco::TcpConnection *pConn, marco::Buffer *pBuf);
		virtual void onWriteComplete(marco::TcpConnection *pConn);
		
	private:
		marco::TcpClient client_;
};
