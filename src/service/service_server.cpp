#include "service_server.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "include.h"
#include "pack_head.h"

using namespace marco;

ServiceServer::ServiceServer(EventLoop *pLoop, const std::string &ip, uint32_t port, int numThread) :
	pBaseLoop_(pLoop),
	server_(pLoop, ip, port, numThread)
{}

ServiceServer::~ServiceServer()
{}

void ServiceServer::start() {
	server_.setNewConnCallback(this);
	server_.Start();


}

void ServiceServer::onConnection(TcpConnection *pConn) {
	LOG(INFO) << "onConnection";
}

void ServiceServer::onClose(TcpConnection *pConn) {
	LOG(INFO) << "onClose";
}

//-1 : 出错
//0  : 没有数据需要处理
//>0 : 处理的数据长度(包括报头)
int ServiceServer::onMessage(TcpConnection *pConn, Buffer *pBuf) {
	uint32_t len = pBuf->Len();
/*	if (len >= HEADER_LEN) {
		PHDR header;
		pop_pack_head(pBuf->BeginRead(), header);
		LOG(INFO) << "uid:" << header.uid << ",sid:" << header.sid << ",cmd:" << header.cmd << ",len:" << header.len;
		if (len >= header.len) {
			std::string str(pBuf->BeginRead(), header.len);
			pConn->send(str);
			return header.len;
		}
		return 0;
	}
	return 0;
*/

	if (len > 0) {
		std::string str(pBuf->BeginRead(), len);
		//LOG(INFO) << "msg str:" << str.c_str();
		//pConn->send(str);
	}
	return len;
}

void ServiceServer::onWriteComplete(TcpConnection *pConn) {
//	LOG(INFO) << "onWriteComplete";
}
