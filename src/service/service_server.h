#include "event_loop.h"
#include "tcp_server.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "call_back.h"

class ServiceServer : public marco::ITcpConnCallback {
	public:
		ServiceServer(marco::EventLoop *pLoop, const std::string &ip, uint32_t port, int numThread);
		~ServiceServer();

		void start();

		virtual void onConnection(marco::TcpConnection *pConn);
		virtual void onClose(marco::TcpConnection *pConn);
		virtual int onMessage(marco::TcpConnection *pConn, marco::Buffer *pBuf);
		virtual void onWriteComplete(marco::TcpConnection *pConn);

	private:
		marco::EventLoop *pBaseLoop_;
		marco::TcpServer server_;
};
