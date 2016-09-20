#include "service_server.h"
#include "tcp_connection.h"
#include "buffer.h"
#include "include.h"
#include "pack_head.h"
#include "cg.pb.h"

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
    if (len >= HEADER_LEN) {
        PHDR header;
        pop_pack_head(pBuf->BeginRead(), header);
        LOG(INFO) << "uid:" << header.uid << ",sid:" << header.sid << ",cmd:" << header.cmd << ",len:" << header.len;
        if (header.cmd != cg::ID_C2S_Login) {
            if (header.cmd == cg::ID_C2S_KeepAlive) {
                LOG(INFO) << "keep alive protocol ing";
                KeepAlive(pConn);
                return header.len;
            }
        }
        if (header.cmd == cg::ID_C2S_Login) {
            cg::C2S_Login recv;
            if (!recv.ParseFromArray(pBuf->BeginRead() + HEADER_LEN, header.len - HEADER_LEN)) {
                LOG(ERROR) << "parse from array, uid:" << header.uid << ",sid:" << header.sid << ",cmd:" << header.cmd << ",len:" << header.len;
                return -1;
            }
            LOG(INFO) << "C2S_Login str:" << recv.DebugString();
            Login(pConn);
            return header.len;
        }
        return 0;
    }
	
    return 0;
}

void ServiceServer::onWriteComplete(TcpConnection *pConn) {
//	LOG(INFO) << "onWriteComplete";
}

void ServiceServer::KeepAlive(marco::TcpConnection *pConn) {
    LOG(INFO) << "KeepAlive";
    PHDR phdr;
    phdr.uid = 0;
    phdr.sid = pConn->GetSockFd();
    phdr.cmd = cg::ID_S2C_KeepAlive;
    cg::S2C_KeepAlive msg;
    Send(pConn, phdr, msg);
}

void ServiceServer::Login(marco::TcpConnection *pConn) {
    LOG(INFO) << "Login";
    PHDR phdr;
    phdr.uid = 0;
    phdr.sid = pConn->GetSockFd();
    phdr.cmd = cg::ID_S2C_Login;
    cg::S2C_Login msg;
    msg.set_ret(1);
    Send(pConn, phdr, msg);
}

void ServiceServer::Send(marco::TcpConnection *pConn, PHDR &header, google::protobuf::Message &body) {
    char data[MAX_MSG_SIZE];
    if(!body.SerializeToArray(data + HEADER_LEN, sizeof(data) - HEADER_LEN)) {
        LOG(ERROR) << "serialize to array failed, uid:" << header.uid << ",sid:" << header.sid << ",cmd:" << header.cmd << ",len:" << header.len;
        return;
    }

    header.len = HEADER_LEN + body.ByteSize();
    push_pack_head(data, header);

    pConn->Send(data, header.len);
}
