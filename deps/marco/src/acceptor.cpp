#include "acceptor.h"
#include "event_loop.h"
#include "channel.h"
#include "include.h"

using namespace marco;

Acceptor::Acceptor(EventLoop *pLoop, const std::string &ip, uint32_t port) :	
	listenfd_(-1),
	pSocketChannel_(NULL),
	pCallback_(NULL),
	pLoop_(pLoop),
	ip_(ip),
	port_(port)
{}

Acceptor::~Acceptor()
{}

void Acceptor::start() {
	listenfd_ = createListen(ip_, port_);

	LOG(INFO) << "create listen sockfd:" << listenfd_;
	pSocketChannel_ = std::shared_ptr<Channel>(new Channel(pLoop_, listenfd_));
	pSocketChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this));
	pSocketChannel_->setWriteCallback(std::bind(&Acceptor::handleWrite, this));
	pSocketChannel_->setCloseCallback(std::bind(&Acceptor::handleClose, this));
	pSocketChannel_->enableReading();
}

void Acceptor::setCallback(IAcceptorCallback *pCallback) {
	pCallback_ = pCallback;
}

bool Acceptor::handleRead() 
{
	int connfd;
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	connfd = ::accept(listenfd_, (sockaddr*)&cliaddr, (socklen_t*)&clilen);
	if (connfd > 0) {
		LOG(INFO) << "new connection from "
			<< "[" << inet_ntoa(cliaddr.sin_addr)
			<< ":" << ntohs(cliaddr.sin_port) << "]"
			<< " new socket fd:" << connfd;
	} else {
		LOG(ERROR) << "accept error, connfd:" << connfd
			<< " errno:" << errno;
		return false;
	}
	
	int flags;
	if ((flags = ::fcntl(connfd, F_GETFL)) == -1) {
		LOG(ERROR) << "fcntl getfl error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}
	if(-1 == ::fcntl(connfd, F_SETFL, flags | O_NONBLOCK)) {
		LOG(ERROR) << "fcntl setfl error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}

	int yes = 1;
	if(-1 == ::setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) {
		LOG(ERROR) << "setsockopt nodelay error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}

	int on = 1;
	if(-1 == setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		LOG(ERROR) << "setsockopt reuseaddr error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}

	int buffsize = TCP_BUF_SIZE;
	if(-1 == setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize))) {
		LOG(ERROR) << "setsockopt sndbuf error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}

	buffsize = TCP_BUF_SIZE;
	if(-1 == setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize))) {
		LOG(ERROR) << "setsockopt rcvbuf error, connfd:" << connfd;
		if (connfd > 0) ::close(connfd);
		return false;
	}

	pCallback_->newConnection(connfd);
	return true;
}

bool Acceptor::handleWrite() {
	return true;
}

bool Acceptor::handleClose() {
	return true;
}

int Acceptor::createListen(const std::string &ip, uint32_t port) {
	int on = 1;
	listenfd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	struct sockaddr_in servaddr;
	::fcntl(listenfd_, F_SETFL, O_NONBLOCK); //no-block io
	setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	servaddr.sin_port = htons(port);

	if(-1 == ::bind(listenfd_, (struct sockaddr *)&servaddr, sizeof(servaddr)))
	{   
		LOG(FATAL) << "bind error, errno:" << errno;
	}   

	if(-1 == ::listen(listenfd_, 1024))
	{   
		LOG(FATAL) << "listen error, errno:" << errno;
	}   

	return listenfd_;
}

