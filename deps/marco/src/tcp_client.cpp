#include "include.h"
#include "tcp_client.h"
#include "tcp_connection.h"
#include "event_loop.h"

using namespace marco;

TcpClient::TcpClient(EventLoop *pLoop, const std::string &ip, uint32_t port) :
	sockfd_(0),
	ip_(ip),
	port_(port),
	pLoop_(pLoop),
	state_(kDisconnected),
	retryDelayMs_(kInitRetryDelayMs)
{
	LOG(INFO) << "TcpClient cons";

	sockfd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	int flags;
	if ((flags = ::fcntl(sockfd_, F_GETFL)) == -1) {
		LOG(ERROR) << "fcntl getfl error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}
	if(-1 == ::fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK)) {
		LOG(ERROR) << "fcntl setfl error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}

	int yes = 1;
	if(-1 == ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) {
		LOG(ERROR) << "setsockopt nodelay error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}

	int on = 1;
	if(-1 == setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		LOG(ERROR) << "setsockopt reuseaddr error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}

	int buffsize = TCP_BUF_SIZE;
	if(-1 == setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize))) {
		LOG(ERROR) << "setsockopt sndbuf error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}

	buffsize = TCP_BUF_SIZE;
	if(-1 == setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize))) {
		LOG(ERROR) << "setsockopt rcvbuf error, sockfd:" << sockfd_;
		if (sockfd_ > 0) ::close(sockfd_);
	}
}

TcpClient::~TcpClient() {
	LOG(INFO) << "TcpClient des";
	::close(sockfd_);
}

int TcpClient::connect() {
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip_.c_str());
	servaddr.sin_port = htons(port_);
	int ret = 0;
	do {
		ret = ::connect(sockfd_, (sockaddr*)&servaddr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
		if (ret == 0) {
			return ret;
		} else if (ret == -1) {
			if (errno == EISCONN) {
				return 0;
			}
		}
	} while(ret == -1 && errno == EINTR);
	return ret;
}

void TcpClient::disconnect() {
	if (pConn_ != NULL && state_ == kConnected) {
		state_ = kDisconnected;

		pConn_->ConnectDestroy();
		pConn_.reset();
	}
}

void TcpClient::Start()  {
	if (sockfd_ <= 0) {
		LOG(INFO) << "sockfd <= 0 error";
		return;
	}
	int ret = connect();
	int savedErrno = (ret == 0) ? 0 : errno;
	switch (savedErrno) {
		case 0:
		case EINPROGRESS:
			{
				if (savedErrno != 0) {
					int err;
					socklen_t errlen = sizeof(err);
					int ret = getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &errlen);
					if (ret < 0 || err != 0) {
						LOG(INFO) << "connect server errno:" << savedErrno << ", so retry connect, seraddr:" 
								  << ip_.c_str() << ":" << port_;
						Retry();
						return;
					}
				}

				state_ = kConnected;
				pConn_ = TcpConnectionPtr(new TcpConnection(sockfd_, pLoop_, BUFFER_MAX_SIZE, TcpConnection::tClient));
				pConn_->setTcpConnectCallback(pConnCallback_);
				pConn_->setTcpCloseCallback(this);
				pConn_->setState(TcpConnection::kConnected);
				pLoop_->runInLoop(std::bind(&TcpConnection::ConnectEstablished, pConn_));
				break;
			} 
		default:
			{
				LOG(INFO) << "connect server errno:" << savedErrno << ", retry connect, seraddr:" 
					<< ip_.c_str() << ":" << port_;
				Retry();
			}
	}
}

void TcpClient::Stop() {
	disconnect();
}

void TcpClient::Retry() {
	if (pConn_ != NULL) {
		pConn_->ConnectDestroy();
		pConn_.reset();
	}
	pLoop_->AddTimer(std::bind(&TcpClient::Start, this), std::time(NULL), retryDelayMs_, false);
	retryDelayMs_ = std::min(retryDelayMs_*2, kMaxRetryDelayMs);
}

void TcpClient::closeConnection(int sockfd) {
	state_ = kDisconnected;

	TcpConnection* conn = pConn_.get();
	pConnCallback_->onClose(conn);
	pConn_.reset();
}

void TcpClient::setNewConnCallback(ITcpConnCallback *pConnCallback) {
	pConnCallback_ = pConnCallback;
}

void TcpClient::Write(std::string &msg) {
	if (state_ == kDisconnected) {
		LOG(ERROR) << "send failed, client state is disconnected";
		return;
	}
	if (pConn_ == NULL) {
		LOG(ERROR) << "send failed, not conn object is null";
		return;
	}
	pConn_->Send(msg);
}
