#include "EventLoop.h"
#include "Channel.h"
#include "epoll.h"
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
	cout<<"this is EventLoop::loop()\n";
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true;
//	poll(NULL, 0, 5*1000);
	quit_ = false;
	while(!quit_){
		activeChannels_.clear();
		poller_->poll(kPollTimeMs, &activeChannels_);	
		for(ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it){
			(*it)->handleEvent();
		}	
		doPendingFunctors();
	}
	cout<<"EventLoop "<<this<<" stop looping\n";
	looping_ = false;
}

void EventLoop::quit(){
	quit_ = true;
}

void EventLoop::updateChannel(Channel* channel){
	cout<<"something wrong here!\n";	
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);

}

void EventLoop::doPendingFunctors(){
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for(size_t i = 0; i < functors.size(); i++){
		functors[i]();
	}
	callingPendingFunctors_ = false;
}
void EventLoop::abortNotInLoopThread(){
	cout<<"EventLoop::abortNotInLoopThread"<<this<<"was created in thread_id"<<threadId_<<",current thread_id = "<<gettid();
}

void EventLoop::runInLoop(const Functor& cb){
	if(isInLoopThread()){
		cb();
	}
	else{
		queueInLoop(cb);
	}
}

void EventLoop::queueInLoop(const Functor& cb){
	{
	MutexGuardLock(mutex_);
	pendingFunctors_.push_back(cb);
	}
	if(!isInLoopThread() || callingPendingFunctors_){
		wakeup();
	}
}
