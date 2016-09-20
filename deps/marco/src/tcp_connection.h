#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include "include.h"
#include "call_back.h"
#include "buffer.h"
#include "timing_wheel.h"

namespace marco {
	class EventLoop;
	class Channel;

	class TcpConnection : //public IChannelCallback,
						  public std::enable_shared_from_this<TcpConnection>
	{
		public:
			enum TypeE {tNone, tClient, tServer};
			enum StateE {kNone, kConnecting, kConnected, kDisconnecting, kDisconnected};

			TcpConnection(int sockfd, EventLoop *pLoop, uint32_t, TypeE type);
			~TcpConnection();

			void Send(const std::string &message);
			void sendInLoop(const std::string &message);
			
			void Send(const char *message, size_t len);
            void sendInLoopEx(const char *message, size_t len);
			
			void setTcpConnectCallback(ITcpConnCallback *pCallback);
			void setTcpCloseCallback(ICloseCallback *pCallback);

			void ConnectEstablished();
			void ConnectDestroy();

			virtual bool handleRead();
			virtual bool handleWrite();
			virtual bool handleClose();
	
/*			void setBucketPos(const WeakBucketPtr& bucket) { bucket_ = bucket; }
			WeakBucketPtr& getBucketPos()			  	{ return bucket_;}
			
			void setSlotPos(const WeakSlotPtr& slot) 	{ slot_ = slot;	}
			WeakSlotPtr& getSlotPos() 			  		{ return slot_;	}

			void setLastReceiveTime(uint32_t now) { lastReceiveTime_ = now;	}
			std::time_t getLastReceiveTime() 	  {	return lastReceiveTime_;}

			void setBucketId(uint64_t id) {	bucketId_ = id;	 }
			uint64_t getBucketId()        {	return bucketId_;}
*/
			uint64_t getBucketInfo() 		  {	return bucketInfo_;	}
			void setBucketInfo(uint64_t info) {	bucketInfo_ = info;	}

			int getSockFd() {	return sockfd_;	}

			TypeE getType() 	    {	return type_;	}
			void setType(TypeE t)   {	type_ = t;	}
			StateE getState()       {	return state_;	}
			void setState(StateE s) {	state_ = s;	}

			EventLoop* getEventLoop() {	return pLoop_;	}

		private:
			int sockfd_;

			EventLoop *pLoop_;
			std::shared_ptr<Channel> pSocketChannel_;

			ITcpConnCallback *pConnCallback_;
			ICloseCallback *pCloseCallback_;

			TypeE  type_;
			StateE state_;

			uint32_t maxSizePerRead_;
			Buffer inBuf_;
			Buffer outBuf_;

			//时间轮盘
			std::time_t lastReceiveTime_;
			uint64_t bucketInfo_; //32位 bucket 32位 slot
	};

}

#endif //_TCP_CONNECTION_H_
