#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include <memory>

class TcpServer{
public:
	TcpServer(EventLoop* loop, int threadnum, int port);
	~TcpServer();
	EventLoop* getLoop() const { return loop_; }
    	void start();
    	void handNewConn();
    	void handThisConn() { loop_->updatePoller(acceptChannel_); }
private:
	 EventLoop *loop_;
    	int threadNum_;
    	std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    	bool started_;
    	std::shared_ptr<Channel> acceptChannel_;
    	int port_;
    	int listenFd_;
    	static const int MAXFDS = 100000;
};
