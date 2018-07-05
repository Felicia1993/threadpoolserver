#include "timer.h"
#include "epoll.h"
#include <unordered_map>
#include <string>
#include <sys/time.h>
#include <deque>
#include <unistd.h>
#include <queue>

#include <iostream>
using namespace std;

TimeNode::TimeNode(SP_ReqData _request_data, int timeout):deleted(false), request_data(_request_data){
	cout<<"TimeNode()"<<endl;
	struct timeval now;
	gettimeofday(&now, NULL);
	expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimeNode::~TimeNode(){
	cout<<"~TimeNode()"<<endl;
	if(request_data){
		Epoll::epoll_del(request_data->getFd());
	}
}

void TimeNode::update(int timeout){
	struct timeval now;
	gettimeofday(&now, NULL);
	expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimeNode::isvalid(){
	struct timeval now;
	gettimeofday(&now, NULL);
	size_t temp = (now.tv_sec * 1000) + (now.tv_usec / 1000);
	if(temp < expired_time){
		return true;
	}
	else{
		this->setDeleted();
		return false;
	}
}

void TimeNode::clearReq(){
	request_data.reset();
	this->setDeleted();
}

void TimeNode::setDeleted() const{
	deleted = true;
}

size_t TimeNode::getExpTime() const{
	return expired_time;
}

TimerManager::TimerManager(){

}

TimerManager::~TimerManager{
	
}

void TimerManager::addTimer(SP_ReqData request_data, int timeout){
	SP_TimeNode new_node(new TimeNode(request_data, timeout));
	{
		MutexLockGuard locker(lock);
		TimeNodeQueue.push(new_node);
	}
	request_data->linkTimer(new_node);
}

void TimerManager::addTimer(SP_TimeNode timer_node){
	
}

void TimerManager::handle_expired_event(){
	MutexLockGuard locker(lock);
	while(!TimeNodeQueue.empty()){
		SP_TimeNode ptimer_now = TimeNodeQueue.top();
		if(ptimer_now -> isDeleted()){
			TimeNodeQueue.pop();
		}
		else if(ptimer_now->isvalid() == false){
			TimeNodeQueue.pop();
		}
		else
			break;
	}
}
