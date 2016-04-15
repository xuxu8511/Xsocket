#include "customer_client.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "include.h"

using namespace marco;

CustomerClient::CustomerClient(EventLoop *pLoop, const std::string &ip, uint32_t port) :
	client_(pLoop, ip, port)
{}

CustomerClient::~CustomerClient()
{}

void CustomerClient::Start() {
	client_.setNewConnCallback(this);
	client_.Start();
}

void CustomerClient::onConnection(TcpConnection *pConn) {
//	LOG(INFO) << "onConnection";
//	pConn->send("hi, man!");
}

void CustomerClient::onClose(TcpConnection *pConn) {
//	LOG(INFO) << "onClose";
}

int CustomerClient::onMessage(TcpConnection *pConn, Buffer *pBuf) {
	if (pBuf->Len() > 0) {
		int len = pBuf->Len();
		std::string message(pBuf->BeginRead(), pBuf->Len());
//		LOG(INFO) << "msg: " << message.c_str();
		//pConn->send(message);
		return len;
	}
	return 0;
}

void CustomerClient::onWriteComplete(TcpConnection *pConn) {
//	LOG(INFO) << "onWriteComplete";
}

void CustomerClient::Write(std::string &msg) {
	client_.Write(msg);
}

