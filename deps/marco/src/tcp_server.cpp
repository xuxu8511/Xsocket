#include "tcp_server.h"
#include "event_loop.h"
#include "tcp_connection.h"
#include "acceptor.h"

#include "include.h"

using namespace marco;

TcpServer::TcpServer(EventLoop *pLoop, const std::string &ip, uint32_t port, int numThread) :
	pLoop_(pLoop),
	pConnCallback_(NULL),
	pAcceptor_(new Acceptor(pLoop, ip, port)),
	connMap_(),
	threadPool_(pLoop),
	numThread_(numThread)
{
	pAcceptor_->setCallback(this);
}

TcpServer::~TcpServer()
{
	for (std::map<int, TcpConnectionPtr>::iterator it = connMap_.begin(); it != connMap_.end(); it++) {
		TcpConnectionPtr conn = it->second;
		conn->getEventLoop()->runInLoop(std::bind(&TcpConnection::ConnectDestroy, conn));
		it->second.reset();
	}
	if (pAcceptor_ != NULL) {
		delete pAcceptor_;
	}
}

void TcpServer::Start() {
	threadPool_.start(numThread_);
	pAcceptor_->start();
}

void TcpServer::newConnection(int sockfd) {
	EventLoop *pLoop = threadPool_.GetNextLoop();
	if (pLoop == NULL) {
		LOG(FATAL) << "BUG not find loop object";
		return;
	}
	//LOG(INFO) << "get eventloop: " << pLoop->GetEventfd() << ", sockfd: " << sockfd;
	TcpConnectionPtr conn(new TcpConnection(sockfd, pLoop, BUFFER_MAX_SIZE, TcpConnection::tServer));
	conn->setTcpConnectCallback(pConnCallback_);
	conn->setTcpCloseCallback(this);
	conn->setState(TcpConnection::kConnected);
	connMap_[sockfd] = conn;
	pLoop->runInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::closeConnection(int sockfd) {
	//LOG(INFO) << "closeConnection sockfd: " << sockfd;
	pLoop_->runInLoop(std::bind(&TcpServer::closeConnectionInLoop, this, sockfd));
}

void TcpServer::closeConnectionInLoop(int sockfd) {
	pLoop_->assertInLoopThread();
	std::map<int, TcpConnectionPtr>::iterator it = connMap_.find(sockfd);
	if (it == connMap_.end()) {
		LOG(FATAL) << "not find sockfd(" << sockfd << ") in conn map cache";
		return;
	}
	std::cout << "tcpserver close sockfd:" << sockfd << std::endl;
	pConnCallback_->onClose(it->second.get());
	size_t n = connMap_.erase(sockfd);
	(void)n;
	assert(n == 1);
}

void TcpServer::setNewConnCallback(ITcpConnCallback *pConnCallback) {
	pConnCallback_ = pConnCallback;
}
