#include "EventLoop.h"
#include "TimeQueue.h"
#include "Channel.h"
#include <sys/timerfd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <iterator>

int createTimerfd(){
	int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if(timerfd < 0){
		printf("Failed in timerfd_create");
	}
	return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop):loop_(loop),timerfd_(createTimerfd()),
timerfdChannel_(loop, timerfd_),timers_(),callingExpiredTimers_(false){
	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	timerChannel_.enableReading();
}

TimerQueue::~TimerQueue(){
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	close(timerfd_);
	Entry p;
	while(!TimerList.empty()){
		p = TimerList.top();
		TimerList.pop();
	}
}

void TimerQueue::handleRead(){
	loop_->assertInLoopThread();
  	Timestamp now(Timestamp::now());
  	readTimerfd(timerfd_, now);
  	std::vector<Entry> expired = getExpired(now);
  	callingExpiredTimers_ = true;
  	cancelingTimers_.clear();
  	for (std::vector<Entry>::iterator it = expired.begin();it != expired.end(); ++it){
    		it->second->run();
  	}
  	callingExpiredTimers_ = false;
  	reset(expired, now);
}

std::vector<Entry> TimerQueue::getExpired(Timestamp now){
	std::vector<Entry> expired;
	Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	while(!timers_.empty()){

	}
	TimerList::iterator it = timers_.lower_bound(sentry);
	assert(it == timers.end() || now < it->first());
	std::copy(timers_.begin(), it, back_inserter(expired));
	timers_.erase(timer_begin(), it);
	
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now){
	Timestamp nextExpire;

	for()
}

bool TimerQueue::insert(Timer* timer){

}
