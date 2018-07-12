#pragma once

#include "time.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <sys/epoll.h>
#include <iostream>

class EventLoop;
class HttpData;

class Channel
{
public:
	
  	typedef std::function<void()> EventCallback;
	Channel(EventLoop* loop);
  	Channel(EventLoop* loop, int fd);
  	~Channel();
  	void handleEvent();

	void setHolder(std::shared_ptr<HttpData> holder){
		holder_ = holder;
	}

	std::shared_ptr<HttpData> getHolder(){
		return holder_;
	}

	
//for user 
	void setReadCallback(EventCallback&& cb){
		readCallback_ = cb;
	}
	void setWriteCallback(EventCallback&& cb){
		writeCallback_ = cb;
	}
	void setErrorCallback(EventCallback&& cb){
		errorCallback_ = cb;
	}
	void setConnCallback(EventCallback&& cb){
		connCallback_ = cb;
	}
	int fd() const{
		return fd_;
	}

	void setEvents(__uint32_t ev){
		events_ = ev;
	}

	__uint32_t& getEvents()const{	
		return events_;
	}

	bool EqualAndUpdateLastEvents(){
		bool ret = (lastEvents_ == events);
		lastEvents_ = events;
		return ret;	
	}

	__uint32_t getLastEvents(){
		return lastEvents_;
	}
private:
	int parseURI();
	int parseHead();
	int analysisRequest();
  	EventLoop* loop_;
  	int  fd_;
  	__uint32_t events_;
  	__uint32_t revents_; 
	__uint32_t lastEvents_;
  
  	std::weak_ptr<HttpData> holder_;
	
  	EventCallback writeCallback_;
  	EventCallback errorCallback_;
	EventCallback readCallback_;	
	EventCallback connCallback_;
};	
typedef std::shared_ptr<Channel> SP_Channel;

