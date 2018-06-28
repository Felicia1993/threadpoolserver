#include "EventLoop.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <assert.h>
#include <iostream>

using namespace std;

__thread EventLoop* t_loopinThisThread = 0;


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
	poll(NULL, 0, 5*1000);
	cout<<"EventLoop "<<this<<" stop looping\n";
	looping_ = false;
}

