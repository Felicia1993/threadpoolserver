#ifndef CHANNEL_H
#define CHANNEL_H

#include <./base/noncopyable.hpp>
#include <memory>
#include "Time.h"

class EventLoop;
typedef std::shared_ptr<Channel> SP_Channel

class Channel : noncopyable
{
public:
  	typedef std::function<void()> EventCallback;
	Channel(EventLoop* loop);
  	Channel(EventLoop* loop, int fd);
  	~Channel();
  	void handleEvent();
	void setFd(int fd);
	int getFd();

	void handl
private:
  	void update();
	void setReadCallback(const EventCallback& cb){
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback& cb){
		writeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback& cb){
		ErrorCallback_ = cb;
	}
	void setConnCallback(const EventCallback& cb){
		ConnCallback_ = cb;
	}

  	static const int kNoneEvent;
  	static const int kReadEvent;
  	static const int kWriteEvent;

  	EventLoop* loop_;
  	const int  fd_;
  	int        events_;
  	int        revents_; 
  
  	bool eventHandling_;
  
	
  	EventCallback writeCallback_;
  	EventCallback errorCallback_;
	EventCallback readCallback_;
  	EventCallback connCallback_;
	
};	
#endif  
