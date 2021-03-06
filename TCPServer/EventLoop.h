#pragma once
#include "base/Thread.h"
#include "Epoll.h"
#include "base/Logging.h"
#include "Channel.h"
#include "base/CurrentThread.h"
#include "util.h"
#include <unistd.h>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
using namespace std;

class EventLoop{
public:
	typedef std::function<void()> Functor;
	EventLoop();
	~EventLoop();
	void loop();
	void quit();
	void runInLoop(Functor&& cb);
	void queueInLoop(Functor&& cb);
	bool isInLoopThread() const{
		return threadId_ == CurrentThread::tid();
	}
	void assertInLoopThread(){
		assert(isInLoopThread());
	}
/*	void shutdown(shared_ptr<Channel> channel){
		shutDownWR(channel->getFd());
	}*/
	void removeFromPoller(shared_ptr<Channel> channel){
		poller_->epoll_del(channel);
	}
	void updatePoller(shared_ptr<Channel> channel, int timeout = 0){
		poller_->epoll_mod(channel, timeout);
	}
	void addToPoller(shared_ptr<Channel> channel, int timeout = 0){
		poller_->epoll_add(channel, timeout);
	}
private:
	bool looping_;
	bool quit_;
	const pid_t threadId_;
	shared_ptr<Epoll> poller_;
	int wakeupFd_;
	shared_ptr<Channel> pwakeupChannel_;	
	bool eventHandling_;
	mutable MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;
	bool callingPendingFunctors_;

	void wakeup();
	void handleRead();
	void doPendingFunctors();
	void handleConn();
	
};
