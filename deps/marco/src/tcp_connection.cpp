#include "tcp_connection.h"
#include "event_loop.h"
#include "channel.h"
#include "timing_wheel.h"
#include "include.h"

using namespace marco;

namespace {
	std::string getTcpConnectTypeString(TcpConnection::TypeE type) {
		if (type == TcpConnection::tClient) {
			return "client";
		} else if (type == TcpConnection::tServer) {
			return "server";
		}
		return "not find conn type";
	 }
}

TcpConnection::TcpConnection(int sockfd, EventLoop *pLoop, uint32_t maxSizePerRead, TypeE type) :
	sockfd_(sockfd),
	pLoop_(pLoop),
	pSocketChannel_(new Channel(pLoop, sockfd)),
	pConnCallback_(NULL),
	pCloseCallback_(NULL),
	type_(type),
	state_(kNone),
	maxSizePerRead_(maxSizePerRead),
	inBuf_(maxSizePerRead_),
	outBuf_(maxSizePerRead_),
	lastReceiveTime_(0),
	bucketInfo_(0)
{
	LOG(INFO) << "TcpConnection cons";

	pSocketChannel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	pSocketChannel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	pSocketChannel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection()
{
	LOG(INFO) << "TcpConnection des, sockfd:" << sockfd_;
	::close(sockfd_);
}

bool TcpConnection::handleRead() {
//	LOG(INFO) << "handleRead()";
	bool is_error = false;
	int nread = -1;
	int ret = -1;
	bool isParseMsg = false;
	if (sockfd_ < 0) {
		LOG(ERROR) << "handleRead sockfd < 0 error";
		is_error = true;
		goto err;
	}
	while (inBuf_.Space() < maxSizePerRead_) {
		if (!inBuf_.Grow()) {
			LOG(ERROR) << "handleRead buffer grow error: " << sockfd_;
			is_error = true;
			goto err;
		}
	}

	do {
		nread = ::read(sockfd_, inBuf_.BeginWrite(), maxSizePerRead_);
		if (nread > 0) {
			inBuf_.HasWrite(nread);
			break;
		} else if (nread == -1) {
			if (errno == EAGAIN) { //非阻塞模式下调用阻塞操作中
				LOG(ERROR) << "read error:" << errno << ", sockfd:" << sockfd_;
				is_error = true;
				goto err;
			} else if (errno != EINTR) { //错误
				LOG(ERROR) << "read error:" << errno << ", sockfd:" << sockfd_;
				is_error = true;
				goto err;
			}
		} else if (nread == 0) { //网络连接断开时
			LOG(ERROR) << "read length 0, sockfd:" << sockfd_;
			is_error = true;
			goto err;
		}
	} while(nread == -1 && errno == EINTR); //中断

err:
	if (is_error) {
		handleClose();
		return false;
	}

	//分包逻辑抛给逻辑层处理
	ret = pConnCallback_->onMessage(this, &inBuf_);	
	while (true) {
		if (ret == -1) {
			handleClose();
			return false;
		}
		if (ret == 0) {
			break;
		}
		if (ret > 0) {
			inBuf_.HasRead(ret);
			isParseMsg = true;
			ret = pConnCallback_->onMessage(this, &inBuf_);
		}
	}
	if (isParseMsg) {
		//只有Server连接执行时间轮盘
		if (type_ == tServer) {
			WeakTcpConnPtr weakConn(shared_from_this());
			pLoop_->runInLoop(std::bind(&EventLoop::UpdateTimingWheel, pLoop_, weakConn));
		}
		inBuf_.Adjust();
	}
	return true;
}

bool TcpConnection::handleWrite() {
//	LOG(INFO) << "handleWrite()";
	if (pSocketChannel_->isWriting()) {
		int nwrite = -1;
		do {
			nwrite = ::write(sockfd_, outBuf_.BeginRead(), outBuf_.Len());
			if (nwrite > 0) {
				
			} else if (nwrite == -1) {
				if (errno == EAGAIN) {
					nwrite = 0;
				} else if (errno != EINTR) {
					nwrite = -1;
				}
			}
		} while (nwrite = -1 && errno == EINTR);
		if (nwrite >= 0) {
			outBuf_.HasRead(nwrite);
			outBuf_.Adjust();
			if (outBuf_.Len() == 0) {
				pSocketChannel_->disableWriting();
			}
		} else {
			return false;
		}
	}
	return true;
}

void TcpConnection::Send(const std::string &message) {
	if (pLoop_->isInLoopThread()) {
		sendInLoop(message);
	} else {
		pLoop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
	}
}

void TcpConnection::sendInLoop(const std::string &message) {
	pLoop_->assertInLoopThread();
	int n = 0;
	if (outBuf_.Len() == 0) {
		n = ::write(sockfd_, message.c_str(), message.size());
		if (n == -1) {
			if (errno == EAGAIN || errno == EINTR) {
				n = 0;
			}
		}
		if (n == -1) {
			LOG(INFO) << "write error handleClose conn";
			handleClose();
			return;
		}
	}

	if (n > 0 && n < message.size()) {
		message.substr(n, message.size());
		outBuf_.Write(message.c_str(), message.size());
		outBuf_.HasWrite(message.size());
		if (!pSocketChannel_->isWriting()) {
			pSocketChannel_->enableWriting();
		}
	}
}

bool TcpConnection::handleClose() {
//	LOG(INFO) << "TcpConnection::handleClose()";
	pLoop_->assertInLoopThread();

	assert(state_ == kConnected || state_ == kDisconnecting);

	setState(kDisconnected);
	pSocketChannel_->disableAll();

	//只有Server连接执行时间轮盘
	if (type_ == tServer) {
		WeakTcpConnPtr weakConn(shared_from_this());
		pLoop_->runInLoop(std::bind(&EventLoop::DeleteTimingWheel, pLoop_, weakConn));
	}
	
	pCloseCallback_->closeConnection(sockfd_);
	
	return true;
}

void TcpConnection::ConnectEstablished() {
	pLoop_->assertInLoopThread();
	if (state_ != kConnected) {
		LOG(ERROR) << "connect established failed, tcp connection state(" << state_ << ")";
		handleClose();
		return;
	}

	pSocketChannel_->enableReading();

	//Server连接执行时间轮盘
	if (type_ == tServer) {
		WeakTcpConnPtr weakConn(shared_from_this());
		pLoop_->runInLoop(std::bind(&EventLoop::InsertTimingWheel, pLoop_, weakConn));
	}

	pConnCallback_->onConnection(this);
}

void TcpConnection::ConnectDestroy() {
	pLoop_->assertInLoopThread();

	if (state_ == kConnected) {
		setState(kDisconnected);
		pSocketChannel_->disableAll();

		//只有Server连接执行时间轮盘
		if (type_ == tServer) {
			WeakTcpConnPtr weakConn(shared_from_this());
			pLoop_->runInLoop(std::bind(&EventLoop::DeleteTimingWheel, pLoop_, weakConn));
		}

		pCloseCallback_->closeConnection(sockfd_);
	}
}

void TcpConnection::setTcpConnectCallback(ITcpConnCallback *pCallback) {
	pConnCallback_ = pCallback;
}

void TcpConnection::setTcpCloseCallback(ICloseCallback *pCallback) {
	pCloseCallback_ = pCallback;
}

