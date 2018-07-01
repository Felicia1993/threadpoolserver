#include "EventLoop.h"
#include "Channel.h"
#include "EpollPoller.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <assert.h>
#include <iostream>

using namespace std;

__thread EventLoop* t_loopinThisThread = 0;

const int kPollTimeMs = 10000;

EventLoop::EventLoop()
:	looping_(false),
	threadId_(gettid())
	{
	if(t_loopinThisThread){
		cout<<"Another EventLoop"<<t_loopinThisThread<<"exists in this thread"<<threadId_<<endl;
	}
	else{
		t_loopinThisThread = this;
	}
}

EventLoop::~EventLoop(){
	assert(!looping_);
	t_loopinThisThread = NULL;
}

void EventLoop::loop(){
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true;
	quit_ = false;
	while(!quit_){
		activeChannels_.clear();
		epollReturnTime_ = epoll_->poll(kPollTimeMs, &activeChannels_);	
		for(ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it){
			(*it)->Channel::handleEvent();
		}
		for(int i =0 ; i < activeChannels_.size(); i++){
			activeChannels_[i]->handleEvent();
		}	
	}
	cout<<"EventLoop "<<this<<" stop looping\n";
	looping_ = false;
}

void EventLoop::quit(){
	quit_ = true;
}

void EventLoop::updateChannel(Channel* channel){
	//assert(channel->ownerLoop());
	assertInLoopThread();
//	poller_->updateChannel(channel);
	cout<<"something wrong in updateChannel,but I can't find!\n";
}

void EventLoop::abortNotInLoopThread(){
	cout<<"EventLoop::abortNotInLoopThread"<<this<<"was created in thread_id"<<threadId_<<",current thread_id = "<<gettid();
}
