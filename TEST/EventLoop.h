#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <memory>
#include <vector>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

#define gettid() syscall(SYS_gettid)

class Channel;
class EPollPoller;

class EventLoop 
{
public:
	friend class Channel;
	friend class EPollPoller;
  	EventLoop();
  	~EventLoop();  
  	void loop();
	void quit();
	void updateChannel(Channel* channel);	
  	void assertInLoopThread(){
    		if (!isInLoopThread()){
      			abortNotInLoopThread();
    		}
  	}
  	bool isInLoopThread() const { return threadId_ == gettid(); }
//	string epollReturnTime() const{return epollReturnTime_;}
	
private:
	typedef std::function<void()> Functor;
	typedef std::vector<Channel*> ChannelList;
//	shared_ptr<EPollPoller> poller_;
	ChannelList activeChannels_;
	bool quit_;
	void abortNotInLoopThread();
	bool looping_;	
  	const pid_t threadId_;  
	string epollReturnTime_;
	shared_ptr<EPollPoller> epoll_;
};
#endif
