#ifndef CHANNEL_H
#define CHANNEL_H

#include <memory>
#include <functional>
#include <sys/epoll.h>

class EventLoop;

class Channel
{
public:
	friend class EventLoop;
  	typedef std::function<void()> EventCallback;
  	Channel(EventLoop* loop, int fd);
  	~Channel();
  	void handleEvent();
//for user 
	void setReadCallback(const EventCallback& cb){
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback& cb){
		writeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback& cb){
		errorCallback_ = cb;
	}
	int fd() const{
		return fd_;
	}
	int events()const{	
		return events_;
	}
	void set_revents(int revt){ 
		revents_ = revt;
	}
	bool isNoneEvent()const{
		return events_ == kNoneEvent;	
	}

	void enableReading(){
		events_ |= kWriteEvent;
		update();
	}
	void set_index(int idx){
		index_ = idx;
	}
	int index() const{
		return index_;
	}
	
	EventLoop* ownerLoop(){
		return loop_;
	}
private:
  	void update();

  	static const int kNoneEvent;
  	static const int kReadEvent;
  	static const int kWriteEvent;

  	EventLoop* loop_;
  	const int  fd_;
  	int events_;
  	int revents_; 
	int index_;
  
  	bool eventHandling_;
	
  	EventCallback writeCallback_;
  	EventCallback errorCallback_;
	EventCallback readCallback_;	
};	
#endif  
