#include "Channel.h"

const int kNoneEvent = 0;
const int kReadEvent = POLLIN|POLLPRI;
const int kWriteEvent = POLLOUT;

Channel:: Channel(EventLoop* loop, int fd):fd_(fd),loop_(loop),index_(-1),events_(0),revents_(0){

}

void Channel::update(){
	loop_->updateChannel(this);
}

void Channel::handleEvent(){
	if(revents_ & POLLNVAL)
		LOG_WARN<<"Channel::handle_event() POLLNVAL";
	if(revents_ & (POLLERR | POLLNVAL)){
		if(errorCallback_)
			errorCallback_();
	}
	if(revent_ & (POLLIN | POLLPRI | POLLRDHUP)){
		if(readCallback_)
			readCallback_();
	}
	if(revents_ & POLLOUT){
		if(writeCallback_)
			writeCallback_();
	}
	connCallback_();
} 

void Channel::setFd(int fd){
	fd_ = fd;
}
int Channel::getFd(){
	return fd_;
}
