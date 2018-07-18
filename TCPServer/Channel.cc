#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <assert.h>
using namespace std;
const int kNoneEvent = 0;
const int kReadEvent = EPOLLIN|EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

Channel:: Channel(EventLoop* loop, int fdArg):fd_(fdArg),loop_(loop),events_(0),lastEvents_(0){

}

Channel::Channel(EventLoop* loop):loop_(loop),events_(0),lastEvents_(0){}

Channel::~Channel(){
	
	
}

void Channel::handleEvent(){
	cout<<"this is Channel handleEvent\n";
	if(revents_ & EPOLLERR){
		if(errorCallback_)
			errorCallback_();
	}
	if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
		if(readCallback_)
			readCallback_();
	}
	if(revents_ & EPOLLOUT){
		if(writeCallback_)
			writeCallback_();
	}
} 


